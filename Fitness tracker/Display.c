/* Display.c
 * Module that control the OLED diplay and data update.
 *  Created on: 21/04/2022
 *      Authors: Zheng chao, Franco Lyu
 *      Group: 62
 */



#include <stdint.h>
#include <stdbool.h>
#include "OrbitOLED/OrbitOLEDInterface.h"
#include "utils/ustdlib.h"
#include "circBufT.h"


//initialize OLED display
void initDisplay (void) {
    OLEDInitialise ();
}


//Update data to OLED.
void DataUpdate (int16_t steps, float km, float mile, uint32_t goal, uint32_t potentiometer, int state, uint8_t charLine, int steps_percentage, int unitState) {

    OLEDStringDraw ("          ", 0, charLine);
    char text_buffer[17];
    int km1,km2,km3;                 //distance for kilometer and two decimal
    int mile1,mile2,mile3;           //distance for Mile and two decimal
    km1 = km/1000;                   //Integer bit kilometer
    km2 = (km-km1*1000)/100;         //first decimal kilometer
    km3 = (km-km1*1000-km2*100)/10;  //second decimal kilometer
    mile1 = mile/1000;               //Integer bit number Mile
    mile2 = (mile-mile1*1000)/100;   //first decimal Mile
    mile3 = (mile-mile1*1000-mile2*100)/10;   //second decimal Mile
     
    //According to the state of the unit to update data. 
    switch (state) {

    case 0:
        if (unitState == 0) {
            usnprintf(text_buffer, sizeof(text_buffer), "%d", steps);
        } else {
            usnprintf(text_buffer, sizeof(text_buffer), "%d", steps_percentage);
        }
        break;

    case 1:
        if (unitState == 0) {
            usnprintf(text_buffer, sizeof(text_buffer), "%d.%d%d", km1, km2, km3);
        } else {
            usnprintf(text_buffer, sizeof(text_buffer), "%d.%d%d", mile1, mile2, mile3);
        }
        break;

    case 2:
        if (unitState == 0) {
            usnprintf(text_buffer, sizeof(text_buffer), "%d", goal);
        } else if (unitState == 1) {
            usnprintf(text_buffer, sizeof(text_buffer), "%d", potentiometer*100);
        }
        break;

    }

    OLEDStringDraw (text_buffer, 2, charLine);

}



void statesDisplay(int state, int unitState) {

    OLEDStringDraw ("                 ", 0, 0);
    OLEDStringDraw ("                 ", 0, 1);
    OLEDStringDraw ("                 ", 0, 2);
    OLEDStringDraw ("                 ", 0, 3);

    //According to the state to display units and screens. 
    switch (state) {

    case 0:
        //text display for steps
        OLEDStringDraw ("Number Of Steps:", 0, 0);
        if (unitState == 0) {
            OLEDStringDraw ("Steps", 10, 3);
        } else if (unitState == 1) {
            //Text display for percentage of goal
            OLEDStringDraw ("%    ", 10, 3);
        }
        break;

    case 1:
        //text display for two different units of distance
        OLEDStringDraw ("Distance:", 0, 0);
        if (unitState == 0) {
            OLEDStringDraw ("Km   ", 10, 3);
        } else if (unitState == 1) {
            OLEDStringDraw ("Mile ", 10, 3);
        }
        break;

    case 2:
        //text display for set goal
        OLEDStringDraw ("NewGoal:", 0, 0);
        OLEDStringDraw ("Steps", 10, 1);
        OLEDStringDraw ("CurrentGoal:", 0, 2);
        OLEDStringDraw ("Steps", 10, 3);
        break;

    case 3:
        //when set goal is succeed
        OLEDStringDraw ("Setup completed", 0, 1);
        break;

    case 4:
        //when long press to reset
        OLEDStringDraw ("Reset completed", 0, 1);
        break;

    case 5:
        //when achieve the goal
        OLEDStringDraw ("Goal completed", 0, 0);
        OLEDStringDraw ("Down Bottom-->", 0, 2);
        break;

    }
}
