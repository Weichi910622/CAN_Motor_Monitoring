#include "stm32f10x.h"
#include "MyCAN.h" 
#include "FreeRTOS.h"
#include "queue.h"
#include "OLED.h"
#include "semphr.h"

extern QueueHandle_t xCanRxQueue;
extern SemaphoreHandle_t xCanTxMutex;

void MyCAN_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitSturcture;
	GPIO_InitSturcture.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitSturcture.GPIO_Pin = GPIO_Pin_12;
	GPIO_InitSturcture.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitSturcture);
	
	GPIO_InitSturcture.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitSturcture.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitSturcture.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitSturcture);
    
	CAN_ITConfig(CAN1, CAN_IT_FMP0, ENABLE);
    
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = USB_LP_CAN1_RX0_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 14; 
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0; 
	NVIC_Init(&NVIC_InitStructure);
	
	CAN_InitTypeDef CAN_InitStructure;
	CAN_InitStructure.CAN_Mode = CAN_Mode_Normal;
	CAN_InitStructure.CAN_Prescaler = 48;
	CAN_InitStructure.CAN_BS1 = CAN_BS1_2tq;
	CAN_InitStructure.CAN_BS2 = CAN_BS2_3tq;
	CAN_InitStructure.CAN_SJW = CAN_SJW_2tq;
	CAN_InitStructure.CAN_NART = DISABLE;
	CAN_InitStructure.CAN_TXFP = DISABLE;
	CAN_InitStructure.CAN_RFLM = DISABLE;
	CAN_InitStructure.CAN_AWUM = DISABLE;
	CAN_InitStructure.CAN_TTCM = DISABLE;
	CAN_InitStructure.CAN_ABOM = DISABLE;
	CAN_Init(CAN1, &CAN_InitStructure);
	
	CAN_FilterInitTypeDef CAN_FilterInitSturcture;
	CAN_FilterInitSturcture.CAN_FilterNumber = 0;
	CAN_FilterInitSturcture.CAN_FilterIdHigh = 0x0000;
	CAN_FilterInitSturcture.CAN_FilterIdLow = 0x0000;
	CAN_FilterInitSturcture.CAN_FilterMaskIdHigh = 0x0000;
	CAN_FilterInitSturcture.CAN_FilterMaskIdLow = 0x0000;
	CAN_FilterInitSturcture.CAN_FilterScale = CAN_FilterScale_32bit;
	CAN_FilterInitSturcture.CAN_FilterMode = CAN_FilterMode_IdMask;
	CAN_FilterInitSturcture.CAN_FilterFIFOAssignment = CAN_Filter_FIFO0;
	CAN_FilterInitSturcture.CAN_FilterActivation = ENABLE;
	CAN_FilterInit(&CAN_FilterInitSturcture);	
}

void MyCAN_Transmit(CanTxMsg *TxMessage)
{
    if (xSemaphoreTake(xCanTxMutex, portMAX_DELAY) == pdTRUE)
    {
        uint8_t TransmitMailbox = CAN_Transmit(CAN1, TxMessage);
     
		    TickType_t xStartTime = xTaskGetTickCount();
		    const TickType_t xTimeoutTicks = pdMS_TO_TICKS(100); 
        
        while (CAN_TransmitStatus(CAN1, TransmitMailbox) != CAN_TxStatus_Ok)
        {
            if ((xTaskGetTickCount() - xStartTime) > xTimeoutTicks)
			      {
				        break;
			      }
            vTaskDelay( (TickType_t) 1 );
        }     
        xSemaphoreGive(xCanTxMutex);
    }
}

void USB_LP_CAN1_RX0_IRQHandler(void)
{
    CanRxMsg rxMsg; 
    BaseType_t xHigherPriorityTaskWoken = pdFALSE; 

	  if(CAN_GetITStatus(CAN1, CAN_IT_FMP0) == SET)
	  {
		  CAN_Receive(CAN1, CAN_FIFO0, &rxMsg);
		  xQueueSendToBackFromISR(xCanRxQueue, &rxMsg, &xHigherPriorityTaskWoken);
	  }   
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}
