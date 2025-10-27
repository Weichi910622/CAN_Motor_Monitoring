#include "stm32f10x.h"
#include "OLED.h"
#include "MyCAN.h" 
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"   

QueueHandle_t xCanRxQueue = NULL;

void vCanMonitorTask(void *pvParameters);

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
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	MyCAN_Init();

  xCanRxQueue = xQueueCreate(10, sizeof(CanRxMsg));

  if (xCanRxQueue != NULL)
  {
        xTaskCreate(vCanMonitorTask,    
                    "CANMonitor",       
                    256,               
                    NULL,           
                    tskIDLE_PRIORITY + 1, 
                    NULL);            
  }
  else
  {
        while(1);
  }
	vTaskStartScheduler();
	while(1);
}

void vCanMonitorTask(void *pvParameters)
{
    CanRxMsg localRxMsg; 
    int16_t real_rpm = 0;
    int16_t target_rpm = 0;
    int16_t last_real_rpm = -1; 
    int16_t last_target_rpm = -1;

    taskENTER_CRITICAL();
    OLED_Init(); 
    taskEXIT_CRITICAL();

	  OLED_ShowString(1, 1, "Monitor");
	  OLED_ShowString(2, 1, "Real RPM:");
	  OLED_ShowString(3, 1, "Target RPM:");
    RPM_Sign(2, 12, real_rpm, 4); 
    RPM_Sign(3, 12, target_rpm, 4); 
    OLED_ShowString(4, 1, "                "); 

	while(1)
	{
    if (xQueueReceive(xCanRxQueue, &localRxMsg, portMAX_DELAY) == pdTRUE)
    {

			if (localRxMsg.RTR == CAN_RTR_Data && localRxMsg.IDE == CAN_Id_Standard)
			{
				if (localRxMsg.StdId == 0x100)
				{
					real_rpm = (int16_t)(localRxMsg.Data[0] << 8) | localRxMsg.Data[1];
          if (real_rpm != last_real_rpm)
          {
              RPM_Sign(2, 12, real_rpm, 4);
              last_real_rpm = real_rpm; 
          }
				}
				else if (localRxMsg.StdId == 0x200)
				{
					target_rpm = (int16_t)(localRxMsg.Data[0] << 8) | localRxMsg.Data[1];
          if (target_rpm != last_target_rpm)
          {
              RPM_Sign(3, 12, target_rpm, 4);
              last_target_rpm = target_rpm; 
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
                  OLED_ShowString(4, 1, "                "); // 清除警告
              }
          }
				}
			}
    }
	}
}
