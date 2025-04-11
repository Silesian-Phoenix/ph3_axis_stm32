#include "stepper_motor.h"
#include "tim.h"

MOTOR_Status MOTOR_current_status = MOTOR_IDLE;
MOTOR_Direction MOTOR_current_dir = DIR_CW;
double MOTOR_current_angle = 0;
uint16_t MOTOR_received_angle = 0;
uint16_t MOTOR_target_angle = 0;
uint16_t MOTOR_step = 0;
bool MOTOR_rot = false;

#define ABS(x) ((x) < 0 ? -(x) : (x))

/*
Funkcja sprawdzająca czy pozycja silnika jest w zakresie marginesu błędu
1 - kąt mieści się w marginesie błędu
0 - kąt nie mieści się w marginesie
*/

void MOTOR_set_direction(MOTOR_Direction dir) {
    if (dir == DIR_CW) {
        HAL_GPIO_WritePin(MOTOR_DIR_GPIO_Port, MOTOR_DIR_Pin, GPIO_PIN_RESET);
    }
    else if (dir == DIR_CCW) {
        HAL_GPIO_WritePin(MOTOR_DIR_GPIO_Port, MOTOR_DIR_Pin, GPIO_PIN_SET);
    }
}

int accept_margin(uint16_t current_angle, uint16_t target_angle, double acceptable_margin) {
    double current_margin = ABS(current_angle - target_angle);
    if (current_margin < acceptable_margin) {
        return 1;
    }
    else {
        return 0;
    }
}

/*
180-115 źle
*/

/*
MOTOR_Direction MOTOR_choice_direction(uint16_t current_angle, uint16_t target_angle) {

    // 0-180
    if (current_angle <= 180) {
        int antipodal_point = current_angle + 180;
        
        // 0-90
        if (current_angle <= 90) {
            if (target_angle > current_angle && target_angle < antipodal_point) {
                return DIR_CCW; // git
            }
            else {
                return DIR_CW;
            }
        }

        // 91-180
        else if (current_angle < 180) {
            if (target_angle > current_angle && target_angle < antipodal_point) {
                return DIR_CCW;
            }
            else {
                return DIR_CW;
            }
        }
    }

    // 180-360
    else if (current_angle > 180) {
        int antipodal_point = current_angle - 180;
        // if (antipodal_point == target_angle) {
        // return DIR_FAULT;
        // }
        // 181-270
        if (current_angle <= 270) {
            if (target_angle < current_angle && target_angle > antipodal_point) { // poprawka
                return DIR_CW;
            }
            else {
                return DIR_CCW;
            }
        }

        // 271-360
        else {
            if (target_angle < current_angle && target_angle > antipodal_point) {
                return DIR_CW;
            }
            else {
                return DIR_CCW;
            }
        }
    }
    
}
*/

MOTOR_Direction MOTOR_choice_direction(uint16_t current_angle, uint16_t target_angle) {
    int antipodal_point = (current_angle + 180) % 360;

    if ((current_angle < antipodal_point && target_angle > current_angle && target_angle < antipodal_point) ||
        (current_angle > antipodal_point && (target_angle > current_angle || target_angle < antipodal_point))) {
        return DIR_CCW;
    } else {
        return DIR_CW;
    }
}


void MOTOR_state_machine(uint16_t current_angle, uint16_t target_angle, uint8_t state) {
    switch (state) {
        case MOTOR_IDLE:
            MOTOR_current_status = MOTOR_ANGLE_RECEIVED;
            break;
        case MOTOR_ANGLE_RECEIVED:
            // ustawienie kierunku obrotu
            MOTOR_current_dir = MOTOR_choice_direction(current_angle, target_angle);
            if (MOTOR_current_dir != DIR_FAULT) {
                MOTOR_set_direction(MOTOR_current_dir);
            }
            HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);
            MOTOR_current_status = MOTOR_IN_MOTION;
            MOTOR_rot = true;
            break;

        case MOTOR_IN_MOTION:
            
            // jesli silnik dotarl gdzie mial
            if (accept_margin(current_angle, target_angle, MOTOR_PROPER_ANGLE_MARGIN)) {
                MOTOR_current_status = MOTOR_AT_POSITION;
                MOTOR_rot = false;
                HAL_TIM_PWM_Stop(&htim2, TIM_CHANNEL_2);
            }
            break;

        case MOTOR_AT_POSITION:
            // jesli silnik wyszedl z pozycji
            if (accept_margin(current_angle, target_angle, MOTOR_PROPER_ANGLE_MARGIN) != 1) {
                MOTOR_current_status = MOTOR_OUT;
            }
            break;
        case MOTOR_OUT:
            // ustawienie kierunku obrotu
            MOTOR_current_dir = MOTOR_choice_direction(current_angle, target_angle);
            if (MOTOR_current_dir != DIR_FAULT) {
                MOTOR_set_direction(MOTOR_current_dir);
            }

            MOTOR_current_status = MOTOR_IN_MOTION;
            MOTOR_rot = true;
            HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);
            break;
    }
}
