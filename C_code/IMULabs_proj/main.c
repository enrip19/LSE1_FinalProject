/* DriverLib Defines */
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>

/* Standard Defines */
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "lib/mock.h"
#include "lib/misc.h"
#include "lib/uartLib.h"
#include "lib/i2cLib.h"

uint8_t mida_TX;

int main(void)
{
    /* Disabling the Watchdog  */
        WDT_A_holdTimer();

        /* inicialitzo el smclk a 24Mhz */
        init_ucs_24MHz();

        /* inicialitzacio FPU pels floating points*/
        init_FPU();

        /* inicialitzacio dels botons i leds */
        //init_buttons();

        /* inicialitzacio de la UART (configuracio) */
        init_UART();


        /* Faig un hardReset IMU */
        hard_reset_imu();

        /* inicialitzacio de l'I2C */
        init_I2C();

        /* inicialitzacio del pin de les interrupcions de la imu */
        init_IMU_int();

        /* inicilatizacio IMU */
        config_IMU();




        //Habilitacio interrupcions master
        Interrupt_enableMaster();

        /* Programa */
       //Inicialitzacio de estat
       /*Maquina d'estats d'enviament de UART i lectura*/
        while(1)
        {
            if (imuFlag == 1) { //si la interrupcio de la imu ha pujat (es a dir, dades preparades) comencem a llegir dades
                //Flags de UART avall per comenc,ar nets
                txFlagUART = 0;
                rxFlagUART = 0;

                //lectura de dades al fitxer de mocking
                MagnetoX = magnetoX[indexFSM];
                MagnetoY = magnetoY[indexFSM];
                MagnetoZ = magnetoZ[indexFSM];
                Angle = angleMot[indexFSM];
                Fase = faseMot[indexFSM];
                //lectura de dades reals a la imu
                llegirIMU();
                saveIMU();
                //creo la string que enviare en csv i la guardo a enviat
                mida_TX = sprintf(enviatUART, ":%.2f,%.2f,%.2f;%.2f,%.2f,%.2f;%.2f,%.2f,%.2f;%u,%u;%.2f\n", AccelX, AccelY, AccelZ,
                        GyroX, GyroY, GyroZ, MagnetoX, MagnetoY, MagnetoZ, Angle, Fase, Temp); 
                //printf("%d\n", mida_TX);

                midaTXUART = 3600;              //mida de la array
                TXDataUART = 0;                 //resetejo TXData
                txFlagUART = 0;                 //poso flag del TX a 0
                i = 0;

                while(TXDataUART!=0x0A)       //mentres TXData no sigui \n (0x0A en ASCII) enviem dadaes
                {
                    txFlagUART = 0;
                    TXDataUART = enviatUART[i];     //guardo el valor a TXData
                    MAP_UART_transmitData(EUSCI_A0_BASE,TXDataUART);    //envio TXdata
                    while(txFlagUART == 0){} //espero fins que la txFlagUART biaxi a 0
                    i++;                    //augmento indexFSM
                }

                if (indexFSM < (midaTXUART-1))
                {
                    indexFSM++; //passo de punt de dades.
                }

                else
                {
                    indexFSM = 0;
                }
                imuFlag = 0;
            }
            else{
                MAP_PCM_gotoLPM0(); //si la imu no dona senyals de vida, la placa s'en va a dormir
            }
        }
}





