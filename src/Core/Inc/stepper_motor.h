#pragma once

#include "main.h"

typedef enum {  
    MOTOR_IDLE, 
    MOTOR_ANGLE_RECEIVED,
    MOTOR_IN_MOTION,
    MOTOR_AT_POSITION,
} MOTOR_Status;

typedef enum {
    DIR_RIGHT, // FROM THE TOP 
    DIR_LEFT
} MOTOR_Direction; 

void MOTOR_set_direction(MOTOR_Direction dir);
void MOTOR_choice_direction(uint16_t current_angle, uint16_t target_angle);
void MOTOR_go_to(uint16_t current_angle, uint16_t target_angle);

extern MOTOR_Status MOTOR_current_status;
extern MOTOR_Direction MOTOR_current_dir;
extern uint16_t MOTOR_current_angle;
extern uint16_t MOTOR_received_angle;
extern uint16_t MOTOR_target_angle;
extern uint16_t MOTOR_step;
extern uint16_t MOTOR_target_step;
