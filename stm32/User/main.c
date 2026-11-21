#include "stm32f10x.h"// Device header
#include "OLED.h"
#include "PWM.h"
#include "Serial.h"
#include "Servo.h"

float Angle1 = 90.0, Angle2 = 110.0;   // 水平、俯仰初始角度

int main(void)
{
	uint16_t TelemCnt = 0;

	Serial_Init();
	Serial2_Init();
	OLED_Init();
	Servo_Init();
	Servo1_SetAngle(Angle1);
	Servo2_SetAngle(Angle2);

	OLED_ShowString(1, 1, "H:");
	OLED_ShowString(2, 1, "V:");

	/* 上电自检：先发一行，确认 USB-TTL 接收链路通（不需要 OpenMV 发帧） */
	Serial2_Printf("A1=%d.%d A2=%d.%d\n",
		(int)Angle1, (int)(Angle1 * 10) % 10,
		(int)Angle2, (int)(Angle2 * 10) % 10);

	while(1)
	{
		/* 收到完整一帧（2 字节命令 + '\n'）才执行，一帧 = 两轴各走一步 */
		if(Serial_GetFrameFlag() == 1)
		{
			uint8_t h = Serial_GetFrameByte(0);   // 水平命令
			uint8_t v = Serial_GetFrameByte(1);   // 垂直命令

			OLED_ShowChar(1, 3, h);
			OLED_ShowChar(2, 3, v);

			/* 水平：a=快右 b=慢右 c=快左 d=慢左 o=不动 */
			if(h == 'a')      Angle1 -= 1.2f;     // 原来是 -=
			else if(h == 'b') Angle1 -= 0.3f;     // 原来是 -=
			else if(h == 'c') Angle1 += 1.2f;     // 原来是 +=
			else if(h == 'd') Angle1 += 0.3f;     // 原来是 +=

			/* 垂直：x=快下 z=慢下 y=快上 w=慢上 o=不动 */
			if(v == 'x')      Angle2 -= 1.0f;
			else if(v == 'z') Angle2 -= 0.2f;
			else if(v == 'y') Angle2 += 1.0f;
			else if(v == 'w') Angle2 += 0.2f;

			/* 角度限幅 */
			if(Angle1 < 0)   Angle1 = 0;
			if(Angle1 > 180) Angle1 = 180;
			if(Angle2 < 0)   Angle2 = 0;
			if(Angle2 > 180) Angle2 = 180;

			Servo1_SetAngle(Angle1);
			Servo2_SetAngle(Angle2);

			/* 每 2 帧往电脑发一次角度，供 Qserial 波形窗口画曲线 */
			if(++TelemCnt >= 2)
			{
				TelemCnt = 0;
				Serial2_Printf("A1=%d.%d A2=%d.%d\n",
					(int)Angle1, (int)(Angle1 * 10) % 10,
					(int)Angle2, (int)(Angle2 * 10) % 10);
			}
		}
	}
}
