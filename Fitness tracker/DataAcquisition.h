/*
 *  Created on: 21/04/2022
 *      Authors: Zheng chao, Franco Lyu
 */

#ifndef DATAACQUISITION_H_
#define DATAACQUISITION_H_

typedef struct{
    int16_t x;
    int16_t y;
    int16_t z;
} vector3_t;

void initAccl (void);

vector3_t getAccl (void);

int16_t getAccl_Mean(circBuf_t* dataBuff, int buffer_size);

int16_t getNorm(vector3_t acc);

#endif /* DATAACQUISITION_H_ */
