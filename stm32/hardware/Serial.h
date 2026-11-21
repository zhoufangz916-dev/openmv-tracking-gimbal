#ifndef __SERIAL_H
#define __SERIAL_H
#include <stdio.h>
#include <stdarg.h>

void Serial_Init(void);
void Serial_SendByte(uint8_t Byte);
void Serial_SendArray(uint8_t *Array, uint8_t Length);
void Serial_SendString(char *String);
void Serial_SendNumber(uint32_t Number, uint8_t Length);
void Serial_Printf(char *format, ...);
uint8_t Serial_GetFrameFlag(void);
uint8_t Serial_GetFrameByte(uint8_t Index);

/* USART2：接 USB-TTL 给电脑看波形用（PA2 = TX，115200） */
void Serial2_Init(void);
void Serial2_SendByte(uint8_t Byte);
void Serial2_SendString(char *String);
void Serial2_Printf(char *format, ...);


#endif
