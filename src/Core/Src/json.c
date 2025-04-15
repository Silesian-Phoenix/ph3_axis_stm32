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
        const lwjson_token_t *token1 = lwjson_find(&lwjson, "angle");
        const lwjson_token_t *token2 = lwjson_find(&lwjson, "setZero");
        const lwjson_token_t *token3 = lwjson_find(&lwjson, "returnAngle");

        const lwjson_token_t *token4 = lwjson_find(&lwjson, "Arm");
        const lwjson_token_t *token5 = lwjson_find(&lwjson, "DisArm");

        const lwjson_token_t *token6 = lwjson_find(&lwjson, "setAddr");
        const lwjson_token_t *token7 = lwjson_find(&lwjson, "getAddr");


        if (token1 != NULL) {
            int data = lwjson_get_val_int(token1);

            if (data <= MAX_LEFT && data >= MAX_RIGHT) {
                MOTOR_target_angle = data;
                MOTOR_current_status = MOTOR_ANGLE_RECEIVED;
            }
        }
        else if (token2 != NULL) {
            int data = lwjson_get_val_int(token2);
            if (data) {
                ENCODER_init = false;
            }
            // MOTOR_target_angle = 0;
        }

        else if (token3 != NULL) {
            int data = lwjson_get_val_int(token3);
            if (data) {
                data_to_send = true;
            }
        }

        // Arm
        else if (token4 != NULL)
        {
            int data = lwjson_get_val_int(token4);
            if (data)
            {
                set_enable = true;
            }
        }

        // DisArm
        else if (token5 != NULL)
        {
            int data = lwjson_get_val_int(token5);
            if (data)
            {
                reset_enable = true;
            }
        }

                // setAddr
                else if (token6 != NULL) {
                    int data = lwjson_get_val_int(token6);
                    if (data >= 0) {
                        received_addr = data;
                        addr_to_set = true;
                    }
                }
                
                // getAddr
                else if (token7 != NULL) {
                    int data = lwjson_get_val_int(token7);
                    if (data) {
                        addr_to_send = true;
                    }
                }



    }
}

