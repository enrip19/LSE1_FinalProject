/*
 * misc.h
 *
 *  Created on: 29 abr. 2020
 *      Author: enric
 */

#ifndef LIB_MISC_H_
#define LIB_MISC_H_

//uint8_t buttonStatus;

uint8_t init_ucs_24MHz();
void init_FPU(void);
void timer_init(void);
void delay(uint16_t time_ms);

#endif /* LIB_MISC_H_ */
