/********************************** (C) COPYRIGHT *******************************
 * File Name          : main.c
 * Author             : WCH
 * Version            : V1.0.0
 * Date               : 2023/12/25
 * Description        : Main program body.
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

/*
 *@Note
 *Multiprocessor communication mode routine:
 *Master:USART1_Tx(PD5)\USART1_Rx(PD6).
 *This routine demonstrates that USART1 receives the data sent by CH341 and inverts
 *it and sends it (baud rate 115200).
 *
 *Hardware connection:PD5 -- Rx
 *                     PD6 -- Tx
 *
 */

#include "debug.h"


/* Global define */

/* Global Variable */

/*********************************************************************
 * @fn      GPIO_Function_Init
 *
 * @brief   Initializes GPIO collection.
 *
 * @return  none
 */
void GPIO_Function_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure={0x00};

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);

    //§¥§Ý§ñ SPI, MOSI
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    //§¥§Ý§ñ ADC
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
}

/*********************************************************************
 * @fn      IWDG_Init
 *
 * @brief   Initializes IWDG.
 *
 * @param   IWDG_Prescaler - specifies the IWDG Prescaler value.
 *            IWDG_Prescaler_4 - IWDG prescaler set to 4.
 *            IWDG_Prescaler_8 - IWDG prescaler set to 8.
 *            IWDG_Prescaler_16 - IWDG prescaler set to 16.
 *            IWDG_Prescaler_32 - IWDG prescaler set to 32.
 *            IWDG_Prescaler_64 - IWDG prescaler set to 64.
 *            IWDG_Prescaler_128 - IWDG prescaler set to 128.
 *            IWDG_Prescaler_256 - IWDG prescaler set to 256.
 *          Reload - specifies the IWDG Reload value.
 *            This parameter must be a number between 0 and 0x0FFF.
 *
 * @return  none
 */
void IWDG_Init(u16 prer, u16 rlr)
{
    IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
    IWDG_SetPrescaler(prer);
    IWDG_SetReload(rlr);
    IWDG_ReloadCounter();
    IWDG_Enable();
}

/*********************************************************************
 * @fn      SPI_Function_Init
 *
 * @brief   Initializes SPI collection.
 *
 * @return  none
 */
void SPI_Function_Init(void)
{
    SPI_InitTypeDef SPI_InitStructure={0};

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);

    SPI_InitStructure.SPI_Direction = SPI_Direction_1Line_Tx;
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_16b;
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
    SPI_InitStructure.SPI_CRCPolynomial = 7;
    SPI_Init(SPI1, &SPI_InitStructure);

    SPI_Cmd(SPI1, ENABLE);
}

/*********************************************************************
 * @fn      ADC_Function_Init
 *
 * @brief   Initializes ADC collection.
 *
 * @return  none
 */
void ADC_Function_Init(void)
{
	ADC_InitTypeDef ADC_InitStructure={0x00};

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
	RCC_ADCCLKConfig(RCC_PCLK2_Div2);

	ADC_DeInit(ADC1);
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_NbrOfChannel = 1;
	ADC_Init(ADC1, &ADC_InitStructure);

    ADC_Calibration_Vol(ADC1, ADC_CALVOL_50PERCENT);
    ADC_Cmd(ADC1, ENABLE);

    ADC_ResetCalibration(ADC1);
    while(ADC_GetResetCalibrationStatus(ADC1));
    ADC_StartCalibration(ADC1);
    while(ADC_GetCalibrationStatus(ADC1));
}

/*********************************************************************
 * @fn      WS2812B_RESET
 *
 * @brief   Reset WS2812B
 *
 * @return  none
 */
void WS2812B_RESET(void)
{
    //§¤§Ö§ß§Ö§â§Ú§â§å§Ö§Þ §ã§Ú§Ô§ß§Ñ§Ý §ã§Ò§â§à§ã§Ñ, 720 §Ú§Þ§á§å§Ý§î§ã§à§Ó SPI
    //§²§Ö§Ø§Ú§Þ SPI 16 §Ò§Ú§ä
    //§¹§Ñ§ã§ä§à§ä§Ñ SPI 48/4 = 12 §®§¤§è
    //45 * 16 / 12 §®§¤§è = 64 §Þ§Ü§ã

    uint8_t i;

    //§£§í§ã§à§Ü§Ú§Û §å§â§à§Ó§Ö§ß§î, 64 §Þ§ã
    for(i = 0x00; i < 45; i++)
    {
        //Send SPI Byte
        while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET); // wait while flag is zero or TX buffer not empty
        SPI_I2S_SendData(SPI1, 0xFFFF);
    }

    //Reloads IWDG counter
    IWDG_ReloadCounter();

    //§¯§Ú§Ù§Ü§Ú§Û §å§â§à§Ó§Ö§ß§î, 64 §Þ§ã
    for(i = 0x00; i < 45; i++)
    {
        //Send SPI Byte
        while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET); // wait while flag is zero or TX buffer not empty
        SPI_I2S_SendData(SPI1, 0x0000);
    }

    //Reloads IWDG counter
    IWDG_ReloadCounter();    
}

/*********************************************************************
 * @fn      WS2812B_SEND_PIXEL
 *
 * @brief   Send 24 bits to WS2812B
 *
 * @return  none
 */
void WS2812B_SEND_PIXEL(uint32_t pixel)
{
    uint16_t data;
    uint8_t i;

    //§°§á§â§Ö§Õ§Ö§Ý§Ú§Þ §Ù§ß§Ñ§é§Ö§ß§Ú§Ö §á§à§ã§í§Ý§Ü§Ú §Ó §Ù§Ñ§Ó§Ú§ã§Ú§Þ§à§ã§ä§Ú §à§ä §Ò§Ú§ä§Ñ
    //§²§Ö§Ø§Ú§Þ SPI 16 §Ò§Ú§ä
    //§¹§Ñ§ã§ä§à§ä§Ñ SPI 48/4 = 12 §®§¤§è
    //5 §Ò§Ú§ä §á§à§Õ§â§ñ§Õ §ï§ä§à 0.83 §Þ§Ü§ã
    //4 §Ò§Ú§ä §á§à§Õ§â§ñ§Õ §ï§ä§à 0.5 §Þ§Ü§ã
    //10 §Ö§Õ§Ö§ß§Ú§è + 6 §ß§å§Ý§Ö§Û - §ï§ä§à §Ü§à§Õ 1
    //6 §Ö§Õ§Ö§ß§Ú§è + 10 §ß§å§Ý§Ö§Û - §ï§ä§à §Ü§à§Õ 0

    for(i = 0x00; i < 0x18; i++)
    {
        if(pixel & 0x00800000)
        {
            data = 0xFFC0;
        }
        else        
        {   
            data = 0xF800;
        }
        pixel = pixel << 0x01;

        //Send SPI Byte
        while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET); // wait while flag is zero or TX buffer not empty
        SPI_I2S_SendData(SPI1, data);
    }
}

void led_program_0(void)
{
    uint32_t led_state[19], i, led_value;
    uint32_t time;

    led_value = 0x00000000;
    time = 50;

    while(time--)    
    {
        //Reloads IWDG counter
        IWDG_ReloadCounter();

        //§±§Ñ§å§Ù§Ñ
        Delay_Ms(300);
        
        for(i = 0x00; i < 19; i++)
        {
            if(led_value == 0x00000000)
                led_value = 0x00FF0000;
            else
                led_value >>= 0x08;
            led_state[i] = led_value;    
        }

        for(i = 0x00; i < 19; i++)
        {
            WS2812B_SEND_PIXEL(led_state[i]);
        }        
    }
}

void led_program_1(void)
{
    uint32_t led_state[19], i, led_value;
    uint32_t r, g, b, cnt, led_cnt;
    uint32_t time;

    led_value = 0x00000000;
    r = 0x00;
    g = 0x00;
    b = 0x00;
    cnt = 0x00;
    led_cnt = 0x00;
    time = 600;

    while(time--)
    {
        //Reloads IWDG counter
        IWDG_ReloadCounter();        

        //§±§Ñ§å§Ù§Ñ
        Delay_Ms(25);

        switch(cnt)
        {
            case 0x00:
            {
                r += 0x08;
                if(r >= 0xFF)
                {
                    cnt++;
                }

                break;
            }
            case 0x01:
            {
                r -= 0x08;
                if(r == 0x00)
                {
                    cnt++;
                }

                break;
            }   
            case 0x02:
            {
                g += 0x08;
                if(g >= 0xFF)
                {
                    cnt++;
                }

                break;
            }
            case 0x03:
            {
                g -= 0x08;
                if(g == 0x00)
                {
                    cnt++;
                }

                break;
            }   
            case 0x04:
            {
                b += 0x08;
                if(b >= 0xFF)
                {
                    cnt++;
                }

                break;
            }
            case 0x05:
            {
                b -= 0x08;
                if(b == 0x00)
                {
                    cnt = 0x00;
                }

                break;
            }                                        
            default:
            {
                r = 0x00;
                g = 0x00;
                b = 0x00;
                cnt = 0x00;

                break;
            }
        }      

        led_cnt++;
        if(led_cnt >= 19) led_cnt = 0x00;

        led_value = (b << 0x10) + (r << 0x08) + g;
        for(i = 0x00; i < 19; i++)
        {
            led_state[i] = 0x00;
        }
        led_state[led_cnt] = led_value;

        for(i = 0x00; i < 19; i++)
        {
            WS2812B_SEND_PIXEL(led_state[i]);
        }        
    }
}

void led_program_2(void)
{
    uint32_t led_state[19], i, led_value;
    uint32_t time;

    led_value = 0x00000000;
    time = 50;

    while(time--)    
    {
        //Reloads IWDG counter
        IWDG_ReloadCounter();

        //§±§Ñ§å§Ù§Ñ
        Delay_Ms(300);
        
        for(i = 0x00; i < 19; i++)
        {
            if(led_value == 0x00000000)
                led_value = 0x00FF0000;
            else
                led_value >>= 0x04;
            led_state[i] = led_value;    
        }

        for(i = 0x00; i < 19; i++)
        {
            WS2812B_SEND_PIXEL(led_state[i]);
        }        
    }
}

void led_program_3(void)
{
    uint32_t led_state[19], i, led_value;
    uint32_t time;

    led_value = 0x00000000;
    time = 50;

    while(time--)    
    {
        //Reloads IWDG counter
        IWDG_ReloadCounter();

        //§±§Ñ§å§Ù§Ñ
        Delay_Ms(300);
        
        for(i = 0x00; i < 19; i++)
        {
            if(led_value == 0x00000000)
                led_value = 0x000000FF;
            else
                led_value <<= 0x08;
            led_state[i] = led_value;    
        }

        for(i = 0x00; i < 19; i++)
        {
            WS2812B_SEND_PIXEL(led_state[i]);
        }        
    }
}

void led_program_4(void)
{
    uint32_t led_state[19], i, led_value, led_cnt;
    uint32_t time;
    uint32_t adc_value;

    led_value = 0x00000000;
    led_cnt = 0x00;
    time = 50;

    while(time--)    
    {
        //Reloads IWDG counter
        IWDG_ReloadCounter();

        //§±§Ñ§å§Ù§Ñ
        Delay_Ms(295);

        led_value = 0x00000000;
        for(i = 0x00; i < 0x18; i++)
        {        
            ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 0x01, ADC_SampleTime_15Cycles);
            ADC_SoftwareStartConvCmd(ADC1, ENABLE);
            while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC));
            adc_value = ADC_GetConversionValue(ADC1);
            led_value += adc_value & 0x01; 
            led_value <<= 0x01;
        }

        led_cnt++;
        if(led_cnt >= 19) led_cnt = 0x00;

        for(i = 0x00; i < 19; i++)
        {
            led_state[i] = 0x00;
        }
        led_state[led_cnt] = led_value;

        for(i = 0x00; i < 19; i++)
        {
            WS2812B_SEND_PIXEL(led_state[i]);
        }     
    }
}

void led_program_5(void)
{
    uint32_t led_state[19], i, led_cnt;
    uint32_t time;

    led_cnt = 0x00;
    time = 50;

    while(time--)    
    {
        //Reloads IWDG counter
        IWDG_ReloadCounter();

        //§±§Ñ§å§Ù§Ñ
        Delay_Ms(295);

        led_cnt++;
        if(led_cnt >= 0x09) led_cnt = 0x00;

        for(i = 0x00; i < 19; i++)
        {
            led_state[i] = 0x00;
        }
        led_state[led_cnt] = 0x0000FFFF;
        led_state[0x12 - led_cnt] = 0x0000FFFF;
        led_state[0x09] = 0x00FF0000;

        for(i = 0x00; i < 19; i++)
        {
            WS2812B_SEND_PIXEL(led_state[i]);
        }     
    }
}

void led_program_6(void)
{
    uint32_t led_state[19], i, led_cnt;
    uint32_t time;

    led_cnt = 0x00;
    time = 50;

    while(time--)    
    {
        //Reloads IWDG counter
        IWDG_ReloadCounter();

        //§±§Ñ§å§Ù§Ñ
        Delay_Ms(295);

        led_cnt++;
        if(led_cnt >= 0x09) led_cnt = 0x00;

        for(i = 0x00; i < 19; i++)
        {
            led_state[i] = 0x00;
        }
        led_state[led_cnt] = 0x00FF00FF;
        led_state[0x12 - led_cnt] = 0x00FF00FF;
        led_state[0x09] = 0x0000FF00;

        for(i = 0x00; i < 19; i++)
        {
            WS2812B_SEND_PIXEL(led_state[i]);
        }     
    }
}

void led_program_7(void)
{
    uint32_t led_state[19], i, led_cnt;
    uint32_t time;

    led_cnt = 0x00;
    time = 50;

    while(time--)    
    {
        //Reloads IWDG counter
        IWDG_ReloadCounter();

        //§±§Ñ§å§Ù§Ñ
        Delay_Ms(295);

        led_cnt++;
        if(led_cnt >= 0x09) led_cnt = 0x00;

        for(i = 0x00; i < 19; i++)
        {
            led_state[i] = 0x00;
        }
        led_state[led_cnt] = 0x00FFFF00;
        led_state[0x12 - led_cnt] = 0x00FFFF00;
        led_state[0x09] = 0x000000FF;

        for(i = 0x00; i < 19; i++)
        {
            WS2812B_SEND_PIXEL(led_state[i]);
        }     
    }
}

void led_program_8(void)
{
    uint32_t led_state[19], i, j, led_value;
    uint32_t time;
    uint32_t adc_value;

    led_value = 0x00000000;
    time = 50;

    while(time--)    
    {
        //Reloads IWDG counter
        IWDG_ReloadCounter();

        //§±§Ñ§å§Ù§Ñ
        Delay_Ms(290);

        led_value = 0x00000000;
        for(i = 0x00; i < 19; i++)
        {
            for(j = 0x00; j < 0x03; j++)
            {
                ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 0x01, ADC_SampleTime_15Cycles);
                ADC_SoftwareStartConvCmd(ADC1, ENABLE);
                while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC));
                adc_value = ADC_GetConversionValue(ADC1);

                led_value <<= 0x08;
                if(adc_value & 0x01)
                {
                    led_value |= 0x000000FF;
                }
            }

            ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 0x01, ADC_SampleTime_15Cycles);
            ADC_SoftwareStartConvCmd(ADC1, ENABLE);
            while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC));
            adc_value = ADC_GetConversionValue(ADC1);

            if(adc_value & 0x01)
            {
                led_state[i] = led_value;
            }
            else
            {
                led_state[i] = 0x00;
            }
        }

        for(i = 0x00; i < 19; i++)
        {
            WS2812B_SEND_PIXEL(led_state[i]);
        }     
    }
}

void led_program_9(void)
{
    uint32_t led_state[19], i, led_value;
    uint32_t time;

    led_value = 0x00FF0000;
    time = 50;

    while(time--)    
    {
        //Reloads IWDG counter
        IWDG_ReloadCounter();

        //§±§Ñ§å§Ù§Ñ
        Delay_Ms(300);
        
        for(i = 0x00; i < 19; i++)
        {
            if(led_value & 0x01)
                led_value |= 0x01000000;
            led_value >>= 0x01;
            led_state[i] = led_value;    
        }

        for(i = 0x00; i < 19; i++)
        {
            WS2812B_SEND_PIXEL(led_state[i]);
        }        
    }
}

void led_program_a(void)
{
    uint32_t led_state[19], i, led_value;
    uint32_t time;

    led_value = 0x00FF0000;
    time = 50;

    while(time--)    
    {
        //Reloads IWDG counter
        IWDG_ReloadCounter();

        //§±§Ñ§å§Ù§Ñ
        Delay_Ms(300);
        
        for(i = 0x00; i < 19; i++)
        {
            if(led_value & 0x01)
                led_value |= 0x01000000;
            led_value >>= 0x01;
            led_state[0x12 - i] = led_value;    
        }

        for(i = 0x00; i < 19; i++)
        {
            WS2812B_SEND_PIXEL(led_state[i]);
        }        
    }
}

void led_program_b(void)
{
    uint32_t led_state[19], i, led_value;
    uint32_t time;

    led_value = 0x00000000;
    time = 50;

    while(time--)    
    {
        //Reloads IWDG counter
        IWDG_ReloadCounter();

        //§±§Ñ§å§Ù§Ñ
        Delay_Ms(300);
        
        for(i = 0x00; i < 19; i++)
        {
            if(led_value == 0x00000000)
                led_value = 0x00FF0000;
            else
                led_value >>= 0x08;
            led_state[0x12 - i] = led_value;    
        }

        for(i = 0x00; i < 19; i++)
        {
            WS2812B_SEND_PIXEL(led_state[i]);
        }        
    }
}

void led_program_c(void)
{
    uint32_t led_state[19], i, led_value;
    uint32_t r, g, b, cnt, led_cnt;
    uint32_t time;

    led_value = 0x00000000;
    r = 0x00;
    g = 0x00;
    b = 0x00;
    cnt = 0x00;
    led_cnt = 0x00;
    time = 600;

    while(time--)
    {
        //Reloads IWDG counter
        IWDG_ReloadCounter();        

        //§±§Ñ§å§Ù§Ñ
        Delay_Ms(25);

        switch(cnt)
        {
            case 0x00:
            {
                r += 0x08;
                if(r >= 0xFF)
                {
                    cnt++;
                }

                break;
            }
            case 0x01:
            {
                r -= 0x08;
                if(r == 0x00)
                {
                    cnt++;
                }

                break;
            }   
            case 0x02:
            {
                g += 0x08;
                if(g >= 0xFF)
                {
                    cnt++;
                }

                break;
            }
            case 0x03:
            {
                g -= 0x08;
                if(g == 0x00)
                {
                    cnt++;
                }

                break;
            }   
            case 0x04:
            {
                b += 0x08;
                if(b >= 0xFF)
                {
                    cnt++;
                }

                break;
            }
            case 0x05:
            {
                b -= 0x08;
                if(b == 0x00)
                {
                    cnt = 0x00;
                }

                break;
            }                                        
            default:
            {
                r = 0x00;
                g = 0x00;
                b = 0x00;
                cnt = 0x00;

                break;
            }
        }      

        led_cnt++;
        if(led_cnt >= 19) led_cnt = 0x00;

        led_value = (b << 0x10) + (r << 0x08) + g;
        for(i = 0x00; i < 19; i++)
        {
            led_state[i] = 0x00;
        }
        led_state[0x12 - led_cnt] = led_value;

        for(i = 0x00; i < 19; i++)
        {
            WS2812B_SEND_PIXEL(led_state[i]);
        }        
    }
}

void led_program_d(void)
{
    uint32_t led_state[19], i, led_value;
    uint32_t time;

    led_value = 0x00000000;
    time = 50;

    while(time--)    
    {
        //Reloads IWDG counter
        IWDG_ReloadCounter();

        //§±§Ñ§å§Ù§Ñ
        Delay_Ms(300);
        
        for(i = 0x00; i < 19; i++)
        {
            if(led_value == 0x00000000)
                led_value = 0x00FF0000;
            else
                led_value >>= 0x08;

            if(time & 0x01)
                led_state[i] = led_value;
            else
                led_state[i] = ~led_value;
        }

        for(i = 0x00; i < 19; i++)
        {
            WS2812B_SEND_PIXEL(led_state[i]);
        }        
    }
}

void led_program_e(void)
{
    uint32_t led_state[19], i, led_value;
    uint32_t time;

    led_value = 0x00000000;
    time = 50;

    while(time--)    
    {
        //Reloads IWDG counter
        IWDG_ReloadCounter();

        //§±§Ñ§å§Ù§Ñ
        Delay_Ms(300);
        
        for(i = 0x00; i < 19; i++)
        {
            if(led_value == 0x00000000)
                led_value = 0x00FF0000;
            else
                led_value >>= 0x08;

            if(time & 0x01)
                led_state[0x12 - i] = led_value;
            else
                led_state[0x12 - i] = ~led_value;
        }

        for(i = 0x00; i < 19; i++)
        {
            WS2812B_SEND_PIXEL(led_state[i]);
        }        
    }
}

void led_program_f(void)
{
    uint32_t led_state[19], i, led_value;
    uint32_t time;

    led_value = 0x00FF0000;
    time = 50;

    while(time--)    
    {
        //Reloads IWDG counter
        IWDG_ReloadCounter();

        //§±§Ñ§å§Ù§Ñ
        Delay_Ms(300);
        
        for(i = 0x00; i < 19; i++)
        {
            led_state[i] = led_value;

            led_value >>= 0x08;

            if(led_value == 0x00000000)
                led_value = 0x00FF0000;
        }

        for(i = 0x00; i < 19; i++)
        {
            WS2812B_SEND_PIXEL(led_state[i]);
        }        
    }
}

void led_program_10(void)
{
    uint32_t led_state[19], i, j, led_value;
    uint32_t time;

    //§±§Ö§â§Ö§Þ§Ö§ß§ß§í§Ö §ç§â§Ñ§ß§ñ§ë§Ú§Ö §ã§à§ã§ä§à§ñ§ß§Ú§Ö §ã§Õ§Ó§Ú§Ô§à§Ó§à§Ô§à §â§Ö§Ô§Ú§ã§ä§â§Ñ LSFR §Õ§Ý§ñ §â§Ö§Ñ§Ý§Ú§Ù§Ñ§è§Ú§Ú §Ñ§Ý§Ô§à§â§Ú§ä§Þ§Ñ Crypto-1
    uint32_t CRYPTO_ONE_LSFR_EVEN = 0x00A0A1A2;//§¹§×§ä§ß§í§Ö §Ò§Ú§ä§í
    uint32_t CRYPTO_ONE_LSFR_ODD  = 0x00A3A4A5;//§¯§Ö§é§×§ä§ß§í§Ö §Ò§Ú§ä§í
    
    uint32_t fc_input_value;
    uint8_t  fc_output_value;
    uint32_t feedback_value;    
    uint8_t  bit_value;

    time = 150;

    while(time--)    
    {
        //Reloads IWDG counter
        IWDG_ReloadCounter();

        //§±§Ñ§å§Ù§Ñ
        Delay_Ms(100);
        
        for(i = 0x00; i < 19; i++)
        {
            led_value = 0x00;

            for(j = 0x00; j < 0x18; j++)
            {
                fc_input_value  = ((0x0000D938 << 0x00) >> ((CRYPTO_ONE_LSFR_ODD >> 0x10) & 0x0F)) & 0x01;
                fc_input_value |= ((0x0000F22C << 0x01) >> ((CRYPTO_ONE_LSFR_ODD >> 0x0C) & 0x0F)) & 0x02;
                fc_input_value |= ((0x0000F22C << 0x02) >> ((CRYPTO_ONE_LSFR_ODD >> 0x08) & 0x0F)) & 0x04;
                fc_input_value |= ((0x0000D938 << 0x03) >> ((CRYPTO_ONE_LSFR_ODD >> 0x04) & 0x0F)) & 0x08;
                fc_input_value |= ((0x0000F22C << 0x04) >> ((CRYPTO_ONE_LSFR_ODD >> 0x01) & 0x0F)) & 0x10;
                fc_output_value = (uint8_t) ((0xEC57E80A >> fc_input_value) & 0x01);
                led_value <<= 0x01;
                led_value |= fc_output_value;

                feedback_value  = CRYPTO_ONE_LSFR_EVEN & 0x00870804;
                feedback_value ^= CRYPTO_ONE_LSFR_ODD & 0x0029CE5C;
                feedback_value = (feedback_value ^ (feedback_value >> 0x10)) & 0xFFFF;
                feedback_value = (feedback_value ^ (feedback_value >> 0x08)) & 0xFF;
                feedback_value = (feedback_value ^ (feedback_value >> 0x04)) & 0x0F;
                feedback_value = (0x6996 >> feedback_value) & 0x01;

                feedback_value = (feedback_value & 0x03) ^ ((feedback_value >> 0x02) & 0x03);
                feedback_value = (feedback_value & 0x01) ^ ((feedback_value >> 0x01) & 0x01);
                bit_value = feedback_value;

                CRYPTO_ONE_LSFR_EVEN = CRYPTO_ONE_LSFR_EVEN << 0x01;
                CRYPTO_ONE_LSFR_EVEN = CRYPTO_ONE_LSFR_EVEN ^ CRYPTO_ONE_LSFR_ODD;//E1 = E0 XOR O0
                CRYPTO_ONE_LSFR_ODD  = CRYPTO_ONE_LSFR_EVEN ^ CRYPTO_ONE_LSFR_ODD;//O1 = E1 XOR O0 = (E0 XOR O0) XOR O0 = E0
                CRYPTO_ONE_LSFR_EVEN = CRYPTO_ONE_LSFR_EVEN ^ CRYPTO_ONE_LSFR_ODD;//E2 = E1 XOR O1 = (E0 XOR O0) XOR O1 = (E0 XOR O0) XOR E0 = O0
                CRYPTO_ONE_LSFR_ODD |= bit_value;
            }

            led_state[i] = led_value;
        }

        for(i = 0x00; i < 19; i++)
        {
            WS2812B_SEND_PIXEL(led_state[i]);
        }        
    }
}

/*********************************************************************
 * @fn      main
 *
 * @brief   Main program.
 *
 * @return  none
 */
int main(void)
{
    uint32_t program;

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
    SystemCoreClockUpdate();
    GPIO_Function_Init();
    SPI_Function_Init();
    ADC_Function_Init();
    IWDG_Init(IWDG_Prescaler_256, 512);
    Delay_Init();

    //Reset WS2812B
    WS2812B_RESET();

    //§³§Ò§â§à§ã §ß§à§Þ§Ö§â§Ñ §á§â§à§Ô§â§Ñ§Þ§Þ§í
    program = 0x10;

    while(0xFF)
    {
        //Reloads IWDG counter
        IWDG_ReloadCounter();

        //§±§Ñ§å§Ù§Ñ
        Delay_Ms(100);

        switch(program)
        {
            case 0x00:
            {
                led_program_0();
                break;
            }
            case 0x01:
            {
                led_program_1();
                break;
            }         
            case 0x02:
            {
                led_program_2();
                break;
            }     
            case 0x03:
            {
                led_program_3();
                break;
            }       
            case 0x04:
            {
                led_program_4();
                break;
            }    
            case 0x05:
            {
                led_program_5();
                break;
            }     
            case 0x06:
            {
                led_program_6();
                break;
            }    
            case 0x07:
            {
                led_program_7();
                break;
            }    
            case 0x08:
            {
                led_program_8();
                break;
            }    
            case 0x09:
            {
                led_program_9();
                break;
            }   
            case 0x0A:
            {
                led_program_a();
                break;
            }     
            case 0x0B:
            {
                led_program_b();
                break;
            }     
            case 0x0C:
            {
                led_program_c();
                break;
            }    
            case 0x0D:
            {
                led_program_d();
                break;
            } 
            case 0x0E:
            {
                led_program_e();
                break;
            }   
            case 0x0F:
            {
                led_program_f();
                break;
            }   
            case 0x10:
            {
                led_program_10();
                break;
            }                                                                                                                                                        
            default:
            {
                program = 0x00;
                break;
            }
        }

        program++;
        if(program >= 0x10) program = 0x00;
    }
}
