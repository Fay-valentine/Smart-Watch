//
// Created by a1191 on 2026/11/13.
//

#ifndef INC_10_WATCH_SERIAL_COMMAND_H
#define INC_10_WATCH_SERIAL_COMMAND_H

#include <stdint.h>

extern char pending_cmd[64];

void SerialCommand_CopyFromISR(char* data, uint16_t size);
void Serial_cmd_execute(char* cmd);
#endif //INC_10_WATCH_SERIAL_COMMAND_H