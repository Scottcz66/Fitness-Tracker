/* StateControl.c
 * Module that update state of the display and unit.
 *  Created on: 21/04/2022
 *      Authors: Zheng chao, Franco Lyu
 *      Group: 62
 */

#include "buttons4.h"

int
stateUpdate(int state)
{
    //if LEFT button pressed, it will change three different cases clockwise.
    switch (checkButton(LEFT)) {
    case PUSHED:
        if (state == 0) {
            state = 1;
        } else if (state == 1) {
            state = 2;
        } else if (state == 2) {
            state = 0;
        }
        break;
    }
    //if RIGHT button pressed, it will change three different cases counterclockwise.
    switch (checkButton(RIGHT)) {
    case PUSHED:
        if (state == 0) {
            state = 2;
        } else if (state == 1) {
            state = 0;
        } else if (state == 2) {
            state = 1;
        }
        break;
    }

    return state;

}



int
unitUpdate(int unitState) {
    //push UP to change data unit, if test mode is on, j
    switch (checkButton(UP)) {
    case PUSHED:
        if (unitState == 0) {
            unitState = 1;
        } else {
            unitState = 0;
        }
        break;
    }

    return unitState;
}
