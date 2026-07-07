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

void test_protection_standard_channel_instant_trip(void) {
    // Channel 0: Nominal 2000 mA -> 130% = 2600 mA
    // Below 130%: Safe
    TEST_ASSERT_TRUE(protection_check_channel_instant(0, 2500.0f));
    TEST_ASSERT_EQUAL_UINT8(1, mosfet_driver_get_status(0));
    
    // Above 130%: Immediate Trip on standard channels
    TEST_ASSERT_FALSE(protection_check_channel_instant(0, 2700.0f));
    TEST_ASSERT_EQUAL_UINT8(0, mosfet_driver_get_status(0));
    TEST_ASSERT_TRUE(fault_manager_is_high_fault_active());
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
    
    // Sample 3: Must TRIP!
    TEST_ASSERT_FALSE(protection_check_channel_instant(CANAL_INVERTER, overcurrent));
    TEST_ASSERT_EQUAL_UINT8(0, mosfet_driver_get_status(CANAL_INVERTER));
    TEST_ASSERT_TRUE(fault_manager_is_high_fault_active());
}

void test_protection_volant_persistence_3_samples(void) {
    // Channel 3 (Volant): Nominal 3000 mA -> 130% = 3900 mA
    float overcurrent = 4200.0f; // > 3900 mA
    
    // Samples 1 & 2: Safe
    TEST_ASSERT_TRUE(protection_check_channel_instant(CANAL_VOLANT, overcurrent));
    TEST_ASSERT_TRUE(protection_check_channel_instant(CANAL_VOLANT, overcurrent));
    TEST_ASSERT_EQUAL_UINT8(1, mosfet_driver_get_status(CANAL_VOLANT));
    
    // Sample 3: Trips
    TEST_ASSERT_FALSE(protection_check_channel_instant(CANAL_VOLANT, overcurrent));
    TEST_ASSERT_EQUAL_UINT8(0, mosfet_driver_get_status(CANAL_VOLANT));
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

void test_protection_battery_undervoltage_debounce(void) {
    // Setup mock V_SENSE to low voltage (< 5.0V)
    // ADC 1000 with divider ~ 4.68V
    mux_adc_driver_set_mock_pin_value(V_SENSE_PIN, 1000);
    
    float vbat = 0;
    // At t = 0ms: Detected, but not tripped
    TEST_ASSERT_TRUE(protection_check_battery(&vbat, 0));
    TEST_ASSERT_EQUAL_UINT8(1, mosfet_driver_get_status(0));
    
    // At t = 100ms (< 200ms debounce): Still safe
    TEST_ASSERT_TRUE(protection_check_battery(&vbat, 100));
    TEST_ASSERT_EQUAL_UINT8(1, mosfet_driver_get_status(0));
    
    // At t = 250ms (> 200ms debounce): Tripped!
    TEST_ASSERT_FALSE(protection_check_battery(&vbat, 250));
    for (int i = 0; i < MUX_CHANNELS; i++) {
        TEST_ASSERT_EQUAL_UINT8(0, mosfet_driver_get_status(i));
    }
    TEST_ASSERT_TRUE(fault_manager_is_high_fault_active());
}

void test_fault_manager(void) {
    fault_manager_init();
    TEST_ASSERT_FALSE(fault_manager_is_high_fault_active());
    
    // Low priority should not trigger high fault state
    fault_manager_report(FAULT_CAT_RESOURCES, FAULT_PRIORITY_LOW, 1);
    TEST_ASSERT_FALSE(fault_manager_is_high_fault_active());
    
    // High priority should lock the system
    fault_manager_report(FAULT_CAT_HARDWARE, FAULT_PRIORITY_HIGH, 2);
    TEST_ASSERT_TRUE(fault_manager_is_high_fault_active());
    
    fault_manager_clear_all();
    TEST_ASSERT_FALSE(fault_manager_is_high_fault_active());
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_mosfet_init_and_control);
    RUN_TEST(test_protection_standard_channel_instant_trip);
    RUN_TEST(test_protection_inverter_persistence_3_samples);
    RUN_TEST(test_protection_volant_persistence_3_samples);
    RUN_TEST(test_protection_persistence_reset_on_recovery);
    RUN_TEST(test_protection_battery_undervoltage_debounce);
    RUN_TEST(test_fault_manager);
    return UNITY_END();
}
