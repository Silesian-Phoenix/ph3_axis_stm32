// #include "stepper_motor.h"
#include "usart.h"
#include "json.h"
#include "stdbool.h"
#include "stdio.h"
#include "stepper_motor.h"

void lwjson_my_init() {
    lwjson_init(&lwjson, tokens, LWJSON_ARRAYSIZE(tokens));
}

void json_process(char *json_str) {
    if (lwjson_parse(&lwjson, (char*)json_str) == lwjsonOK) {
        const lwjson_token_t *token = lwjson_find(&lwjson, "angle");
        if (token) {
        int data = lwjson_get_val_int(token);

        // dane dla silnika
        MOTOR_received_angle = data;
        MOTOR_target_angle = MOTOR_received_angle;
        MOTOR_current_status = MOTOR_ANGLE_RECEIVED; // zmiana statusu

        // ------------------------------------------------------------
        // wysłanie wiadomości zwrotnej
        char buf_to_send[50];
        sprintf(buf_to_send, "json: %s -> %d", json_str, MOTOR_received_angle);
        if (!uart2_tx_busy) {
            HAL_UART_Transmit_DMA(&huart2, (uint8_t*)buf_to_send, strlen(buf_to_send));
            uart2_tx_busy = true;
        }
        // ------------------------------------------------------------
        }
    }
}

