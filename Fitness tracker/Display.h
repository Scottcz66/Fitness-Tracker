/*
 *  Created on: 21/04/2022
 *      Authors: Zheng chao, Franco Lyu
 */



#ifndef DISPLAY_H_
#define DISPLAY_H_

void statesDisplay(int state, int unitState);

void DataUpdate (uint16_t steps, float km, float mile, uint32_t goal, uint32_t potentiometer, int state, uint8_t charLine, int steps_percentage, int unitState);

void initDisplay (void);

#endif /* DISPLAY_H_ */
