#include "stm32f10x.h"                  // Device header
#include "PWM.h"
void Servo_Init(void)
{
	PWM_Init();
}

void Servo1_SetAngle(float Angle)
{
	//PWM_SetCompare2(Angle / 180 *2000 +500);
	PWM_SetCompare1(Angle / 180 *2000 +500);
}
void Servo2_SetAngle(float Angle)
{
	PWM_SetCompare2(Angle / 180 *2000 +500);
	//PWM_SetCompare1(Angle / 180 *2000 +500);
}
