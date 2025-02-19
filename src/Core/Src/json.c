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
    if (lwjson_parse(&lwjson, json_str) == lwjsonOK) {
        const lwjson_token_t *token = lwjson_find(&lwjson, "angle");
        const lwjson_token_t *token1 = lwjson_find(&lwjson, "setZero");

        if (token != NULL) {
            int data = lwjson_get_val_int(token);

            if ((data >= 0 && data <= MAX_RIGHT) || (data >= MAX_LEFT && data <= 360)) {
                MOTOR_target_angle = data;
                MOTOR_current_status = MOTOR_ANGLE_RECEIVED;
            }

            char buf_to_send[100];
            snprintf(buf_to_send, sizeof(buf_to_send), "%s-%d", json_str, data);

            if (!uart2_tx_busy) {
                HAL_UART_Transmit_DMA(&huart2, (uint8_t*)buf_to_send, strlen(buf_to_send));
                uart2_tx_busy = true;
            }
        } 
        else if (token1 != NULL) {
            int data = lwjson_get_val_int(token1);

            char buf_to_send[50];
            snprintf(buf_to_send, sizeof(buf_to_send), "%s-%d", json_str, data);

            if (!uart2_tx_busy) {
                HAL_UART_Transmit_DMA(&huart2, (uint8_t*)buf_to_send, strlen(buf_to_send));
                uart2_tx_busy = true;
            }
        }
    }
}

/*
void json_process(char *json_str) {
    if (lwjson_parse(&lwjson, (char*)json_str) == lwjsonOK) {
        const lwjson_token_t *token = lwjson_find(&lwjson, "angle");
        if (token) {
        int data = lwjson_get_val_int(token);

        // dane dla silnika
        // MOTOR_received_angle = data;
        if ((data >= 0 && data <= MAX_RIGHT) || (data >= MAX_LEFT && data <= 360)) {
            MOTOR_target_angle = data;
            MOTOR_current_status = MOTOR_ANGLE_RECEIVED; // zmiana statusu
        }
        

        // TO DO: odrzucenie danych poza zakresem

        // ------------------------------------------------------------
        // wysłanie wiadomości zwrotnej
        char buf_to_send[50];
        sprintf(buf_to_send, "json: %s -> %d", json_str, MOTOR_target_angle);
        if (!uart2_tx_busy) {
            HAL_UART_Transmit_DMA(&huart2, (uint8_t*)buf_to_send, strlen(buf_to_send));
            uart2_tx_busy = true;
        }
        // ------------------------------------------------------------
        }
    }
}
*/


/*
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size) {
    if (huart == &huart2) {
        if (Size < sizeof(rx_buffer)) {
            rx_buffer[Size] = '\0';
        } else {
            rx_buffer[sizeof(rx_buffer) - 1] = '\0';
        }
  
        uart2_data_received = true;
  
        if (uart2_data_received) {
          json_process(rx_buffer);
          uart2_data_received = false;
        }
  
        HAL_UARTEx_ReceiveToIdle_DMA(&huart2, (uint8_t*)rx_buffer, sizeof(rx_buffer));
    }
  }
  
  void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
    if (huart == &huart2) {
      uart2_tx_busy = false;
    }
  }
  
  */