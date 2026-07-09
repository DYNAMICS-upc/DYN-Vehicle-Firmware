#include <unity.h>
#include "pdm_config.h"
#include "mosfet_driver.h"
#include "mux_adc_driver.h"
#include "protection.h"
#include "fault_manager.h"

void setUp(void) {
    fault_manager_init();
    mosfet_driver_init();
    protection_init();
}

void tearDown(void) {}

void test_mosfet_init_and_control(void) {
    // All 12 channels should be ON by default
    for (int i = 0; i < MUX_CHANNELS; i++) {
        TEST_ASSERT_EQUAL_UINT8(1, mosfet_driver_get_status(i));
    }
    
    // Toggle individual channel
    mosfet_driver_set_channel(0, false);
    TEST_ASSERT_EQUAL_UINT8(0, mosfet_driver_get_status(0));
    TEST_ASSERT_EQUAL_UINT8(1, mosfet_driver_get_status(1));
    
    mosfet_driver_set_all(false);
    for (int i = 0; i < MUX_CHANNELS; i++) {
        TEST_ASSERT_EQUAL_UINT8(0, mosfet_driver_get_status(i));
    }
}

void test_protection_standard_channel_instant_trip_and_locking(void) {
    // Channel 0: Nominal 2000 mA -> 130% = 2600 mA
    // Above 130%: Immediate Trip
    TEST_ASSERT_FALSE(protection_check_channel_instant(0, 2700.0f));
    TEST_ASSERT_EQUAL_UINT8(0, mosfet_driver_get_status(0));
    TEST_ASSERT_TRUE(fault_manager_is_high_fault_active());
    TEST_ASSERT_TRUE(fault_manager_is_channel_locked(0));

    // ATTEMPT RE-ENABLE: Must be blocked by Safety Layer!
    mosfet_driver_set_channel(0, true);
    TEST_ASSERT_EQUAL_UINT8(0, mosfet_driver_get_status(0)); // Sigue apagado!
    
    // Diagnostic cause verification
    fault_record_t rec = fault_manager_get_last_fault();
    TEST_ASSERT_TRUE(rec.active);
    TEST_ASSERT_EQUAL(FAULT_CAT_HARDWARE, rec.category);
    TEST_ASSERT_EQUAL(FAULT_PRIORITY_HIGH, rec.priority);
    TEST_ASSERT_EQUAL_UINT32(1, rec.code); // Canal 0 -> Code 1
}

void test_protection_inverter_persistence_3_samples(void) {
    // Channel 9 (Inverter): Nominal 2500 mA -> 130% = 3250 mA
    float overcurrent = 3500.0f; // > 3250 mA
    
    // Sample 1: Should NOT trip yet (inrush tolerance)
    TEST_ASSERT_TRUE(protection_check_channel_instant(CANAL_INVERTER, overcurrent));
    TEST_ASSERT_EQUAL_UINT8(1, mosfet_driver_get_status(CANAL_INVERTER));
    
    // Sample 2: Should NOT trip yet
    TEST_ASSERT_TRUE(protection_check_channel_instant(CANAL_INVERTER, overcurrent));
    TEST_ASSERT_EQUAL_UINT8(1, mosfet_driver_get_status(CANAL_INVERTER));
    
    // Sample 3: Must TRIP and LOCK!
    TEST_ASSERT_FALSE(protection_check_channel_instant(CANAL_INVERTER, overcurrent));
    TEST_ASSERT_EQUAL_UINT8(0, mosfet_driver_get_status(CANAL_INVERTER));
    TEST_ASSERT_TRUE(fault_manager_is_high_fault_active());
    TEST_ASSERT_TRUE(fault_manager_is_channel_locked(CANAL_INVERTER));
    
    // Attempt re-enable must be blocked
    mosfet_driver_set_channel(CANAL_INVERTER, true);
    TEST_ASSERT_EQUAL_UINT8(0, mosfet_driver_get_status(CANAL_INVERTER));
}

void test_protection_volant_persistence_3_samples(void) {
    // Channel 3 (Volant): Nominal 3000 mA -> 130% = 3900 mA
    float overcurrent = 4200.0f; // > 3900 mA
    
    // Samples 1 & 2: Safe
    TEST_ASSERT_TRUE(protection_check_channel_instant(CANAL_VOLANT, overcurrent));
    TEST_ASSERT_TRUE(protection_check_channel_instant(CANAL_VOLANT, overcurrent));
    TEST_ASSERT_EQUAL_UINT8(1, mosfet_driver_get_status(CANAL_VOLANT));
    
    // Sample 3: Trips and Locks
    TEST_ASSERT_FALSE(protection_check_channel_instant(CANAL_VOLANT, overcurrent));
    TEST_ASSERT_EQUAL_UINT8(0, mosfet_driver_get_status(CANAL_VOLANT));
    TEST_ASSERT_TRUE(fault_manager_is_channel_locked(CANAL_VOLANT));
}

void test_protection_persistence_reset_on_recovery(void) {
    // 2 High samples on Inverter
    protection_check_channel_instant(CANAL_INVERTER, 3500.0f);
    protection_check_channel_instant(CANAL_INVERTER, 3500.0f);
    
    // 1 Normal sample -> Resets counter
    protection_check_channel_instant(CANAL_INVERTER, 2000.0f);
    
    // Next 2 High samples should still be tolerated
    TEST_ASSERT_TRUE(protection_check_channel_instant(CANAL_INVERTER, 3500.0f));
    TEST_ASSERT_TRUE(protection_check_channel_instant(CANAL_INVERTER, 3500.0f));
    TEST_ASSERT_EQUAL_UINT8(1, mosfet_driver_get_status(CANAL_INVERTER));
}

void test_fault_manager_records_and_clearing(void) {
    fault_manager_init();
    TEST_ASSERT_FALSE(fault_manager_is_high_fault_active());
    
    // Low priority should not trigger high fault state
    fault_manager_report(FAULT_CAT_RESOURCES, FAULT_PRIORITY_LOW, 1);
    TEST_ASSERT_FALSE(fault_manager_is_high_fault_active());
    
    // High priority should lock the system and register record
    fault_manager_report(FAULT_CAT_HARDWARE, FAULT_PRIORITY_HIGH, 42);
    TEST_ASSERT_TRUE(fault_manager_is_high_fault_active());
    
    fault_record_t r = fault_manager_get_last_fault();
    TEST_ASSERT_EQUAL(FAULT_CAT_HARDWARE, r.category);
    TEST_ASSERT_EQUAL_UINT32(42, r.code);
    TEST_ASSERT_EQUAL_UINT32(2, r.fault_count);
    
    fault_manager_clear_all();
    TEST_ASSERT_FALSE(fault_manager_is_high_fault_active());
}

int main(int argc, char **argv) {
    (void)argc;
    (void)argv;
    UNITY_BEGIN();
    RUN_TEST(test_mosfet_init_and_control);
    RUN_TEST(test_protection_standard_channel_instant_trip_and_locking);
    RUN_TEST(test_protection_inverter_persistence_3_samples);
    RUN_TEST(test_protection_volant_persistence_3_samples);
    RUN_TEST(test_protection_persistence_reset_on_recovery);
    RUN_TEST(test_fault_manager_records_and_clearing);
    return UNITY_END();
}
