#include <unity.h>
#include "apps_driver.h"
#include "r2d_manager.h"
#include "torque_ctrl.h"
#include "fault_manager.h"

void setUp(void) {
    apps_driver_init(1, 2);
}
void tearDown(void) {}

void test_apps_plausible(void) {
    apps_driver_set_mock(400, 200); // 400 == 200 * 2
    uint16_t val;
    TEST_ASSERT_TRUE(apps_driver_read(&val));
    TEST_ASSERT_EQUAL(400, val);
}

void test_apps_implausible(void) {
    apps_driver_set_mock(400, 100); // 400 != 100 * 2 (out of bounds)
    uint16_t val;
    TEST_ASSERT_FALSE(apps_driver_read(&val));
}

extern "C" uint32_t mock_tick;

void test_r2d_transition(void) {
    r2d_manager_init();
    
    r2d_manager_update(true, false, false); // OFF -> WAITING_BRAKE
    TEST_ASSERT_EQUAL(R2D_STATE_WAITING_BRAKE, r2d_manager_get_state());
    
    r2d_manager_update(true, true, false); // WAITING_BRAKE -> WAITING_BUTTON
    TEST_ASSERT_EQUAL(R2D_STATE_WAITING_BUTTON, r2d_manager_get_state());
    
    r2d_manager_update(true, true, true); // WAITING_BUTTON -> SOUNDING
    TEST_ASSERT_EQUAL(R2D_STATE_SOUNDING, r2d_manager_get_state());
    
    mock_tick = 2500; // pass 2 seconds
    r2d_manager_update(true, true, false); // SOUNDING -> READY
    TEST_ASSERT_EQUAL(R2D_STATE_READY, r2d_manager_get_state());
}

void test_torque_ctrl_no_r2d(void) {
    TEST_ASSERT_EQUAL(0, torque_ctrl_calculate(500, 100, false, false, 1000));
}

void test_torque_ctrl_brake_pressed(void) {
    TEST_ASSERT_EQUAL(0, torque_ctrl_calculate(500, 100, true, true, 1000));
}

void test_torque_ctrl_normal_map(void) {
    // 500 is 50%, mapped to 32767 -> ~16383
    int32_t t = torque_ctrl_calculate(500, 100, false, true, 1000);
    TEST_ASSERT_EQUAL(16383, t);
}

void test_torque_ctrl_slip_multiplier(void) {
    // 500 throttle -> 16383, but slip is 500 (50%), so torque should be ~8191
    int32_t t = torque_ctrl_calculate(500, 100, false, true, 500);
    TEST_ASSERT_EQUAL(8191, t);
}

void test_torque_ctrl_anti_kick(void) {
    // 1000 throttle -> 32767, but rpm is 10 (< 50), limit is 3276
    int32_t t = torque_ctrl_calculate(1000, 10, false, true, 1000);
    TEST_ASSERT_EQUAL(3276, t);
}

void test_bspd(void) {
    torque_ctrl_init();
    // BSPD active: Throttle > 25% (assuming 1023 is max, 25% is ~255) and brake is pressed
    int32_t torque = torque_ctrl_calculate(300, 1000, true, true, 100);
    TEST_ASSERT_EQUAL(0, torque);
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
    RUN_TEST(test_apps_plausible);
    RUN_TEST(test_apps_implausible);
    RUN_TEST(test_r2d_transition);
    RUN_TEST(test_bspd);
    RUN_TEST(test_torque_ctrl_no_r2d);
    RUN_TEST(test_torque_ctrl_brake_pressed);
    RUN_TEST(test_torque_ctrl_normal_map);
    RUN_TEST(test_torque_ctrl_slip_multiplier);
    RUN_TEST(test_torque_ctrl_anti_kick);
    RUN_TEST(test_fault_manager);
    return UNITY_END();
}
