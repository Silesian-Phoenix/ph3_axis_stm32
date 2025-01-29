#include "stepper_motor.h"
#include "tim.h"

MOTOR_Status MOTOR_current_status = MOTOR_IDLE;
MOTOR_Direction MOTOR_current_dir = DIR_RIGHT;
uint16_t MOTOR_current_angle = 0;
uint16_t MOTOR_received_angle = 0;
uint16_t MOTOR_target_angle = 0;
uint16_t MOTOR_step = 0;

#define MOTOR_PROPER_ANGLE_MARGIN 1
// JAKIES MAKRO DO LICZENIA ROZNICY

/*to do: dostosować jak to ma się obracać*/
void MOTOR_set_direction(MOTOR_Direction dir) {
    if (dir == DIR_RIGHT) {
        HAL_GPIO_WritePin(MOTOR_DIR_GPIO_Port, MOTOR_DIR_Pin, GPIO_PIN_SET); // prawwo - wlaczona
    }
    else if (dir == DIR_LEFT) {
        HAL_GPIO_WritePin(MOTOR_DIR_GPIO_Port, MOTOR_DIR_Pin, GPIO_PIN_RESET);
    }
}


/*
Funkcja sprawdzająca czy pozycja silnika jest w zakresie marginesu błędu
zalozenie: margines to 10*
*/
// uint16_t check_margin(uint16_t current_angle, uint16_t target_angle, uint16_t margin) {

// }

void MOTOR_choice_direction(uint16_t current_angle, uint16_t target_angle) {
    if (current_angle >= 0 && current_angle <= 360 && target_angle >= 0 && target_angle <= 360) {
        return;
    }

    // 0-90
    if (current_angle < 180) {
        int antipodal_point = current_angle + 180;
        if (antipodal_point == target_angle) {
        return;
        }

        if (current_angle < 90) {
            if (target_angle > current_angle && target_angle < antipodal_point) {
            MOTOR_current_dir = DIR_RIGHT;
            }
            else {
            MOTOR_current_dir = DIR_LEFT;
            }
        }

        // 91-180
        else if (current_angle < 180) {
        if (target_angle > current_angle && target_angle < antipodal_point) {
            MOTOR_current_dir = DIR_LEFT;
        }
        else {
            MOTOR_current_dir = DIR_RIGHT;
        }
        }
    }

    else if (current_angle > 180) {
        int antipodal_point = current_angle - 180;
        if (antipodal_point == target_angle) {
        return;
        }
        // 181-270
        if (current_angle < 270) {
        if (target_angle < current_angle && target_angle > antipodal_point) {
            MOTOR_current_dir = DIR_RIGHT;
        }
        else {
            MOTOR_current_dir = DIR_LEFT;
        }
        }

        // 271-360
        else {
        if (target_angle < current_angle && target_angle > antipodal_point) {
            MOTOR_current_dir = DIR_LEFT;
        }
        else {
            MOTOR_current_dir = DIR_RIGHT;
        }
        }
    }
}

// narazie obort o zadany kąt
void MOTOR_go_to(uint16_t current_angle, uint16_t target_angle) {
    MOTOR_choice_direction(MOTOR_current_angle, target_angle); // wybranie kierunku obrotu
    MOTOR_set_direction(MOTOR_current_dir); // ustawienie kierunku obrotu
    MOTOR_target_angle = target_angle;
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);

    MOTOR_current_status = MOTOR_IN_MOTION; // zmiana statusu
}

