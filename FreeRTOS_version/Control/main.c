#include "stm32f10x.h"
#include "OLED.h"
#include "MyCAN.h"  
#include "Key.h" 
#include "FreeRTOS.h"
#include "task.h" 
#include "queue.h"

extern QueueHandle_t xKeyQueue;
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

void vMainTask(void *pvParameters); 

int main(void)
{
	Key_Init();   
	
	xTaskCreate(vMainTask, "MainTask", 256, NULL, 
                tskIDLE_PRIORITY + 1, NULL); 
				
	vTaskStartScheduler(); 
	while(1);
}

void vMainTask(void *pvParameters)
{
  uint16_t localKeyNum; 
  taskENTER_CRITICAL();
  OLED_Init();   
  MyCAN_Init();  
  taskEXIT_CRITICAL(); 

	OLED_ShowString(1, 1, "Control");
	OLED_ShowString(2, 1, "Target RPM:");
  RPM_Sign(2, 12, TARGET_RPM, 4); 
	
	while(1)
	{
		if (xQueueReceive(xKeyQueue, &localKeyNum, portMAX_DELAY) == pdTRUE)
		{
			if (localKeyNum == 1)
			{	
				vTaskDelay(pdMS_TO_TICKS(50));  

				if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_1) == 0)
				{
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
					while(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_1) == 0)
					{
						vTaskDelay(pdMS_TO_TICKS(20));
					}
				}
			}
			else if (localKeyNum == 2)
			{
				vTaskDelay(pdMS_TO_TICKS(50));
				if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_11) == 0)
				{
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
					while(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_11) == 0)
					{
						vTaskDelay(pdMS_TO_TICKS(20));
					}
				}
			}
		}
	}
}
