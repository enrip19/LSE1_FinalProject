/*
 * misc.c
 *
 *  Created on: 29 abr. 2020
 *      Author: enric
 */

/* DriverLib Defines */
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>

/* Standard Defines */
#include <stdio.h>
#include <stdint.h>

#include "misc.h"

uint16_t delay_ms_count;


//inicialitzacio del rellotge
uint8_t init_ucs_24MHz(){ //by Christophe
    //Configuracion del reloj a 24MHz, a partir del DCO del micro.
    //Sin ningun ajuste, dejando todo por defecto, saldria MCLK del orden de 3MHz basado en el DCO.

    PJSEL0 |= BIT0 + BIT1; //activando pines de entradas LFXT de 32kHz

    ////Ajustes para utilizar el DCO a 24MHz:
    CS->KEY = 0x695A; //contraseï¿½a para poder modificar los registros protegido CSCTLn
    CS->CTL0 = CS_CTL0_DCORSEL_4 ; // Seleccionamos rango de DCO 16MHz-32MHz
    //  sin ningun otro ajuste, se queda en la frec. nominal central de 24MHz.

    CS->CTL1 = CS_CTL1_SELS__DCOCLK |CS_CTL1_DIVS__1  |  CS_CTL1_SELM__DCOCLK; //Oscilador fuente para cada reloj

    //ACLK queda por defecto a LFXT, que es el cuarzo de 32kHz
    while (!(CS->STAT & (CS_STAT_MCLK_READY | CS_STAT_SMCLK_READY | CS_STAT_ACLK_READY))); //me espero hasta que el registro CSSTAT me indique que todos mis relojes se han estabilizado

      //  Activar los pines siguientes para comprobar y debugar los relojes -->
      P4SEL0 |= BIT2|BIT3|BIT4; //activo funciones alternativas primarias MCLK (P4.3), HSMCLK (P4.4) y ACLK (P4.2),
      //para poder observar los relojes con el osciloscopio.
      // <-- Volver a comentar una vez comprobado que los relojes estan bien configurados.
      //Los correspondientes bits en P4SEL1 han de estar a 0, que es su valor por defecto al iniciar el micro
      P4DIR |=BIT2|BIT3|BIT4;   //activo salida MCLK (P4.3), HSMCLK (P4.4) y ACLK (P4.2)
      // <-- Volver a comentar una vez comprobado que los relojes estan bien configurados.

     /*En este punto, tenemos:
      * MCLK = 24 MHz
      * SMCLK = 24 MHz
      * ACLK = LFXTCLK = 32 kHz (el cuarzo)
      */

    return 24;
}

//inicialitzacio de la FPU (unitat per calcular FLOATS)
void init_FPU(void)
{
   /* Enabling the FPU with stacking enabled (for use within ISR) */
   FPU_enableModule();             //habilitacio de la floating-point unit
   FPU_enableLazyStacking();
}


void timer_init(void){
    /* Timer_A UpMode Configuration Parameter */
    const Timer_A_UpModeConfig upConfig =
    {
            TIMER_A_CLOCKSOURCE_ACLK,               // ACLK Clock Source
            TIMER_A_CLOCKSOURCE_DIVIDER_1,          // ACLK/1
            32,                                     // 32 tick period (1ms)
            TIMER_A_TAIE_INTERRUPT_DISABLE,         // Disable Timer interrupt
            TIMER_A_CCIE_CCR0_INTERRUPT_ENABLE ,    // Enable CCR0 interrupt
            TIMER_A_DO_CLEAR                        // Clear value
    };

    /* Configuring Timer_A1 for Up Mode */
    MAP_Timer_A_configureUpMode(TIMER_A2_BASE, &upConfig);
    MAP_Interrupt_enableInterrupt(INT_TA2_0);
}

void delay(uint16_t time_ms){

    delay_ms_count = 0;

    MAP_Timer_A_clearTimer(TIMER_A2_BASE);
    MAP_Timer_A_startCounter(TIMER_A2_BASE, TIMER_A_UP_MODE);

    while(delay_ms_count < time_ms){
        MAP_PCM_gotoLPM0();
    }

    MAP_Timer_A_stopTimer(TIMER_A2_BASE);

}

/* ISR */
void TA2_0_IRQHandler(void)
{
    // Clear interrupt flag
    MAP_Timer_A_clearCaptureCompareInterrupt(
    TIMER_A2_BASE,
    TIMER_A_CAPTURECOMPARE_REGISTER_0
    );
    // Increment ms counter
    delay_ms_count++;
}

/*
//inicialitzacio dels leds i botons (per proxims projectes)
void init_GPIO(void)
{
    //habilito el port1 pin1 i port1 pin 4 com input per als botons
    MAP_GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P1, GPIO_PIN4);
    MAP_GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P1, GPIO_PIN1); //Poso una R de pulldonw per a que quan el boto
    //sigui pulsat el valor passi a ser 0.

    //borro flags i habilito interrupcions de cada boto
    MAP_GPIO_clearInterruptFlag(GPIO_PORT_P1, GPIO_PIN1);
    MAP_GPIO_enableInterrupt(GPIO_PORT_P1, GPIO_PIN1);
    MAP_GPIO_clearInterruptFlag(GPIO_PORT_P1, GPIO_PIN4);
    MAP_GPIO_enableInterrupt(GPIO_PORT_P1, GPIO_PIN4);

    //habilito interrupcions del port 1 i borro el buttonStatus
    MAP_Interrupt_enableInterrupt(INT_PORT1);
    buttonStatus = 0;
}*/


/* M�dul d'interrupcions per als polsadors (si els necessitessim)
void PORT1_IRQHandler(void)
{
    uint32_t status;

    status = MAP_GPIO_getEnabledInterruptStatus(GPIO_PORT_P1);
    MAP_GPIO_clearInterruptFlag(GPIO_PORT_P1, status);      //netejo flags d'interrupcio

    if(status & GPIO_PIN1)
    {
    buttonStatus = 0;
    }
    else if(status & GPIO_PIN4)
    {
        buttonStatus = 1;                                //quan s'activa el boto li dono valor a la variable d'estat
    }
 }
*/
