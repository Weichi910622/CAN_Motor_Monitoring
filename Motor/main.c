#include "stm32f10x.h"
#include "Delay.h"
#include "OLED.h"
#include "Motor.h"
#include "RedSensor.h"
#include "Timer.h"
#include "MyCAN.h"

float Kp = 0.015; 
float Ki = 0.03; 
float Kd = 0.04; 

float integral_error = 0; 
float previous_error = 0; 

int16_t speed; 
int16_t RPM_magnitude;
uint8_t keycount = 0;
uint8_t TimingFlag;
int16_t TARGET_RPM;
volatile uint16_t count;
volatile int16_t RPM;
int16_t real_rpm;

void RPM_Sign(uint8_t Line, uint8_t Column, int16_t Number, uint8_t Length)
{
	if (Number < 0)
	{
		OLED_ShowChar(Line, Column, '-');
		OLED_ShowNum(Line, Column + 1, -Number, Length);
	}
	else
	{
		OLED_ShowChar(Line, Column, ' '); 
		OLED_ShowNum(Line, Column + 1, Number, Length);
	}
}

CanTxMsg TxMsg_Timing = {
	.StdId = 0x100, 
	.ExtId = 0x00000000,
	.IDE = CAN_Id_Standard,
	.RTR = CAN_RTR_Data,
	.DLC = 2,
	.Data = {0x00, 0x00}
};

int main(void)
{
	OLED_Init();
	RedSensor_Init();
	Timer_Init();
	Motor_Init();
	MyCAN_Init();
	
	OLED_ShowString(1, 1, "Motor");
	OLED_ShowString(2, 1, "Real RPM:  ");
	OLED_ShowString(3, 1, "Target RPM:");
	
	while(1)
	{
		if(MyCAN_RxFlag == 1)
		{
			MyCAN_RxFlag = 0;

			if (MyCAN_RxMsg.StdId == 0x200 && MyCAN_RxMsg.IDE == CAN_Id_Standard)
			{
				TARGET_RPM = (int16_t)((MyCAN_RxMsg.Data[0] << 8) | MyCAN_RxMsg.Data[1]);
				integral_error = 0;
				previous_error = 0;
			}
		}
		
		if (TimingFlag == 1)
		{
			TimingFlag = 0;
			count = RedSensor_Get();
			RPM_magnitude = count * 200;
			RedSensor_Clear();
			
			if (TARGET_RPM < 0)
			{
				RPM = -RPM_magnitude; 
			}
			else 
			{
				RPM = RPM_magnitude; 
			}
			
			TxMsg_Timing.Data[0] = (uint8_t)(RPM >> 8);
			TxMsg_Timing.Data[1] = (uint8_t)(RPM & 0xFF);
			
			MyCAN_Transmit(&TxMsg_Timing);
		
			RPM_Sign(2, 12, RPM, 4);
			
			float error = TARGET_RPM - RPM;
        
			integral_error += error;
			if (integral_error > 8000) integral_error = 8000;
			if (integral_error < -8000) integral_error = -8000;

			float derivative_error = error - previous_error;
        
			float output = (Kp * error) + (Ki * integral_error) + (Kd * derivative_error);
        
			speed = (int16_t)output;

			previous_error = error;
			// -----------------------------------------------------------
		
			if (TARGET_RPM == 0)
			{
				speed = 0;
			}
			if (speed > 100) speed = 100;
			if (speed < -100) speed = -100; 
			Motor_SetSpeed(speed);
		}
		RPM_Sign(3, 12, TARGET_RPM, 4); 
	}
}

void TIM3_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM3, TIM_IT_Update) == SET)
	{
		TimingFlag = 1;
		TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
	}
}
