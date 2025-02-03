#pragma once

#include "main.h"
#include "stdbool.h"

#define MOTOR_PROPER_ANGLE_MARGIN 1.7 // bo krok 1.8
// #define MOTOR_PROPER_ANGLE_MARGIN (360.0 / 256.0)

typedef enum {  
    MOTOR_IDLE, // stan po resecie
    MOTOR_ANGLE_RECEIVED, // otzymano kąt
    MOTOR_IN_MOTION, // silnik w trakcie ustawiania na pozycję (rownież podczas korekty)
    MOTOR_AT_POSITION, // silnik na pozycji
    MOTOR_OUT
} MOTOR_Status;

typedef enum {
    DIR_RIGHT, // FROM THE TOP 
    DIR_LEFT,
    DIR_FAULT
} MOTOR_Direction; 

MOTOR_Direction MOTOR_choice_direction(uint16_t current_angle, uint16_t target_angle);
void MOTOR_set_direction(MOTOR_Direction dir);
void MOTOR_state_machine(uint16_t current_angle, uint16_t target_angle, uint8_t state);
int accept_margin(uint16_t current_angle, uint16_t target_angle, double acceptable_margin);

extern MOTOR_Status MOTOR_current_status;
extern MOTOR_Direction MOTOR_current_dir;
extern bool MOTOR_rot;
extern uint16_t MOTOR_current_angle;
extern uint16_t MOTOR_received_angle;
extern uint16_t MOTOR_target_angle;
extern uint16_t MOTOR_step;
extern uint16_t MOTOR_target_step;