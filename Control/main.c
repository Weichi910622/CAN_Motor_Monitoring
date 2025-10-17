#include "stm32f10x.h"
#include "Delay.h"
#include "OLED.h"
#include "MyCAN.h"
#include "Key.h"

uint8_t KeyNum;
uint8_t TriggerFlag;
int16_t TARGET_RPM = 0;
int16_t reconstructed_rpm;

CanTxMsg TxMsg_Trigger = {
	.StdId = 0x200,
	.ExtId = 0x00000000,
	.IDE = CAN_Id_Standard,
	.RTR = CAN_RTR_Data,
	.DLC = 2,
	.Data = {0x00, 0x00}
};
	
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

int main(void)
{
	OLED_Init();
	Key_Init();
	MyCAN_Init();
	Timer_Init();
	
	OLED_ShowString(1, 1, "Control");
	OLED_ShowString(2, 1, "Target RPM:");
	
	while(1)
	{
		KeyNum = Key_GetNum();
		if (KeyNum == 1)
		{
			TriggerFlag = 1;
		}
		else if (KeyNum == 2)
		{
			TriggerFlag = 2;
		}

		
		if(TriggerFlag == 1)
		{
			TriggerFlag = 0;
			if (TARGET_RPM < 8000)
			{
				TARGET_RPM += 1000;
				TxMsg_Trigger.Data[0] = (uint8_t)(TARGET_RPM >> 8);
				TxMsg_Trigger.Data[1] = (uint8_t)(TARGET_RPM & 0xFF);
		
				MyCAN_Transmit(&TxMsg_Trigger);
			
				reconstructed_rpm = (int16_t)(TxMsg_Trigger.Data[0] << 8) | TxMsg_Trigger.Data[1];
				RPM_Sign(2, 12, reconstructed_rpm, 4);
				OLED_ShowString(3, 1, "                ");
			}
			else
			{
				OLED_ShowString(3, 1, "Maximun is 8000");
			}
		}
		if(TriggerFlag == 2)
		{
			TriggerFlag = 0;
			if (TARGET_RPM > -8000)
			{
				TARGET_RPM -= 1000;
				TxMsg_Trigger.Data[0] = (uint8_t)(TARGET_RPM >> 8);
				TxMsg_Trigger.Data[1] = (uint8_t)(TARGET_RPM & 0xFF);
		
				MyCAN_Transmit(&TxMsg_Trigger);
			
				reconstructed_rpm = (int16_t)(TxMsg_Trigger.Data[0] << 8) | TxMsg_Trigger.Data[1];
				RPM_Sign(2, 12, reconstructed_rpm, 4);
				OLED_ShowString(3, 1, "                ");
			}
			else
			{
				OLED_ShowString(3, 1, "Minimun is -8000");
			}
		}

	}
}
