/*
 * uartLib.h
 *
 *  Created on: 29 abr. 2020
 *      Author: enric
 */
/* DriverLib Defines */
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>

/* Standard Defines */
#include <stdio.h>
#include <stdint.h>

#ifndef LIB_UARTLIB_H_
#define LIB_UARTLIB_H_

int i, k, ok, indexFSM;

//vars UART
uint16_t midaTXUART;
uint8_t RXDataUART, TXDataUART;
char enviatUART [100];

//Flags
bool rxFlagUART, startFlagUART, txFlagUART;

/* Funcions */

void init_UART(void);

#endif /* LIB_UARTLIB_H_ */
