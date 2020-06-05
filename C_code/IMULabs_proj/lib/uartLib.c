/*
 * uartLib.c
 *
 *  Created on: 29 abr. 2020
 *      Author: enric
 */



#include "uartLib.h"



//inicialitzacio de la UART
void init_UART(void)
{
    /* Constant de configuracio per driverlib de la UART A (A0) */
    const eUSCI_UART_ConfigV1 UARTconfig =
    {
         EUSCI_A_UART_CLOCKSOURCE_SMCLK,         //Posem com a clock source el SMCLK (UCA0CTLW0)
         13,                                    //clockPrescalar = 13 (BRDIV = 13)
         0,                                     //firstModReg = 0  (UC0BRF = 0)
         37,                                    //secondModReg = 37 (UC0BRS = 37)
         EUSCI_A_UART_NO_PARITY,                //poso en mode no paritat
         EUSCI_A_UART_LSB_FIRST,                //poso primer el LSB al byte de dades
         EUSCI_A_UART_ONE_STOP_BIT,             //dic que hi haura dos stops bits
         EUSCI_A_UART_MODE,                     //poso que la EUSCI esta en mode UART
         EUSCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION     //Poso que hi hagi oversampling amb els valors del principi
         //EUSCI_A_UART_8_BIT_LEN                 //poso que la dada te una llargada de 8 bits
    };

    //habilitem pins per funcio de UART (p1.2 I P1.3)
    MAP_GPIO_setAsPeripheralModuleFunctionInputPin(
    GPIO_PORT_P1,
    GPIO_PIN2 | GPIO_PIN3,
    GPIO_PRIMARY_MODULE_FUNCTION
    );
    //configuracio de la UART
    MAP_UART_initModule(EUSCI_A0_BASE, &UARTconfig); //Exectuem la config de la UART
    MAP_UART_enableModule(EUSCI_A0_BASE);            //Habilitem la UART

    /* Enabling EUSCI interrupts */
    MAP_UART_enableInterrupt(EUSCI_A0_BASE, EUSCI_A_UART_RECEIVE_INTERRUPT);
    MAP_UART_enableInterrupt(EUSCI_A0_BASE, EUSCI_A_UART_TRANSMIT_INTERRUPT);
    MAP_Interrupt_enableInterrupt(INT_EUSCIA0);
}



/*******************************************************************************
 * eUSCIB0 ISR. The repeated start and transmit/receive operations happen
 * within this ISR.
 *******************************************************************************/

/* Modul dinterrupcions per a la UART */
void EUSCIA0_IRQHandler(void)
{
    uint32_t status = MAP_UART_getEnabledInterruptStatus(EUSCI_A0_BASE);    //creem una variable d'estat on hi ha els registres de uart

    MAP_UART_clearInterruptFlag(EUSCI_A0_BASE, status); //borra les flags d'interrupcio (modul, variable)

    if(status & EUSCI_A_UART_RECEIVE_INTERRUPT_FLAG) //estem fent una mascara on mirem nomes el bit de la flag d'interrupcio de rx. Si es 1 fem l'if, sino no
    {
        RXDataUART = MAP_UART_receiveData(EUSCI_A0_BASE); //igualem a la variable a la posicio i de la RXData el que hi tenim el registre d'entrada de la UART
        //rebutUART[i] = RXDataUART; //Guardo a l'array rebut per comparar despres a CHECK
        rxFlagUART = 1;
    }

    if(status & EUSCI_A_UART_TRANSMIT_INTERRUPT_FLAG) //estem fent una mascara on mirem el bit de flag d'ISR de byte transmes
    {
        txFlagUART = 1;
    }
}
