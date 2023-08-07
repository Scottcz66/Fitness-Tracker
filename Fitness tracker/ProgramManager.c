/* ProgramManager.c
 * Module that hold all the datas and running the program.
 *  Created on: 21/04/2022
 *      Authors: Zheng chao, Franco Lyu
 *      Group: 62
 */



#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "driverlib/adc.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "circBufT.h"
#include "driverlib/gpio.h"
#include "buttons4.h"
#include "StateControl.h"
#include "Display.h"
#include "DataAcquisition.h"



//*****************************************************************************
// Constants
//*****************************************************************************
#define SYSTICK_RATE_HZ 1000

#define PTRBuffer_size 30
#define acclBuffer_size 20



//*****************************************************************************
// Global variables
//*****************************************************************************
static uint32_t systick_clock = 0;
static uint32_t accl_clock = 0;
static uint32_t potentiometer_clock = 0;
static uint32_t flag_clock = 0;
static int downBtn_clock = 0;
static int notice_clock = 0;

circBuf_t xBuffer;
circBuf_t yBuffer;
circBuf_t zBuffer;
circBuf_t PTRBuffer;

static vector3_t rawData_accl;
static vector3_t meanData_accl;
static uint16_t norm = 0;
static uint32_t goal = 1000;
static int16_t steps = 0;
static int goal_percentage = 0;
static float km = 0;
static float mile = 0;

static uint32_t potentiometer = 0;

static int state = 0;
static int unit_state = 0;

static bool norm_flag = 1;
static bool test_flag = 0;
static bool downBtn_flag = 0;
static bool notice_flag = 0;



//*****************************************************************************
//In test mode, if Up button is pressed then increase 100 steps, if Down button is pressed then decrease 500 steps.
//*****************************************************************************
void testMode(void) {

    switch (checkButton(UP)) {
    case PUSHED:
        steps = steps + 100;
        break;
    }

    switch (checkButton(DOWN)) {
    case PUSHED:
        if (steps >= 500) {
            steps -= 500;
        } else {
            steps = 0;
        }
         break;
    }

}


//*****************************************************************************
//Caculate mean value of the potentiometer.
//*****************************************************************************
uint16_t getPotentiometer(circBuf_t PTRBuffer) {

    uint16_t i;
    int32_t sum = 0;
    uint16_t ans = 0;

    for (i = 0; i < PTRBuffer_size; i++)
        sum = sum + readCircBuf (&PTRBuffer);

    ans = (2 * sum + PTRBuffer_size) / 2 / PTRBuffer_size;

    return ans;

}



//*****************************************************************************
//Write the data get from the accelerometer to buffers, and calculate mean values of them.
//*****************************************************************************
void accl_event(void) {

    rawData_accl = getAccl();
    writeCircBuf(&xBuffer, rawData_accl.x);
    writeCircBuf(&yBuffer, rawData_accl.y);
    writeCircBuf(&zBuffer, rawData_accl.z);
    meanData_accl.x = getAccl_Mean(&xBuffer, acclBuffer_size); //有问题
    meanData_accl.y = getAccl_Mean(&yBuffer, acclBuffer_size);
    meanData_accl.z = getAccl_Mean(&zBuffer, acclBuffer_size);

    norm = getNorm(rawData_accl); //暂时用初始值

}



//*****************************************************************************
//When the Down button is pressed set flags true and set the new  goal when button released and it has been short pressed.
//*****************************************************************************
void downBtn_Event(void) {

    switch (checkButton(DOWN)) {

    case PUSHED:
        if (state == 2) {
            downBtn_flag = true;
            notice_flag = true;
        } else if (state == 1 || state == 0) {
            downBtn_flag = true;
        } else if (state == 5) {
            state = 0;
        }
        break;

    case RELEASED:
        if (state == 2) {
            goal = potentiometer * 100;
            DataUpdate (steps, km, mile, goal, potentiometer, state, 3, goal_percentage, 0);
            state = 3;
        }
        downBtn_flag = false;
        downBtn_clock = 0;
        break;
    }

}



//*****************************************************************************
//Reset all variables.
//*****************************************************************************
void reset(void) {
    steps = 0;
    goal = 1000;
    km = 0;
    mile = 0;
    goal_percentage = 0;
    unit_state = 0;
}



//*****************************************************************************
// (ISR) The interrupt handler for the for SysTick interrupt.
//*****************************************************************************
void SysTickIntHandler(void) {
    systick_clock++;
}



//*****************************************************************************
// Initialization functions for the clock (incl. SysTick), ADC, display.
//*****************************************************************************
void initClock (void) {

    // Set the clock rate to 20 MHz
    SysCtlClockSet (SYSCTL_SYSDIV_10 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN |
                   SYSCTL_XTAL_16MHZ);
    //
    // Set up the period for the SysTick timer.  The SysTick timer period is
    // set as a function of the system clock.
    SysTickPeriodSet(SysCtlClockGet() / SYSTICK_RATE_HZ);
    //
    // Register the interrupt handler
    SysTickIntRegister(SysTickIntHandler);
    //
    // Enable interrupt and device
    SysTickIntEnable();
    SysTickEnable();

}



//*****************************************************************************
// Enable SW1 Peripheral.
//*****************************************************************************
void initSwitch(void) {

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    GPIOPadConfigSet(GPIO_PORTA_BASE, GPIO_PIN_7, GPIO_STRENGTH_4MA, GPIO_PIN_TYPE_STD_WPD);
    GPIODirModeSet(GPIO_PORTA_BASE, GPIO_PIN_7, GPIO_DIR_MODE_HW);

}


//*****************************************************************************
// (ISR) The interrupt handler for the potentiometer.
//*****************************************************************************
void PotentiometerIntHandler(void) {
    uint32_t adc0_value;

    //
    // Get the single sample from ADC0.  ADC_BASE is defined in
    // inc/hw_memmap.h
    ADCSequenceDataGet(ADC0_BASE, 3, &adc0_value);
    //
    // Place it in the circular buffer (advancing write index)
    writeCircBuf (&PTRBuffer, adc0_value);
    //
    // Clean up, clearing the interrupt
    ADCIntClear(ADC0_BASE, 3);
}


//*****************************************************************************
// Initialize the potentiometer.
//*****************************************************************************
void
initPotentiometer (void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
    ADCSequenceConfigure(ADC0_BASE, 3, ADC_TRIGGER_PROCESSOR, 0);
    ADCSequenceStepConfigure(ADC0_BASE, 3, 0, ADC_CTL_CH0 | ADC_CTL_IE |
                             ADC_CTL_END);
    ADCSequenceEnable(ADC0_BASE, 3);
    //
    // Register the interrupt handler
    ADCIntRegister (ADC0_BASE, 3, PotentiometerIntHandler);

    //
    // Enable interrupts for ADC0 sequence 3 (clears any outstanding interrupts)
    ADCIntEnable(ADC0_BASE, 3);
}


//*****************************************************************************
// Initialize the buffers.
//*****************************************************************************
void initBuffers (){
    initCircBuf (&PTRBuffer, PTRBuffer_size);
    initCircBuf (&xBuffer, acclBuffer_size);
    initCircBuf (&yBuffer, acclBuffer_size);
    initCircBuf (&zBuffer, acclBuffer_size);
}

//*****************************************************************************
// Initialize all.
//*****************************************************************************
void initialize(void) {

    initBuffers ();
    initDisplay();
    initClock();
    initButtons ();
    initAccl ();
    initPotentiometer ();
    initSwitch();
    statesDisplay(state, unit_state);
    DataUpdate (steps, km, mile, goal, potentiometer, state, 3, goal_percentage, unit_state);

}



int main(void) {

    int last_state = state;
    int last_unit_state = unit_state;
    uint16_t last_steps = steps;
    uint32_t last_potentiometer = potentiometer;

    initialize();

    while(1){

        updateButtons();

        //Set the test flag true if Pin 7 is high (switch 1).
        test_flag = GPIOPinRead(GPIO_PORTA_BASE,GPIO_PIN_7);
        //Record all current states.
        last_state = state;
        last_unit_state = unit_state;
        last_potentiometer = potentiometer;
        last_steps = steps;
        //Update state.
        state = stateUpdate(state);

        //Reset and change to the inform state for 1 second if goal is completed.
        if (steps >= goal) {
            reset();
            state = 5;
        }

        //If test flag is true then enter the test mode, otherwise do the normal Down button action.
        if (test_flag && state != 2 && state != 5) {
            testMode();
        } else {
            //Check whether Up button is pressed.
            unit_state = unitUpdate(unit_state);
            downBtn_Event();
            //Reset if Down button has been long pressed and change to the inform state to tell the user reset is completed.
            if (downBtn_clock > 15)
            {
                reset();
                notice_flag = true;
                state = 4;
            }
        }

        //Return to the previous state after the inform state (1 second long display).
        if (notice_clock > 10) {
            switch (state) {
            case 3:
                state = 2;
                break;
            case 4:
                state = 0;
                break;
            case 5:
                state = 0;
                break;
            }
            notice_flag = false;
            notice_clock = 0;
        }

        //Update the screen if state is changed or unit is changed.
        if (state != last_state || (last_unit_state != unit_state && state != 2)) {
            statesDisplay(state, unit_state);
            DataUpdate (steps, km, mile, goal, potentiometer, state, 3, goal_percentage, unit_state);
            if (state == 2) {
                DataUpdate (steps, km, mile, goal, potentiometer, state, 1, goal_percentage, 1);
            }

        }

        //20 HZ (1000/50)
        if (systick_clock - accl_clock >= 50) {
            accl_event();
            accl_clock = systick_clock;
        }

        //50HZ (1000/20)
        if (systick_clock - potentiometer_clock >= 20) {
            ADCProcessorTrigger(ADC0_BASE, 3);
            potentiometer = getPotentiometer(PTRBuffer);

            potentiometer_clock = systick_clock;
        }

        //10HZ (1000/100)
        if (systick_clock - flag_clock >= 100) {
            if (downBtn_flag) { downBtn_clock++; }
            if (notice_flag) {  notice_clock++; }
            flag_clock = systick_clock;
        }

        //Step detection.
        if (norm >= 130 && norm_flag == 0 && state != 2 && test_flag == false && state != 5) {
             steps++;
             norm_flag = 1;
        } else if (norm < 95) {
             norm_flag = 0;
        }

        //Update the screen and other related variables if step value changed.
        if (steps != last_steps && state != 2) {
            km = steps * 0.9;
            mile = steps * 0.559234073;
            if (goal_percentage < 100) {
                goal_percentage = (steps*100) / goal;
            }
            DataUpdate (steps, km, mile, goal, potentiometer, state, 3, goal_percentage, unit_state);
        }

        //Update screen if value of potentiometer changed.
        if (state == 2 && (last_potentiometer != potentiometer)) {
            DataUpdate (steps, km, mile, goal, potentiometer, state, 1, goal_percentage, 1);
        }

    }

}
