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

// --------------------------------------------------------------------------
// RANGE 1: Advisory Warning when Current > 110% of Nominal
// --------------------------------------------------------------------------
void test_protection_range1_warning_above_110_percent(void) {
    // Channel 0: Nominal 2000 mA -> 110% = 2200 mA, 140% = 2800 mA
    // Current at 2300 mA (+15%): Advisory Warning
    protection_level_t lvl = protection_check_channel(0, 2300.0f, 1000);
    TEST_ASSERT_EQUAL(PROT_LEVEL_WARNING_110, lvl);
    TEST_ASSERT_TRUE(protection_is_warning_active(0));
    TEST_ASSERT_FALSE(protection_is_timer_running(0));
    TEST_ASSERT_EQUAL_UINT8(1, mosfet_driver_get_status(0)); // Must remain powered
    TEST_ASSERT_FALSE(fault_manager_is_channel_locked(0));
    TEST_ASSERT_FALSE(fault_manager_is_high_fault_active());

    // Recovery test: Current returns to nominal (1900 mA <= 110%)
    lvl = protection_check_channel(0, 1900.0f, 2000);
    TEST_ASSERT_EQUAL(PROT_LEVEL_NORMAL, lvl);
    TEST_ASSERT_FALSE(protection_is_warning_active(0));
}

// --------------------------------------------------------------------------
// RANGE 2: Overload (140% to 170%) -> Starts 60s Timer
// --------------------------------------------------------------------------
void test_protection_range2_timer_start_between_140_and_170_percent(void) {
    // Channel 0: Nominal 2000 mA -> 140% = 2800 mA, 170% = 3400 mA
    // Current at 3000 mA (150%): Starts 60s countdown
    protection_level_t lvl = protection_check_channel(0, 3000.0f, 5000);
    TEST_ASSERT_EQUAL(PROT_LEVEL_TIMER_140_170, lvl);
    TEST_ASSERT_TRUE(protection_is_warning_active(0));
    TEST_ASSERT_TRUE(protection_is_timer_running(0));
    TEST_ASSERT_EQUAL_UINT32(0, protection_get_timer_elapsed_ms(0, 5000));
    TEST_ASSERT_EQUAL_UINT8(1, mosfet_driver_get_status(0)); // Still powered

    // 30 seconds later (t = 35000 ms), current still at 3000 mA
    lvl = protection_check_channel(0, 3000.0f, 35000);
    TEST_ASSERT_EQUAL(PROT_LEVEL_TIMER_140_170, lvl);
    TEST_ASSERT_EQUAL_UINT32(30000, protection_get_timer_elapsed_ms(0, 35000));
    TEST_ASSERT_EQUAL_UINT8(1, mosfet_driver_get_status(0)); // Still powered during 60s
}

// --------------------------------------------------------------------------
// RANGE 2 RECOVERY: Drops below 110% -> Timer Resets & Power Remains ON
// --------------------------------------------------------------------------
void test_protection_range2_timer_recovery_under_110_percent(void) {
    // Start 60s timer at t = 0 ms with 3000 mA (150%)
    protection_check_channel(0, 3000.0f, 0);
    TEST_ASSERT_TRUE(protection_is_timer_running(0));

    // At t = 20000 ms, current drops below 110% (2100 mA <= 2200 mA)
    protection_level_t lvl = protection_check_channel(0, 2100.0f, 20000);
    TEST_ASSERT_EQUAL(PROT_LEVEL_NORMAL, lvl);
    TEST_ASSERT_FALSE(protection_is_timer_running(0)); // Timer cancelled!
    TEST_ASSERT_FALSE(protection_is_warning_active(0));
    TEST_ASSERT_EQUAL_UINT8(1, mosfet_driver_get_status(0));
}

// --------------------------------------------------------------------------
// RANGE 2 TRIP: 60s Expired without dropping <= 110% -> Cutoff & Lockout
// --------------------------------------------------------------------------
void test_protection_range2_timer_expired_trips_and_locks_channel(void) {
    // Start timer at t = 0 ms with 3000 mA (150%)
    protection_check_channel(0, 3000.0f, 0);

    // At t = 59000 ms (59s): Still alive
    protection_level_t lvl = protection_check_channel(0, 3000.0f, 59000);
    TEST_ASSERT_EQUAL(PROT_LEVEL_TIMER_140_170, lvl);
    TEST_ASSERT_EQUAL_UINT8(1, mosfet_driver_get_status(0));

    // At t = 60000 ms (60s exact threshold reached): CUTOFF & LOCKOUT!
    lvl = protection_check_channel(0, 3000.0f, 60000);
    TEST_ASSERT_EQUAL(PROT_LEVEL_TRIPPED_TIMED, lvl);
    TEST_ASSERT_EQUAL_UINT8(0, mosfet_driver_get_status(0)); // Power cut off!
    TEST_ASSERT_TRUE(fault_manager_is_channel_locked(0));   // Locked out!
    TEST_ASSERT_TRUE(fault_manager_is_high_fault_active());

    // Attempted re-enable must be blocked by safety layer
    mosfet_driver_set_channel(0, true);
    TEST_ASSERT_EQUAL_UINT8(0, mosfet_driver_get_status(0));
}

// --------------------------------------------------------------------------
// RANGE 2 HYSTERESIS: Timer continues if current drops to 125% (> 110%)
// --------------------------------------------------------------------------
void test_protection_range2_hysteresis_does_not_reset_if_above_110_percent(void) {
    // Start timer at t = 0 ms with 3000 mA (150%)
    protection_check_channel(0, 3000.0f, 0);

    // At t = 20000 ms, current drops to 2500 mA (125%, > 110% threshold)
    protection_level_t lvl = protection_check_channel(0, 2500.0f, 20000);
    TEST_ASSERT_EQUAL(PROT_LEVEL_TIMER_140_170, lvl);
    TEST_ASSERT_TRUE(protection_is_timer_running(0)); // Must NOT reset timer!

    // At t = 60000 ms, current is still at 2500 mA (> 110%) -> Trips!
    lvl = protection_check_channel(0, 2500.0f, 60000);
    TEST_ASSERT_EQUAL(PROT_LEVEL_TRIPPED_TIMED, lvl);
    TEST_ASSERT_EQUAL_UINT8(0, mosfet_driver_get_status(0));
    TEST_ASSERT_TRUE(fault_manager_is_channel_locked(0));
}

// --------------------------------------------------------------------------
// RANGE 3: Instantaneous Trip when Current > 170% of Nominal
// --------------------------------------------------------------------------
void test_protection_range3_instant_trip_above_170_percent(void) {
    // Channel 1: Nominal 2000 mA -> 170% = 3400 mA
    // Current at 3600 mA (> 170%): Immediate Trip
    protection_level_t lvl = protection_check_channel(1, 3600.0f, 100);
    TEST_ASSERT_EQUAL(PROT_LEVEL_TRIPPED_INSTANT, lvl);
    TEST_ASSERT_EQUAL_UINT8(0, mosfet_driver_get_status(1)); // Power cut off instantly!
    TEST_ASSERT_TRUE(fault_manager_is_channel_locked(1));
    TEST_ASSERT_TRUE(fault_manager_is_high_fault_active());

    // Re-enable must be blocked
    mosfet_driver_set_channel(1, true);
    TEST_ASSERT_EQUAL_UINT8(0, mosfet_driver_get_status(1));
}

// --------------------------------------------------------------------------
// TELEMETRY & CAN MASKS
// --------------------------------------------------------------------------
void test_protection_warning_and_timer_masks(void) {
    // Channel 0: 2300 mA (115% -> Range 1 Warning)
    // Channel 2: 3000 mA (150% -> Range 2 Timer Active)
    // Channel 4: 1500 mA (75% -> Range 0 Normal)
    protection_check_channel(0, 2300.0f, 1000);
    protection_check_channel(2, 3000.0f, 1000);
    protection_check_channel(4, 1500.0f, 1000);

    uint16_t warn_mask = protection_get_warning_mask();
    uint16_t timer_mask = protection_get_timer_active_mask();

    TEST_ASSERT_TRUE((warn_mask & (1U << 0)) != 0);
    TEST_ASSERT_TRUE((warn_mask & (1U << 2)) != 0);
    TEST_ASSERT_FALSE((warn_mask & (1U << 4)) != 0);

    TEST_ASSERT_FALSE((timer_mask & (1U << 0)) != 0);
    TEST_ASSERT_TRUE((timer_mask & (1U << 2)) != 0);
    TEST_ASSERT_FALSE((timer_mask & (1U << 4)) != 0);
}

void test_protection_inverter_persistence_3_samples(void) {
    // Channel 9 (Inverter): Nominal 2500 mA -> 170% = 4250 mA
    float severe_overcurrent = 4500.0f; // > 170%
    
    // Sample 1: Inrush tolerance
    TEST_ASSERT_EQUAL(PROT_LEVEL_WARNING_110, protection_check_channel(CANAL_INVERTER, severe_overcurrent, 100));
    TEST_ASSERT_EQUAL_UINT8(1, mosfet_driver_get_status(CANAL_INVERTER));
    
    // Sample 2: Inrush tolerance
    TEST_ASSERT_EQUAL(PROT_LEVEL_WARNING_110, protection_check_channel(CANAL_INVERTER, severe_overcurrent, 200));
    TEST_ASSERT_EQUAL_UINT8(1, mosfet_driver_get_status(CANAL_INVERTER));
    
    // Sample 3: Trips and Locks!
    TEST_ASSERT_EQUAL(PROT_LEVEL_TRIPPED_INSTANT, protection_check_channel(CANAL_INVERTER, severe_overcurrent, 300));
    TEST_ASSERT_EQUAL_UINT8(0, mosfet_driver_get_status(CANAL_INVERTER));
    TEST_ASSERT_TRUE(fault_manager_is_channel_locked(CANAL_INVERTER));
}

void test_protection_volant_persistence_3_samples(void) {
    // Channel 3 (Volant): Nominal 3000 mA -> 170% = 5100 mA
    float severe_overcurrent = 5500.0f; // > 170%
    
    // Samples 1 & 2: Safe
    TEST_ASSERT_EQUAL(PROT_LEVEL_WARNING_110, protection_check_channel(CANAL_VOLANT, severe_overcurrent, 100));
    TEST_ASSERT_EQUAL(PROT_LEVEL_WARNING_110, protection_check_channel(CANAL_VOLANT, severe_overcurrent, 200));
    TEST_ASSERT_EQUAL_UINT8(1, mosfet_driver_get_status(CANAL_VOLANT));
    
    // Sample 3: Trips and Locks
    TEST_ASSERT_EQUAL(PROT_LEVEL_TRIPPED_INSTANT, protection_check_channel(CANAL_VOLANT, severe_overcurrent, 300));
    TEST_ASSERT_EQUAL_UINT8(0, mosfet_driver_get_status(CANAL_VOLANT));
    TEST_ASSERT_TRUE(fault_manager_is_channel_locked(CANAL_VOLANT));
}

void test_protection_persistence_reset_on_recovery(void) {
    // 2 High samples on Inverter
    protection_check_channel(CANAL_INVERTER, 4500.0f, 100);
    protection_check_channel(CANAL_INVERTER, 4500.0f, 200);
    
    // 1 Normal sample -> Resets counter
    protection_check_channel(CANAL_INVERTER, 2000.0f, 300);
    
    // Next 2 High samples should still be tolerated
    TEST_ASSERT_EQUAL(PROT_LEVEL_WARNING_110, protection_check_channel(CANAL_INVERTER, 4500.0f, 400));
    TEST_ASSERT_EQUAL(PROT_LEVEL_WARNING_110, protection_check_channel(CANAL_INVERTER, 4500.0f, 500));
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
    RUN_TEST(test_protection_range1_warning_above_110_percent);
    RUN_TEST(test_protection_range2_timer_start_between_140_and_170_percent);
    RUN_TEST(test_protection_range2_timer_recovery_under_110_percent);
    RUN_TEST(test_protection_range2_timer_expired_trips_and_locks_channel);
    RUN_TEST(test_protection_range2_hysteresis_does_not_reset_if_above_110_percent);
    RUN_TEST(test_protection_range3_instant_trip_above_170_percent);
    RUN_TEST(test_protection_warning_and_timer_masks);
    RUN_TEST(test_protection_inverter_persistence_3_samples);
    RUN_TEST(test_protection_volant_persistence_3_samples);
    RUN_TEST(test_protection_persistence_reset_on_recovery);
    RUN_TEST(test_fault_manager_records_and_clearing);
    return UNITY_END();
}

