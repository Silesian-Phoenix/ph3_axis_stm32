#pragma once

#include "main.h"
#include "lwjson.h"

#define JSON_BUFFER_MAX 32

char rx_buffer[JSON_BUFFER_MAX];
lwjson_token_t tokens[2];
lwjson_t lwjson;

void lwjson_my_init();
void json_process(char* json_str);

