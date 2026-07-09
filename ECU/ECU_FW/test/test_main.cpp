#include <unity.h>
#include <math.h>
#include "fan_driver.h"
#include "ads8688_driver.h"
#include "pid_ctrl.h"
#include "fault_manager.h"
#include "ota_service.h"

void setUp(void) {}
void tearDown(void) {}

void test_ads8688_bosch_ntc_conversion(void) {
    // 2500 Ohms corresponde a 20 °C
    float t20 = ads8688_driver_bosch_r2t(2500.0f);
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 20.0f, t20);

    // 834 Ohms corresponde a 50 °C
    float t50 = ads8688_driver_bosch_r2t(834.0f);
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 50.0f, t50);

    // Fuera de rango debe devolver NAN
    float t_invalid = ads8688_driver_bosch_r2t(0.0f);
    TEST_ASSERT_TRUE(isnan(t_invalid));
}

void test_fan_driver_esc_scaling(void) {
    fan_driver_init();
    
    // 0% -> 1000 us
    TEST_ASSERT_EQUAL_UINT16(1000, fan_driver_pct_to_us(0.0));
    // 100% -> 2000 us
    TEST_ASSERT_EQUAL_UINT16(2000, fan_driver_pct_to_us(100.0));
    // 50% -> 1140 + 0.5 * (2000 - 1140) = 1570 us
    TEST_ASSERT_EQUAL_UINT16(1570, fan_driver_pct_to_us(50.0));

    // Conversión a Duty LEDC a 50Hz 14 bits (16383 max)
    uint32_t duty_min = fan_driver_us_to_duty(1000);
    TEST_ASSERT_EQUAL_UINT32(819, duty_min); // 1000 * 16383 * 50 / 1e6 = 819
}

void test_fan_driver_slew_rate(void) {
    double current = 0.0;
    double target = 100.0;
    
    // En 100 ms (0.1 s) a 20%/s el incremento máximo es 2.0%
    current = fan_driver_slew_pct(current, target, 0.1);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 2.0f, (float)current);

    // Si ya está cerca del objetivo, salta directamente
    current = 99.5;
    current = fan_driver_slew_pct(current, target, 0.1);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 100.0f, (float)current);
}

void test_pid_cooling_reverse_action(void) {
    pid_ctrl_t pid;
    // Setpoint 45°C, Acción REVERSA
    pid_ctrl_init(&pid, 2.0, 0.1, 0.0, 45.0, PID_DIRECTION_REVERSE);

    // Si la temperatura es menor que el setpoint (30°C), salida debe ser 0%
    double out_cold = pid_ctrl_compute(&pid, 30.0, 1.0);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.0f, (float)out_cold);

    // Si la temperatura sube a 55°C (error = +10°C), salida debe subir proporcionalmente
    double out_hot = pid_ctrl_compute(&pid, 55.0, 1.0);
    TEST_ASSERT_TRUE(out_hot > 20.0);
}

void test_fault_manager_failsafe_escalation(void) {
    fault_manager_init();
    TEST_ASSERT_FALSE(fault_manager_is_high_fault_active());
    TEST_ASSERT_FALSE(fault_manager_is_failsafe_active());

    // Reporte de fallo en NTC activa failsafe
    fault_manager_report(FAULT_CAT_HARDWARE, FAULT_PRIORITY_HIGH, FAULT_CODE_MOTOR_NTC_FAIL);
    fault_manager_set_failsafe(true);

    TEST_ASSERT_TRUE(fault_manager_is_high_fault_active());
    TEST_ASSERT_TRUE(fault_manager_is_failsafe_active());

    fault_record_t rec = fault_manager_get_last_fault();
    TEST_ASSERT_EQUAL_UINT32(FAULT_CODE_MOTOR_NTC_FAIL, rec.code);
    TEST_ASSERT_EQUAL_INT(FAULT_PRIORITY_HIGH, rec.priority);
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_ads8688_bosch_ntc_conversion);
    RUN_TEST(test_fan_driver_esc_scaling);
    RUN_TEST(test_fan_driver_slew_rate);
    RUN_TEST(test_pid_cooling_reverse_action);
    RUN_TEST(test_fault_manager_failsafe_escalation);
    return UNITY_END();
}
