#include <stdio.h>
#include <stdlib.h>
#include <xc.h>
#include <p18f4620.h>

#include "main.h"
#include "utils.h"

void Init_ADC()
{
    ADCON0 = 0x00;
    ADCON1 = 0x0F; 
    ADCON2 = 0xA9;
}

void Init_UART(void)
{
    OpenUSART (USART_TX_INT_OFF & USART_RX_INT_OFF &
    USART_ASYNCH_MODE & USART_EIGHT_BIT & USART_CONT_RX &
    USART_BRGH_HIGH, 25);
    OSCCON = 0x70;
}

void putch (char c)
{
    while (!TRMT);
    TXREG = c;
}





