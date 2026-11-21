#include "stm32f10x.h"                  // Device header
#include <stdio.h>
#include <stdarg.h>
/* 帧接收：每帧 2 字节命令 + '\n' 帧尾 */
volatile uint8_t Serial_Frame[2];     // [0] = 水平命令，[1] = 垂直命令
volatile uint8_t Serial_FrameFlag;    // 1 = 收到一帧新的完整指令

void Serial_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 ;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 ;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	
	USART_InitTypeDef USART_InitStructure;
	USART_InitStructure.USART_BaudRate = 9600;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Tx |USART_Mode_Rx;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	
	USART_Init(USART1,&USART_InitStructure);
	
	USART_ITConfig(USART1,USART_IT_RXNE,ENABLE);
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel =USART1_IRQn ;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_Init(&NVIC_InitStructure);
	USART_Cmd(USART1,ENABLE);
	
	
	

}

void Serial_SendByte(uint8_t Byte)
{
	USART_SendData(USART1,Byte);
	while(USART_GetFlagStatus(USART1,USART_FLAG_TXE) == RESET);
}
void Serial_SendArray(uint8_t *Array, uint8_t Length)
{
	int i;
	for( i = 0;i < Length ;i++){
		Serial_SendByte(Array[i]);
	}
}
void Serial_SendString(char *String)
{
	for(int i = 0;String[i] != 0;i++){
		Serial_SendByte(String[i]);
	}
}
uint32_t Serial_Pow(uint32_t X,uint32_t Y)
{
	uint32_t result = 1;
	while(Y --)
	{
		result *= X;
	}
	return result;
	
}
void Serial_SendNumber(uint32_t Number, uint8_t Length)
{
	uint8_t i;
	uint8_t Byte;
	for(i = 0; i < Length;i++)
	{
		Byte = (Number / Serial_Pow(10,Length-i-1))%10;
		Serial_SendByte(Byte + 0x30);	
	}	
}
int fputc(int ch,FILE *f)  //printf底层函数重映射
{
	Serial_SendByte(ch);
	return ch;
}
void Serial_Printf(char *format, ...)
{
	char String[100];
	va_list arg;
	va_start(arg, format);
	vsprintf(String,format,arg);
	va_end(arg);
	Serial_SendString(String);
}
uint8_t Serial_GetFrameFlag(void)
{
	if(Serial_FrameFlag == 1)
	{
		Serial_FrameFlag = 0;
		return 1;
	}
	return 0;
}
uint8_t Serial_GetFrameByte(uint8_t Index)
{
	return Serial_Frame[Index];
}
void USART1_IRQHandler(void)
{
	static uint8_t Buf[2];
	static uint8_t Idx = 0;
	if(USART_GetITStatus(USART1,USART_IT_RXNE) == SET)
	{
		uint8_t Byte = USART_ReceiveData(USART1);
		if(Byte == '\n')              // 帧尾：长度正好 2 字节才有效
		{
			if(Idx == 2)
			{
				Serial_Frame[0] = Buf[0];
				Serial_Frame[1] = Buf[1];
				Serial_FrameFlag = 1;
			}
			Idx = 0;                  // 好坏帧都重新对齐
		}
		else if(Byte == '\r')
		{
			/* 忽略回车：兼容串口助手默认发送的 \r\n 换行 */
		}
		else if(Idx < 2)
		{
			Buf[Idx++] = Byte;
		}
		else
		{
			Idx = 3;                  // 多字节：本帧作废，等帧尾重置
		}
		USART_ClearITPendingBit(USART1,USART_IT_RXNE);
	}
}

/* ================= USART2：给电脑看波形（PA2 = TX，115200） ================= */
void Serial2_Init(void)
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	USART_InitTypeDef USART_InitStructure;
	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Tx;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_Init(USART2, &USART_InitStructure);
	USART_Cmd(USART2, ENABLE);
}
void Serial2_SendByte(uint8_t Byte)
{
	USART_SendData(USART2, Byte);
	while(USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);
}
void Serial2_SendString(char *String)
{
	int i;
	for(i = 0; String[i] != 0; i++)
	{
		Serial2_SendByte(String[i]);
	}
}
void Serial2_Printf(char *format, ...)
{
	char String[80];
	va_list arg;
	va_start(arg, format);
	vsprintf(String, format, arg);
	va_end(arg);
	Serial2_SendString(String);
}

	
	
