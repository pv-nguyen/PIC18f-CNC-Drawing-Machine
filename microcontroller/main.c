#include <stdio.h>
#include <stdlib.h>
#include <xc.h>
#include <math.h>
#include <p18f4620.h>
#include <usart.h>
#include <string.h>

#include "g_code.h"
#include "utils.h"
#include "motor.h"
#include "main.h"

#pragma config OSC = INTIO67
#pragma config WDT = OFF
#pragma config LVP = OFF
#pragma config BOREN = OFF

#define _XTAL_FREQ  8000000             // Set operation for 8 Mhz
#define second 17940
#define min_pulse_delay 200


extern char instructions;
extern float xPos;
extern float yPos;
extern float zPos;

short beginFlag = 0;


void main()
{
    nRBPU = 0;
    Init_UART();
    Init_ADC();
    TRISD = 0x00;
    
    //interrupt emergency stop button initialization
    INTCONbits.GIE = 1;
    INTCONbits.INT0IE = 1;
    INTCONbits.INT0IF = 0;
    INTCON2bits.INTEDG0 = 1;    //rising edge
    TRISBbits.RB0 = 1;
    
    xPos = 0;
    yPos = 0;
    zPos = 0;
    M1Dir = 1;
    M2Dir = 0;
    M1Step = 0;
    M2Step = 0;
    
    //call gcode
    liftServo();
    while (!beginFlag) {}
    printf("Program Starting\r\n");
    run_g_code();
    //moveToXYZ(0,0,2);
    //printf("Program completed\r\n");
    liftServo();
    moveToXYZ(0,0,2);
    
    
    
    while(1){}
}


void interrupt high_priority stop()
{
    printf("Interrupt\r\n");
    for (int k=0; k<20000;k++);
    INTCONbits.INT0IF=0;
    beginFlag = 1;
}