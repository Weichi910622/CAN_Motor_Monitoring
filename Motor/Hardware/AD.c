#include "stm32f10x.h"                  // Device header

void AD_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	
	RCC_ADCCLKConfig(RCC_PCLK2_Div6 ); //72MHz/6 = 12

	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_55Cycles5); //12.5(fix) + 55.5 = 68, 72MHz / 68T = 5.6us
	
	ADC_InitTypeDef ADC_InitSturcture;
	ADC_InitSturcture.ADC_ContinuousConvMode = DISABLE;
	ADC_InitSturcture.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitSturcture.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None; //使用軟件觸發非外部觸發
	ADC_InitSturcture.ADC_Mode = ADC_Mode_Independent;
	ADC_InitSturcture.ADC_NbrOfChannel = 1;
	ADC_InitSturcture.ADC_ScanConvMode = DISABLE;
	ADC_Init(ADC1, &ADC_InitSturcture);
	
	ADC_Cmd(ADC1, ENABLE);
	
	ADC_ResetCalibration(ADC1);
	while(ADC_GetResetCalibrationStatus(ADC1) == SET); //SET == 1, RESET == 0(等待自動清0即跳出while)
	ADC_StartCalibration(ADC1);
	while(ADC_GetCalibrationStatus(ADC1) == SET);
	
}

uint16_t ADC_GetValue(void)
{
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);
	while(ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET); //RESET == 轉換未完成, 大約需5.6us
	return ADC_GetConversionValue(ADC1);
}
