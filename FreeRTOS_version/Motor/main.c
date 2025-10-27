#include "stm32f10x.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "OLED.h"
#include "Motor.h"
#include "RedSensor.h"
#include "Timer.h"
#include "MyCAN.h"

const float Kp = 0.015f;
const float Ki = 0.03f;
const float Kd = 0.04f;
float   g_integral_error = 0.0f;
float   g_previous_error = 0.0f;
int16_t g_TargetRpm = 0;

QueueHandle_t xCanRxQueue = NULL;
SemaphoreHandle_t xPidSyncSemaphore = NULL;
SemaphoreHandle_t xOledMutex = NULL;
SemaphoreHandle_t xPidDataMutex = NULL;
SemaphoreHandle_t xCanTxMutex = NULL;

static void vPidControlTask(void *pvParameters);
static void vCanRxTask(void *pvParameters);

void RPM_Sign(uint8_t Line, uint8_t Column, int16_t Number, uint8_t Length)
{
    if (Number < 0) 
	{
		OLED_ShowChar(Line, Column, '-');
		OLED_ShowNum(Line, Column + 1, (uint32_t)(-Number), Length);
	} 
	else 
	{
		OLED_ShowChar(Line, Column, ' ');
        OLED_ShowNum(Line, Column + 1, (uint32_t)Number, Length);
    }
}

int main(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);-
    OLED_Init();
    RedSensor_Init();
    Timer_Init();
    Motor_Init();
    MyCAN_Init();

    OLED_Clear();
    OLED_ShowString(1, 1, "Motor"); 
    OLED_ShowString(2, 1, "Real RPM:  ");    
    OLED_ShowString(3, 1, "Target RPM:");   
    RPM_Sign(3, 12, 0, 4);                  

    xCanRxQueue = xQueueCreate(5, sizeof(CanRxMsg));
    configASSERT(xCanRxQueue != NULL);
    xPidSyncSemaphore = xSemaphoreCreateBinary();
    configASSERT(xPidSyncSemaphore != NULL);
    xOledMutex = xSemaphoreCreateMutex();
    configASSERT(xOledMutex != NULL);
    xPidDataMutex = xSemaphoreCreateMutex();
    configASSERT(xPidDataMutex != NULL);
    xCanTxMutex = xSemaphoreCreateMutex();
    configASSERT(xCanTxMutex != NULL);

    BaseType_t xTaskCreateResult;
    xTaskCreateResult = xTaskCreate(vPidControlTask, "PID_Task", 256, NULL, tskIDLE_PRIORITY + 2, NULL);
    configASSERT(xTaskCreateResult == pdPASS);
    (void)xTaskCreateResult;
    xTaskCreateResult = xTaskCreate(vCanRxTask, "CAN_RX_Task", 256, NULL, tskIDLE_PRIORITY + 1, NULL);
    configASSERT(xTaskCreateResult == pdPASS);
    (void)xTaskCreateResult;

    vTaskStartScheduler();
    while(1); 
}

static void vPidControlTask(void *pvParameters)
{
    int16_t speed = 0;
    int16_t RPM_magnitude;
    uint16_t count;
    int16_t RPM = 0;
    float error, derivative_error, output;
    int16_t local_TargetRpm;

    CanTxMsg txMsg = {
        .StdId = 0x100,
        .IDE = CAN_Id_Standard,
        .RTR = CAN_RTR_Data,
        .DLC = 2,
        .Data = {0x00, 0x00}
    };

    while(1)
    {
        if (xSemaphoreTake(xPidSyncSemaphore, portMAX_DELAY) == pdTRUE)
        {
            count = RedSensor_GetAndClear();
            RPM_magnitude = (int16_t)(count * 200);

            if (xSemaphoreTake(xPidDataMutex, pdMS_TO_TICKS(10)) == pdTRUE)
            {
                local_TargetRpm = g_TargetRpm;

                if (g_TargetRpm < 0) 
				        { 
					          RPM = -RPM_magnitude; 
				        }
                else if (g_TargetRpm > 0) 
				        { 
					          RPM = RPM_magnitude; 
				        }
                else 
				        { 
					          RPM = 0; RPM_magnitude = 0; 
				        }

                error = (float)g_TargetRpm - (float)RPM;
                g_integral_error += error;
                if (g_integral_error > 8000.0f) g_integral_error = 8000.0f;
                if (g_integral_error < -8000.0f) g_integral_error = -8000.0f;
                derivative_error = error - g_previous_error;
                output = (Kp * error) + (Ki * g_integral_error) + (Kd * derivative_error);
                g_previous_error = error;

                xSemaphoreGive(xPidDataMutex); 

                if (local_TargetRpm == 0) 
				        {
                     speed = 0;
                } 
				        else 
				        {
                     speed = (int16_t)output;
                     if (speed > 100) speed = 100;
                     if (speed < -100) speed = -100;
                }
                Motor_SetSpeed(speed); 
            } 
			else 
			{
           continue; 
      }

            txMsg.Data[0] = (uint8_t)(RPM >> 8);
            txMsg.Data[1] = (uint8_t)(RPM & 0xFF);
  
            MyCAN_Transmit(&txMsg); 

            if (xSemaphoreTake(xOledMutex, pdMS_TO_TICKS(50)) == pdTRUE)
            {
                RPM_Sign(2, 12, RPM, 4); 
                // OLED_ShowNum(4, 5, count, 5); 
                xSemaphoreGive(xOledMutex);
            } 
        }
    }
}

static void vCanRxTask(void *pvParameters)
{
    CanRxMsg rxMsg;
    int16_t new_TargetRpm;

    while(1)
    {
        if (xQueueReceive(xCanRxQueue, &rxMsg, portMAX_DELAY) == pdPASS)
        {
            if (rxMsg.StdId == 0x200 && rxMsg.IDE == CAN_Id_Standard)
            {
                new_TargetRpm = (int16_t)(((uint16_t)rxMsg.Data[0] << 8) | rxMsg.Data[1]);

                if (xSemaphoreTake(xPidDataMutex, pdMS_TO_TICKS(10)) == pdTRUE)
                {
                    if (new_TargetRpm != g_TargetRpm) {
                        g_integral_error = 0.0f;
                        g_previous_error = 0.0f;
                    }
                    g_TargetRpm = new_TargetRpm;
                    xSemaphoreGive(xPidDataMutex);

                    if (xSemaphoreTake(xOledMutex, pdMS_TO_TICKS(10)) == pdTRUE)
                    {
                        RPM_Sign(3, 12, new_TargetRpm, 4); 
                        xSemaphoreGive(xOledMutex);
                    }
                }
            }
        }
    }
}

void TIM3_IRQHandler(void)
{
    if(TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)
    {
        TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        if (xPidSyncSemaphore != NULL)
        {
            xSemaphoreGiveFromISR(xPidSyncSemaphore, &xHigherPriorityTaskWoken);
        }
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken); 
    }
}
