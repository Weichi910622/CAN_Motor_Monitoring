#include "stm32f10x.h"
#include "Delay.h"
#include "OLED.h"
#include "MyCAN.h"

uint8_t KeyNum;
int16_t real_rpm;
int16_t target_rpm;
CanRxMsg RxMsg;

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
	MyCAN_Init();
	
	OLED_ShowString(1, 1, "Monitor");
	OLED_ShowString(2, 1, "Real RPM:");
	OLED_ShowString(3, 1, "Target RPM:");
	
	while(1)
	{
		if(MyCAN_ReceiveFlag())
		{
			MyCAN_Receive(&RxMsg);
			
			if (RxMsg.RTR == CAN_RTR_Data)
			{
				if (RxMsg.StdId == 0x100 && RxMsg.IDE == CAN_Id_Standard)
				{
					real_rpm = (int16_t)(RxMsg.Data[0] << 8) | RxMsg.Data[1];
				}
				if (RxMsg.StdId == 0x200 && RxMsg.IDE == CAN_Id_Standard)
				{
					target_rpm = (int16_t)(RxMsg.Data[0] << 8) | RxMsg.Data[1];
					RPM_Sign(3, 12, target_rpm, 4);
					if (target_rpm >= 8000)
					{
						OLED_ShowString(4, 1, "Maximun is 8000");
					}
					else if (target_rpm <= -8000)
					{
						OLED_ShowString(4, 1, "Minimum is -8000");
					}
					else
					{
						OLED_ShowString(4, 1, "                ");
					}
				}
			}
		}
		RPM_Sign(2, 12, real_rpm, 4);
	}
}
