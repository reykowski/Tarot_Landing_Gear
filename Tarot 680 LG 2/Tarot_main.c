/* --COPYRIGHT--,BSD
 * Copyright (c) 2015, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * --/COPYRIGHT--*/
/*******************************************************************************
 *
 * main.c
 *
 * Out of Box Demo for the MSP-EXP430FR6989
 * Main loop, initialization, and interrupt service routines
 *
 * This demo provides 2 application modes: Stopwatch Mode and Temperature Mode
 *
 * The stopwatch mode provides a simple stopwatch application that supports split
 * time, where the display freezes while the stopwatch continues running in the
 * background.
 *
 * The temperature mode provides a simple thermometer application using the
 * on-chip temperature sensor. Display toggles between C/F.
 *
 * February 2015
 * E. Chen
 *
 ******************************************************************************/
#include <msp430.h>
#include <driverlib.h>
#include "StopWatchMode.h"
#include "TempSensorMode.h"
#include "hal_LCD.h"

#define STARTUP_MODE         0

volatile unsigned char mode = STARTUP_MODE;
volatile unsigned char stopWatchRunning = 0;
volatile unsigned char tempSensorRunning = 0;
volatile unsigned char S1buttonDebounce = 0;
volatile unsigned char S2buttonDebounce = 0;
volatile unsigned int holdCount = 0;
volatile unsigned int counter = 0;
volatile unsigned int wait_for_echo = 0;
volatile unsigned long Echo_Start = 0;
volatile unsigned long Echo_End = 0;
volatile unsigned long Pulse_Duration_us = 0;
volatile unsigned long Pulse_Duration_avg_us = 0;
volatile unsigned long Distance_mm = 0;
volatile unsigned long D6 = 0;
volatile unsigned long D5 = 0;
volatile unsigned long D4 = 0;
volatile unsigned long D3 = 0;
volatile unsigned long D2 = 0;
volatile unsigned long D1 = 0;
volatile unsigned long P = 0;
volatile unsigned long P1 = 0;
volatile unsigned long P2 = 0;
volatile unsigned long P3 = 0;
volatile unsigned long M1 = 0;
volatile unsigned long M2 = 0;
volatile unsigned long M3 = 0;
volatile unsigned long x = 0;
volatile int centisecond = 0;
Calendar currentTime;

// TimerA0 UpMode Configuration Parameter
Timer_A_initUpModeParam initUpParam_A0 =
{
        TIMER_A_CLOCKSOURCE_SMCLK,              // SMCLK Clock Source
        TIMER_A_CLOCKSOURCE_DIVIDER_1,          // SMCLK/4 = 2MHz
        30000,                                  // 15ms debounce period
        TIMER_A_TAIE_INTERRUPT_DISABLE,         // Disable Timer interrupt
        TIMER_A_CCIE_CCR0_INTERRUPT_ENABLE ,    // Enable CCR0 interrupt
        TIMER_A_DO_CLEAR,                       // Clear value
        true                                    // Start Timer
};

// Initialization calls
void Init_GPIO(void);
void Init_Clock(void);


/*
 * Main routine
 */
int main(void) {
    // Stop watchdog timer
    WDT_A_hold(__MSP430_BASEADDRESS_WDT_A__);

    // Disable the GPIO power-on default high-impedance mode to activate
    // previously configured port settings
    PMM_unlockLPM5();

    //enable interrupts
    __enable_interrupt();

    // Initializations
    Init_GPIO();
    Init_Clock();
    Init_LCD();

    GPIO_clearInterrupt(GPIO_PORT_P1, GPIO_PIN1);
    GPIO_clearInterrupt(GPIO_PORT_P1, GPIO_PIN2);
    GPIO_clearInterrupt(GPIO_PORT_P1, GPIO_PIN6);
    GPIO_clearInterrupt(GPIO_PORT_P1, GPIO_PIN7);

    __enable_interrupt();

    displayScrollText("GEAR CONTROL");





/*

    // Configure GPIO for PWM
     P1DIR |= BIT3 ;                     //  P1.3 output
     P1SEL0 |= BIT3 ;                    //  P1.3 options select: Assign P1.3 as output to Timer 1
     PJSEL0 |= BIT4 | BIT5;

*/

     // Disable the GPIO power-on default high-impedance mode to activate
     // previously configured port settings
     PM5CTL0 &= ~LOCKLPM5;

     // XT1 Setup
     CSCTL0_H = CSKEY >> 8;                    // Unlock CS registers
     CSCTL1 = DCOFSEL_6;                       // Set DCO to 8MHz
     CSCTL2 = SELA__LFXTCLK | SELS__DCOCLK | SELM__DCOCLK; // Set ACLK = XT1; MCLK = DCO
     CSCTL3 = DIVA__1 | DIVS__2 | DIVM__2;     // Set all dividers
     CSCTL4 &= ~LFXTOFF;

     do
     {
       CSCTL5 &= ~LFXTOFFG;                    // Clear XT1 fault flag
       SFRIFG1 &= ~OFIFG;
     }while (SFRIFG1&OFIFG);                   // Test oscillator fault flag

     CSCTL0_H = 0;                             // Lock CS registers
     TA1CCR0 = 590*12;                            // PWM Period 18ms*12=216ms
     TA1CCTL2 = OUTMOD_7;                      // CCR2 reset/set
     TA1CCR2 = 33;                              // CCR2 PWM duty cycle = 1ms , signal output is P1.3
     TA1CTL = TASSEL__ACLK | MC__UP | TACLR;   // ACLK, up mode, clear TAR


    int i = 0x01;

    while(1)
    {
        LCD_C_selectDisplayMemory(LCD_C_BASE, LCD_C_DISPLAYSOURCE_MEMORY);
        switch(mode)
        {
            case STARTUP_MODE:        // Startup mode
                // Set RTC counter to trigger interrupt every ~250 ms
                RTC_C_initCounter(RTC_C_BASE, RTC_C_CLOCKSELECT_32KHZ_OSC, RTC_C_COUNTERSIZE_16BIT);
                RTC_C_definePrescaleEvent(RTC_C_BASE, RTC_C_PRESCALE_1, RTC_C_PSEVENTDIVIDER_32);
                RTC_C_enableInterrupt(RTC_C_BASE, RTC_C_PRESCALE_TIMER1_INTERRUPT);
                RTC_C_startClock(RTC_C_BASE);

                // Cycle through all LCD segments and display instruction message
                if (i <= 0x80)

                {
                	/*
                    LCDMEM[pos1] = LCDMEM[pos1+1] = i;
                    LCDMEM[pos2] = LCDMEM[pos2+1] = i;
                    LCDMEM[pos3] = LCDMEM[pos3+1] = i;
                    LCDMEM[pos4] = LCDMEM[pos4+1] = i;
                    LCDMEM[pos5] = LCDMEM[pos5+1] = i;
                    LCDMEM[pos6] = LCDMEM[pos6+1] = i;
                    LCDM14 = i << 4;
                    LCDM18 = i;
                    LCDM3 = i;
                    i<<=1;
                    */
                }

                else
                {
                    i=1;
                    clearLCD();
                    displayScrollText("HOLD S1 AND S2 TO SWITCH MODES");
                }
                __bis_SR_register(LPM3_bits | GIE);         // enter LPM3
                __no_operation();
                break;
            case STOPWATCH_MODE:         // Stopwatch Timer mode
                clearLCD();              // Clear all LCD segments


                //stopWatchModeInit();     // Initialize stopwatch mode
                //stopWatch();
                break;
            case TEMPSENSOR_MODE:        // Temperature Sensor mode
                clearLCD();              // Clear all LCD segments
                displayScrollText("TEMPSENSOR MODE");


                //tempSensorModeInit();    // initialize temperature mode
                //tempSensor();
                break;
        }
    }
}


/*
 * GPIO Initialization
 */
void Init_GPIO()
{
    // Set all GPIO pins to output low to prevent floating input and reduce power consumption
    GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN0|GPIO_PIN1|GPIO_PIN2|GPIO_PIN3|GPIO_PIN4|GPIO_PIN5|GPIO_PIN6|GPIO_PIN7);
    GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN0|GPIO_PIN1|GPIO_PIN2|GPIO_PIN3|GPIO_PIN4|GPIO_PIN5|GPIO_PIN6|GPIO_PIN7);
    GPIO_setOutputLowOnPin(GPIO_PORT_P3, GPIO_PIN0|GPIO_PIN1|GPIO_PIN2|GPIO_PIN3|GPIO_PIN4|GPIO_PIN5|GPIO_PIN6|GPIO_PIN7);
    GPIO_setOutputLowOnPin(GPIO_PORT_P4, GPIO_PIN0|GPIO_PIN1|GPIO_PIN2|GPIO_PIN3|GPIO_PIN4|GPIO_PIN5|GPIO_PIN6|GPIO_PIN7);
    GPIO_setOutputLowOnPin(GPIO_PORT_P5, GPIO_PIN0|GPIO_PIN1|GPIO_PIN2|GPIO_PIN3|GPIO_PIN4|GPIO_PIN5|GPIO_PIN6|GPIO_PIN7);
    GPIO_setOutputLowOnPin(GPIO_PORT_P6, GPIO_PIN0|GPIO_PIN1|GPIO_PIN2|GPIO_PIN3|GPIO_PIN4|GPIO_PIN5|GPIO_PIN6|GPIO_PIN7);
    GPIO_setOutputLowOnPin(GPIO_PORT_P7, GPIO_PIN0|GPIO_PIN1|GPIO_PIN2|GPIO_PIN3|GPIO_PIN4|GPIO_PIN5|GPIO_PIN6|GPIO_PIN7);
    GPIO_setOutputLowOnPin(GPIO_PORT_P8, GPIO_PIN0|GPIO_PIN1|GPIO_PIN2|GPIO_PIN3|GPIO_PIN4|GPIO_PIN5|GPIO_PIN6|GPIO_PIN7);
    GPIO_setOutputLowOnPin(GPIO_PORT_P9, GPIO_PIN0|GPIO_PIN1|GPIO_PIN2|GPIO_PIN3|GPIO_PIN4|GPIO_PIN5|GPIO_PIN6|GPIO_PIN7);

    GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN0|GPIO_PIN1|GPIO_PIN2|GPIO_PIN3|GPIO_PIN4|GPIO_PIN5|GPIO_PIN6|GPIO_PIN7);
    GPIO_setAsOutputPin(GPIO_PORT_P2, GPIO_PIN0|GPIO_PIN1|GPIO_PIN2|GPIO_PIN3|GPIO_PIN4|GPIO_PIN5|GPIO_PIN6|GPIO_PIN7);
    GPIO_setAsOutputPin(GPIO_PORT_P3, GPIO_PIN0|GPIO_PIN1|GPIO_PIN2|GPIO_PIN3|GPIO_PIN4|GPIO_PIN5|GPIO_PIN6|GPIO_PIN7);
    GPIO_setAsOutputPin(GPIO_PORT_P4, GPIO_PIN0|GPIO_PIN1|GPIO_PIN2|GPIO_PIN3|GPIO_PIN4|GPIO_PIN5|GPIO_PIN6|GPIO_PIN7);
    GPIO_setAsOutputPin(GPIO_PORT_P5, GPIO_PIN0|GPIO_PIN1|GPIO_PIN2|GPIO_PIN3|GPIO_PIN4|GPIO_PIN5|GPIO_PIN6|GPIO_PIN7);
    GPIO_setAsOutputPin(GPIO_PORT_P6, GPIO_PIN0|GPIO_PIN1|GPIO_PIN2|GPIO_PIN3|GPIO_PIN4|GPIO_PIN5|GPIO_PIN6|GPIO_PIN7);
    GPIO_setAsOutputPin(GPIO_PORT_P7, GPIO_PIN0|GPIO_PIN1|GPIO_PIN2|GPIO_PIN3|GPIO_PIN4|GPIO_PIN5|GPIO_PIN6|GPIO_PIN7);
    GPIO_setAsOutputPin(GPIO_PORT_P8, GPIO_PIN0|GPIO_PIN1|GPIO_PIN2|GPIO_PIN3|GPIO_PIN4|GPIO_PIN5|GPIO_PIN6|GPIO_PIN7);
    GPIO_setAsOutputPin(GPIO_PORT_P9, GPIO_PIN0|GPIO_PIN1|GPIO_PIN2|GPIO_PIN3|GPIO_PIN4|GPIO_PIN5|GPIO_PIN6|GPIO_PIN7);



    // Configure GPIO for PWM
    //P1DIR |= BIT3 ;                     //  P1.3 output
    GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN3);
     //P1SEL0 |= BIT3 ;                    //  P1.3 options select: Assign P1.3 as output to Timer 1
    GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P1, GPIO_PIN3,  GPIO_PRIMARY_MODULE_FUNCTION) ;

    // PJSEL0 |= BIT4 | BIT5;
     GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_PJ, GPIO_PIN4 + GPIO_PIN5,  GPIO_PRIMARY_MODULE_FUNCTION) ;


    GPIO_setAsInputPin(GPIO_PORT_P3, GPIO_PIN5);

    // Configure button S1 (P1.1) interrupt
    GPIO_selectInterruptEdge(GPIO_PORT_P1, GPIO_PIN1, GPIO_HIGH_TO_LOW_TRANSITION);
    GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P1, GPIO_PIN1);
    GPIO_clearInterrupt(GPIO_PORT_P1, GPIO_PIN1);
    GPIO_enableInterrupt(GPIO_PORT_P1, GPIO_PIN1);

    // Configure button S2 (P1.2) interrupt
    GPIO_selectInterruptEdge(GPIO_PORT_P1, GPIO_PIN2, GPIO_HIGH_TO_LOW_TRANSITION);
    GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P1, GPIO_PIN2);
    GPIO_clearInterrupt(GPIO_PORT_P1, GPIO_PIN2);
    GPIO_enableInterrupt(GPIO_PORT_P1, GPIO_PIN2);

    // Configure Trigger (P1.3) interrupt
    GPIO_selectInterruptEdge(GPIO_PORT_P1, GPIO_PIN3, GPIO_HIGH_TO_LOW_TRANSITION);
    //GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P1, GPIO_PIN2);
    GPIO_clearInterrupt(GPIO_PORT_P1, GPIO_PIN3);
    GPIO_enableInterrupt(GPIO_PORT_P1, GPIO_PIN3);

    // Configure Echo Start Input (P1.6) interrupt
    GPIO_selectInterruptEdge(GPIO_PORT_P1, GPIO_PIN6, GPIO_LOW_TO_HIGH_TRANSITION);
    GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P1, GPIO_PIN6);
    GPIO_clearInterrupt(GPIO_PORT_P1, GPIO_PIN6);
    GPIO_enableInterrupt(GPIO_PORT_P1, GPIO_PIN6);

    // Configure Echo End Input (P1.7) interrupt
    GPIO_selectInterruptEdge(GPIO_PORT_P1, GPIO_PIN7, GPIO_HIGH_TO_LOW_TRANSITION);
    GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P1, GPIO_PIN7);
    GPIO_clearInterrupt(GPIO_PORT_P1, GPIO_PIN7);
    GPIO_enableInterrupt(GPIO_PORT_P1, GPIO_PIN7);

    // Set P4.1 and P4.2 as Secondary Module Function Input, LFXT.
    GPIO_setAsPeripheralModuleFunctionInputPin(
           GPIO_PORT_PJ,
           GPIO_PIN4 + GPIO_PIN5,
           GPIO_PRIMARY_MODULE_FUNCTION
           );

    // Disable the GPIO power-on default high-impedance mode
    // to activate previously configured port settings
    PMM_unlockLPM5();
}

/*
 * Clock System Initialization
 */
void Init_Clock()
{
    // Set DCO frequency to default 8MHz
    CS_setDCOFreq(CS_DCORSEL_0, CS_DCOFSEL_6);

    // Configure MCLK and SMCLK to default 2MHz
    CS_initClockSignal(CS_MCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_8);
    CS_initClockSignal(CS_SMCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_8);

    // Intializes the XT1 crystal oscillator
    CS_turnOnLFXT(CS_LFXT_DRIVE_3);
}

/*
 * RTC Interrupt Service Routine
 * Wakes up every ~10 milliseconds to update stowatch
 */
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=RTC_VECTOR
__interrupt
#elif defined(__GNUC__)
__attribute__((interrupt(RTC_VECTOR)))
#endif
void RTC_ISR(void)
{
    switch(__even_in_range(RTCIV, 16))
    {
    case RTCIV_NONE: break;      //No interrupts
    case RTCIV_RTCOFIFG: break;      //RTCOFIFG
    case RTCIV_RTCRDYIFG:             //RTCRDYIFG
        counter = RTCPS;
        centisecond = 0;
        __bic_SR_register_on_exit(LPM3_bits);
        break;
    case RTCIV_RTCTEVIFG:             //RTCEVIFG
        //Interrupts every minute
        __no_operation();
        break;
    case RTCIV_RTCAIFG:             //RTCAIFG
        __no_operation();
        break;
    case RTCIV_RT0PSIFG:
        centisecond = RTCPS - counter;
        __bic_SR_register_on_exit(LPM3_bits);
        break;     //RT0PSIFG
    case RTCIV_RT1PSIFG:
        __bic_SR_register_on_exit(LPM3_bits);
        break;     //RT1PSIFG

    default: break;
    }
}

/*
 * PORT1 Interrupt Service Routine
 * Handles S1 and S2 button press interrupts
 */
#pragma vector = PORT1_VECTOR
__interrupt void PORT1_ISR(void)
{
    switch(__even_in_range(P1IV, P1IV_P1IFG7))
    {
        case P1IV_NONE : break;
        case P1IV_P1IFG0 : break;
        case P1IV_P1IFG1 :    // Button S1 pressed
            P1OUT |= BIT0;    // Turn LED1 On


            TA1CCTL2 = OUTMOD_7;                      // CCR2 reset/set
            TA1CCR2 = 33;                              // CCR2 PWM duty cycle = 1ms , signal output is P1.3
            TA1CTL = TASSEL__ACLK | MC__UP | TACLR;   // ACLK, up mode, clear TAR
            showChar(' ',pos1);
            showChar('D',pos2);
            showChar('O',pos3);
            showChar('W',pos4);
            showChar('N',pos5);
            showChar(' ',pos6);
            //         __delay_cycles(100000);


            if ((S1buttonDebounce) == 0)
            {
                // Set debounce flag on first high to low transition
                S1buttonDebounce = 1;
                holdCount = 0;\
                // Start debounce timer
                Timer_A_initUpMode(TIMER_A0_BASE, &initUpParam_A0);
            }
            break;
        case P1IV_P1IFG2 :    // Button S2 pressed


            TA1CCTL2 = OUTMOD_7;                      // CCR2 reset/set
            TA1CCR2 = 66;                              // CCR2 PWM duty cycle = 2ms , signal output is P1.3
            TA1CTL = TASSEL__ACLK | MC__UP | TACLR;   // ACLK, up mode, clear TAR
            showChar(' ',pos1);
            showChar(' ',pos2);
            showChar('U',pos3);
            showChar('P',pos4);
            showChar(' ',pos5);
            showChar(' ',pos6);


            P9OUT |= BIT7;    // Turn LED2 On
            if ((S2buttonDebounce) == 0)
            {
                // Set debounce flag on first high to low transition
                S2buttonDebounce = 1;
                holdCount = 0;
 /*               switch (mode)
               {
                    case STOPWATCH_MODE:
                        break;
                    case TEMPSENSOR_MODE:
                        break;
                }
*/
                // Start debounce timer
                Timer_A_initUpMode(TIMER_A0_BASE, &initUpParam_A0);
            }
            break;
        case P1IV_P1IFG3 :
			x=D1;
        	break;
        case P1IV_P1IFG4 : break;
        case P1IV_P1IFG5 : break;
        case P1IV_P1IFG6 :
        	if (wait_for_echo == 0)
        	{
            	Echo_Start = TA1R;
            	wait_for_echo = 1;
        	}

        	//showChar('T',pos1);
        	//showChar('R',pos2);
        	//showChar('I',pos3);
        	//showChar('G',pos4);
        	//showChar('G',pos5);
        	//showChar('R',pos6);
        	break;
        case P1IV_P1IFG7 :
        	Echo_End = TA1R;
        	if ((Echo_End > Echo_Start) && (wait_for_echo == 1))
        	{
                wait_for_echo = 0;
        		Pulse_Duration_us = (Echo_End - Echo_Start)*(1000/330);    //Pulse Duration in us
        	}
        	else
        	{
                wait_for_echo = 0;
                Echo_Start = 0;
        		break;
        	}
        	P3 = P2;
        	P2 = P1;
        	P1 = Pulse_Duration_us;
        	M1 = P1;
        	M2 = P2;
        	M3 = P3;
        	if (M1>M2)
        	{
        		x = M2;
        		M2 = M1;
        		M1 = x;
        	}
        	if (M2>M3)
        	{
        		x = M3;
        		M3 = M2;
        		M2 = x;
        	}
        	if (M1>M2)
        	{
        		x = M2;
        		M2 = M1;
        		M1 = x;
        	}

        	//Pulse_Duration_avg_us = (Pulse_Duration_avg_us*100+Pulse_Duration_us)/101;
        	Pulse_Duration_avg_us =M2;
        	Distance_mm = (343.2/100)*Pulse_Duration_avg_us/2;
        	P =  Distance_mm;
        	D6 = P/100000;
        	P = P -(D6*100000);
        	D5 = P/10000;
        	P = P -(D5*10000);
        	D4 = P/1000;
        	P = P -(D4*1000);
        	D3 = P/100;
        	P = P -(D3*100);
        	D2 = P/10;
        	P = P -(D2*10);
        	D1 = P;
    		showChar(D6+'0',pos1);
    		showChar(D5+'0',pos2);
    		showChar(D4+'0',pos3);
    		showChar(D3+'0',pos4);
    		showChar(D2+'0',pos5);
    		showChar(D1+'0',pos6);
    		if (Distance_mm <100)
    		{
    			x=D1;
    		}
            Echo_Start = 0;
    		break;
    }
}

/*
 * Timer A0 Interrupt Service Routine
 * Used as button debounce timer
 */
#pragma vector = TIMER0_A0_VECTOR
__interrupt void TIMER0_A0_ISR (void)
{
    // Both button S1 & S2 held down
    if (!(P1IN & BIT1) && !(P1IN & BIT2))
    {
        holdCount++;
        if (holdCount == 40)
        {
            // Stop Timer A0
            Timer_A_stop(TIMER_A0_BASE);

            // Change mode
            if (mode == STARTUP_MODE)
                mode = STOPWATCH_MODE;
            else if (mode == STOPWATCH_MODE)
            {
                mode = TEMPSENSOR_MODE;
                stopWatchRunning = 0;
                // Hold RTC
                RTC_C_holdClock(RTC_C_BASE);
            }
            else if (mode == TEMPSENSOR_MODE)
            {
                mode = STOPWATCH_MODE;
                tempSensorRunning = 0;
                // Disable ADC12, TimerA1, Internal Ref and Temp used by TempSensor Mode
                ADC12_B_disable(ADC12_B_BASE);
                ADC12_B_disableConversions(ADC12_B_BASE,true);

                Timer_A_stop(TIMER_A1_BASE);
            }
            __bic_SR_register_on_exit(LPM3_bits);                // exit LPM3
        }
    }

    // Button S1 released
    if (P1IN & BIT1)
    {
        S1buttonDebounce = 0;                                   // Clear button debounce
        P1OUT &= ~BIT0;
    }

    // Button S2 released
    if (P1IN & BIT2)
    {
        S2buttonDebounce = 0;                                   // Clear button debounce
        P9OUT &= ~BIT7;
    }

    // Both button S1 & S2 released
    if ((P1IN & BIT1) && (P1IN & BIT2))
    {
        // Stop timer A0
        Timer_A_stop(TIMER_A0_BASE);
    }

    if (mode == STOPWATCH_MODE || mode == TEMPSENSOR_MODE)
        __bic_SR_register_on_exit(LPM3_bits);            // exit LPM3
}

/*
 * ADC 12 Interrupt Service Routine
 * Wake up from LPM3 to display temperature
 */
#pragma vector=ADC12_VECTOR
__interrupt void ADC12_ISR(void)
{
    switch(__even_in_range(ADC12IV,12))
    {
    case  0: break;                         // Vector  0:  No interrupt
    case  2: break;                         // Vector  2:  ADC12BMEMx Overflow
    case  4: break;                         // Vector  4:  Conversion time overflow
    case  6: break;                         // Vector  6:  ADC12BHI
    case  8: break;                         // Vector  8:  ADC12BLO
    case 10: break;                         // Vector 10:  ADC12BIN
    case 12:                                // Vector 12:  ADC12BMEM0 Interrupt
        ADC12_B_clearInterrupt(ADC12_B_BASE, 0, ADC12_B_IFG0);
        __bic_SR_register_on_exit(LPM3_bits);   // Exit active CPU
        break;                              // Clear CPUOFF bit from 0(SR)
    case 14: break;                         // Vector 14:  ADC12BMEM1
    case 16: break;                         // Vector 16:  ADC12BMEM2
    case 18: break;                         // Vector 18:  ADC12BMEM3
    case 20: break;                         // Vector 20:  ADC12BMEM4
    case 22: break;                         // Vector 22:  ADC12BMEM5
    case 24: break;                         // Vector 24:  ADC12BMEM6
    case 26: break;                         // Vector 26:  ADC12BMEM7
    case 28: break;                         // Vector 28:  ADC12BMEM8
    case 30: break;                         // Vector 30:  ADC12BMEM9
    case 32: break;                         // Vector 32:  ADC12BMEM10
    case 34: break;                         // Vector 34:  ADC12BMEM11
    case 36: break;                         // Vector 36:  ADC12BMEM12
    case 38: break;                         // Vector 38:  ADC12BMEM13
    case 40: break;                         // Vector 40:  ADC12BMEM14
    case 42: break;                         // Vector 42:  ADC12BMEM15
    case 44: break;                         // Vector 44:  ADC12BMEM16
    case 46: break;                         // Vector 46:  ADC12BMEM17
    case 48: break;                         // Vector 48:  ADC12BMEM18
    case 50: break;                         // Vector 50:  ADC12BMEM19
    case 52: break;                         // Vector 52:  ADC12BMEM20
    case 54: break;                         // Vector 54:  ADC12BMEM21
    case 56: break;                         // Vector 56:  ADC12BMEM22
    case 58: break;                         // Vector 58:  ADC12BMEM23
    case 60: break;                         // Vector 60:  ADC12BMEM24
    case 62: break;                         // Vector 62:  ADC12BMEM25
    case 64: break;                         // Vector 64:  ADC12BMEM26
    case 66: break;                         // Vector 66:  ADC12BMEM27
    case 68: break;                         // Vector 68:  ADC12BMEM28
    case 70: break;                         // Vector 70:  ADC12BMEM29
    case 72: break;                         // Vector 72:  ADC12BMEM30
    case 74: break;                         // Vector 74:  ADC12BMEM31
    case 76: break;                         // Vector 76:  ADC12BRDY
    default: break;
    }
}
