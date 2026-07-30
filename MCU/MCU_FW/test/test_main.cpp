#include <unity.h>
#include "apps_driver.h"
#include "r2d_manager.h"
#include "torque_ctrl.h"

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

void test_bspd(void) {
    torque_ctrl_init();
    // BSPD active: Throttle > 25% (assuming 1023 is max, 25% is ~255) and brake is pressed
    int32_t torque = torque_ctrl_calculate(300, 1000, true, true, 100);
    TEST_ASSERT_EQUAL(0, torque);
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_apps_plausible);
    RUN_TEST(test_apps_implausible);
    RUN_TEST(test_r2d_transition);
    RUN_TEST(test_bspd);
    return UNITY_END();
}
