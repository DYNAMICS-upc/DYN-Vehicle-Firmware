#include <unity.h>
#include "r2d_manager.h"
#include "torque_ctrl.h"
#include "fault_manager.h"

void setUp(void) {
    fault_manager_init();
    r2d_manager_init();
    torque_ctrl_init();
}

void tearDown(void) {}

void test_r2d_transition(void) {
    r2d_manager_init();
    fault_manager_init();
    
    r2d_manager_update(true, false, false); // OFF -> WAITING_BRAKE
    TEST_ASSERT_EQUAL(R2D_STATE_WAITING_BRAKE, r2d_manager_get_state());
    
    r2d_manager_update(true, true, false); // WAITING_BRAKE -> WAITING_BUTTON
    TEST_ASSERT_EQUAL(R2D_STATE_WAITING_BUTTON, r2d_manager_get_state());
}

void test_r2d_blocked_by_critical_fault(void) {
    r2d_manager_init();
    fault_manager_init();

    // Si hay un fallo crítico, R2D debe forzarse a OFF y rechazar armarse
    fault_manager_report(FAULT_CAT_HARDWARE, FAULT_PRIORITY_HIGH, FAULT_CODE_APPS_IMPLAUSIBLE);
    r2d_manager_update(true, true, true);
    TEST_ASSERT_EQUAL(R2D_STATE_OFF, r2d_manager_get_state());
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

void test_torque_cut_on_critical_fault(void) {
    fault_manager_report(FAULT_CAT_HARDWARE, FAULT_PRIORITY_HIGH, FAULT_CODE_APPS_IMPLAUSIBLE);
    
    // Acelerando al 100% con R2D activo -> Debe cortar par a 0 de inmediato
    int32_t t = torque_ctrl_calculate(1000, 1000, false, true, 1000);
    TEST_ASSERT_EQUAL(0, t);
}

void test_bspd(void) {
    // BSPD active: Throttle > 25% and brake is pressed
    int32_t torque = torque_ctrl_calculate(300, 1000, true, true, 100);
    TEST_ASSERT_EQUAL(0, torque);
}

void test_fault_manager_locking_and_dtc(void) {
    TEST_ASSERT_FALSE(fault_manager_is_high_fault_active());
    
    // Low priority should not trigger high fault state
    fault_manager_report(FAULT_CAT_RESOURCES, FAULT_PRIORITY_LOW, 1);
    TEST_ASSERT_FALSE(fault_manager_is_high_fault_active());
    
    // Bloqueo de subsistema APPS
    fault_manager_lock_subsystem(FAULT_SUBSYS_APPS);
    TEST_ASSERT_TRUE(fault_manager_is_high_fault_active());
    TEST_ASSERT_TRUE(fault_manager_is_subsystem_locked(FAULT_SUBSYS_APPS));
    
    fault_record_t rec = fault_manager_get_last_fault();
    (void)rec;
    TEST_ASSERT_EQUAL_UINT32(FAULT_SUBSYS_APPS, fault_manager_get_locked_subsystems());

    fault_manager_clear_all();
    TEST_ASSERT_FALSE(fault_manager_is_high_fault_active());
}

int main(int argc, char **argv) {
    (void)argc;
    (void)argv;
    UNITY_BEGIN();
    RUN_TEST(test_r2d_transition);
    RUN_TEST(test_r2d_blocked_by_critical_fault);
    RUN_TEST(test_bspd);
    RUN_TEST(test_torque_ctrl_no_r2d);
    RUN_TEST(test_torque_ctrl_brake_pressed);
    RUN_TEST(test_torque_ctrl_normal_map);
    RUN_TEST(test_torque_ctrl_slip_multiplier);
    RUN_TEST(test_torque_ctrl_anti_kick);
    RUN_TEST(test_torque_cut_on_critical_fault);
    RUN_TEST(test_fault_manager_locking_and_dtc);
    return UNITY_END();
}
