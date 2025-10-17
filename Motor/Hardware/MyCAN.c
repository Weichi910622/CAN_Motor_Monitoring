#include "stm32f10x.h"                  // Device header

CanRxMsg MyCAN_RxMsg;
volatile uint8_t MyCAN_RxFlag;

void MyCAN_Init(void)
{
	//初始化第一步RCC開啟時鐘
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);   //CAN1位於PA11, PA12開啟GPIOA時鐘
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, ENABLE);    //CAN外設掛載於APB1，開啟CAN外設時鐘
	
	//初始化第二步初始化GPIO口
	GPIO_InitTypeDef GPIO_InitSturcture;
	GPIO_InitSturcture.GPIO_Mode = GPIO_Mode_AF_PP;         //復用模式，引腳控制權交給CAN外設
	GPIO_InitSturcture.GPIO_Pin = GPIO_Pin_12;              //TD
	GPIO_InitSturcture.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitSturcture);
	
	GPIO_InitSturcture.GPIO_Mode = GPIO_Mode_IPU;           //引腳默認狀態為高電平，選擇上拉輸入
	GPIO_InitSturcture.GPIO_Pin = GPIO_Pin_11;              //TD
	GPIO_InitSturcture.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitSturcture);
	
	CAN_ITConfig(CAN1, CAN_IT_FMP0, ENABLE);
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = USB_LP_CAN1_RX0_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_Init(&NVIC_InitStructure);
	
	//初始化第三步CAN外設初始化
	CAN_InitTypeDef CAN_InitStructure;
	CAN_InitStructure.CAN_Mode = CAN_Mode_Normal;
	CAN_InitStructure.CAN_Prescaler = 48;         //波特率 = 36M(APB1) / 48 / (1 + 2 + 3) = 125k (高速CAN波特率範圍為125k~1M)
	CAN_InitStructure.CAN_BS1 = CAN_BS1_2tq;      //與TIMER不同，此處庫函數寫入寄存器已經-1
	CAN_InitStructure.CAN_BS2 = CAN_BS2_3tq;
	CAN_InitStructure.CAN_SJW = CAN_SJW_2tq;
	CAN_InitStructure.CAN_NART = DISABLE;         //關閉不自動重傳
	CAN_InitStructure.CAN_TXFP = DISABLE;         //ID小先發送
	CAN_InitStructure.CAN_RFLM = DISABLE;         //FIFO溢出，最後收到的報文被新報文覆蓋
	CAN_InitStructure.CAN_AWUM = DISABLE;         //手動喚醒
	CAN_InitStructure.CAN_TTCM = DISABLE;         //時間觸發通信模式關閉
	CAN_InitStructure.CAN_ABOM = DISABLE;         //離線自動恢復關閉
	CAN_Init(CAN1, &CAN_InitStructure);			  //工作模式切換封裝於此
	
	//初始化第四步配置過濾器
	CAN_FilterInitTypeDef CAN_FilterInitSturcture;
	CAN_FilterInitSturcture.CAN_FilterNumber = 0;	                      //初始化過濾器0
	
	CAN_FilterInitSturcture.CAN_FilterIdHigh = 0x0000;		              //全通模式，隨意給ID
	CAN_FilterInitSturcture.CAN_FilterIdLow = 0x0000;
	CAN_FilterInitSturcture.CAN_FilterMaskIdHigh = 0x0000;
	CAN_FilterInitSturcture.CAN_FilterMaskIdLow = 0x0000;
	CAN_FilterInitSturcture.CAN_FilterScale = CAN_FilterScale_32bit;      //過濾器位寬，此處32位
	CAN_FilterInitSturcture.CAN_FilterMode = CAN_FilterMode_IdMask;       //過濾器模式，此處為屏蔽
	CAN_FilterInitSturcture.CAN_FilterFIFOAssignment = CAN_Filter_FIFO0;  //通過過濾器報文進入FIFO0排隊
	CAN_FilterInitSturcture.CAN_FilterActivation = ENABLE;               //激活過濾器
	CAN_FilterInit(&CAN_FilterInitSturcture);	
	
		
}

void MyCAN_Transmit(CanTxMsg *TxMessage)
{
	//CRC由硬體自動生成校驗
	uint8_t TransmitMailbox = CAN_Transmit(CAN1, TxMessage);            //寫入發送郵箱並由管理員發送，CAN_Transmit返回值表示選中的郵箱
	
	uint32_t Timeout = 0;
	while (CAN_TransmitStatus(CAN1, TransmitMailbox) != CAN_TxStatus_Ok) //請求發送後數據不會立刻發送完成，加入等待發送完成，返回值CAN_TxStatus_Ok表示傳輸成功
	{
		Timeout ++;
		if (Timeout > 100000)                                            //避免超時程序卡死
		{
			break;
		}
	}
}

void USB_LP_CAN1_RX0_IRQHandler(void)
{
	if(CAN_GetITStatus(CAN1, CAN_IT_FMP0) == SET)
	{
		CAN_Receive(CAN1, CAN_FIFO0, &MyCAN_RxMsg);
		MyCAN_RxFlag = 1;
	}
}
