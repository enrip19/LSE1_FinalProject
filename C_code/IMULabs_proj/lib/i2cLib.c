/*
 * i2cLib.c
 *
 *  Created on: 29 abr. 2020
 *      Author: enric
 */
#include "i2cLib.h"

/* Definicio de funcions*/
//inicialtizacio del I2C
void init_I2C (void)
{
    /* I2C Master Configuration Parameter */
    const eUSCI_I2C_MasterConfig i2cConfig =
    {
        EUSCI_B_I2C_CLOCKSOURCE_SMCLK,          // SMCLK Clock Source
        24000000,                                // SMCLK = 3MHz
        EUSCI_B_I2C_SET_DATA_RATE_400KBPS,      // Desired I2C Clock of 400khz
        0,                                      // No byte counter threshold
        EUSCI_B_I2C_NO_AUTO_STOP                // No Autostop
    };
    /* Select Port 1 for I2C - Set Pin 6, 7 to input Primary Module Function,
     *   (UCB0SIMO/UCB0SDA, UCB0SOMI/UCB0SCL).
     */
    MAP_GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P1,
            GPIO_PIN6 + GPIO_PIN7, GPIO_PRIMARY_MODULE_FUNCTION);
    stopSent = false;
    memset(RXDataI2C, 0x00, NUM_OF_REC_BYTES); //posem a 0 RXData

    /* Initializing I2C Master to SMCLK (24Mhz) at 100khz with no autostop */
    MAP_I2C_initMaster(EUSCI_B0_BASE, &i2cConfig);

    /* Specify slave address */
    MAP_I2C_setSlaveAddress(EUSCI_B0_BASE, SLAVE_ADDRESS); //SLAVE_ADDRESS = 0x068 (A0=GND a la imu)

    /* Enable I2C Module to start operations */
    MAP_I2C_enableModule(EUSCI_B0_BASE);
    MAP_Interrupt_enableInterrupt(INT_EUSCIB0);

}

//Vcc virtual IMU
void hard_reset_imu() //By Christophe (Falta cambiar el delaycycles per timer)
{
    //VCC virtual pel pin 2.3
    P2SEL0 &=  ~PIN_VCC_VIRTUAL; //GPIO
    P2SEL1 &=  ~PIN_VCC_VIRTUAL; //GPIO
    P2DIR |= PIN_VCC_VIRTUAL; //salida
    P2DS  |= PIN_VCC_VIRTUAL; //Drive Strength = High
    P2OUT &= ~PIN_VCC_VIRTUAL; //IMU apagada al inicio

    P2OUT &= ~PIN_VCC_VIRTUAL; //apagamos la IMU => "Hardware reset"
    _delay_cycles(500000); //Duracion del pulso "OFF" //Se tendria que hacer con un timer
    P2OUT |= PIN_VCC_VIRTUAL; //y la volvemos a encender
    _delay_cycles(500000);  //esperamos a que se establilice (aprox. 20ms)

}
void init_IMU_int(void){
    MAP_GPIO_setAsInputPin(GPIO_PORT_P5,GPIO_PIN0); //habilito pin 5.0 com a entrada per les interrupcions
    MAP_GPIO_clearInterruptFlag(GPIO_PORT_P5,GPIO_PIN0); //netejo les flags del pin 5.0
    MAP_GPIO_enableInterrupt(GPIO_PORT_P5, GPIO_PIN0); //habilito interrupcions pel pin 5.0
    //activo interrupcions del port 5 i poso la flag a 0
    MAP_Interrupt_enableInterrupt(INT_PORT5);
    imuFlag = 0;
}

//inicialitzacio IMU (trec de Sleep Mode)
void config_IMU(void)
{
    /* configuracio del registre del mode sleep (0x6B) */
    MAP_I2C_masterSendMultiByteStart(EUSCI_B0_BASE, PWR_MGMT_1); //inicio comm I2C i demano escriure al reg 0x6B (sleep mode config)
    MAP_I2C_masterSendMultiByteNext(EUSCI_B0_BASE, 0x00); //poso el registre 0x6B = 0x00 (Posar el bit 7 d'aquest reg vol dir que surti del mode sleep. Com que no fem servir res mes podem posarlo tot a 0, que es el que fem)
    MAP_I2C_masterSendMultiByteStop(EUSCI_B0_BASE); //envio el byte d'stop per finalitzar la comm I2C

    /* configuracio del giroscopi */
    MAP_I2C_masterSendMultiByteStart(EUSCI_B0_BASE, GYRO_CONFIG_REG); //inicio comm I2C i demano escriure al registre 0x1B per configurar el mode de funcionament del Gyro
    MAP_I2C_masterSendMultiByteNext(EUSCI_B0_BASE, FS_SEL_2); //activo el mode 1 (escala de +/- 4g)
    MAP_I2C_masterSendMultiByteStop(EUSCI_B0_BASE); //envio el byte d'stop per finalitzar la comm I2C

    /* configuracio de l'accelerometre */
    MAP_I2C_masterSendMultiByteStart(EUSCI_B0_BASE, ACCEL_CONFIG_REG); //inicio comm I2C i demano escriure al registre 0x1C per configurar el mode de funcionament de l'Accel
    MAP_I2C_masterSendMultiByteNext(EUSCI_B0_BASE, FS_SEL_1); //activo el mode 1 (escala de +/- 500º/s)
    MAP_I2C_masterSendMultiByteStop(EUSCI_B0_BASE); //envio el byte d'stop per finalitzar la comm I2C

    /* configuracio del registre d'interrupcions. Activo interrupcions de dataReady */
    MAP_I2C_masterSendMultiByteStart(EUSCI_B0_BASE, INT_ENABLE_REG); //inicio comm I2c i demano escriure al reg 0x38 (Interrupt enable config)
    MAP_I2C_masterSendMultiByteNext(EUSCI_B0_BASE, 0x01); //activo interrupcions per dataReady
    MAP_I2C_masterSendMultiByteStop(EUSCI_B0_BASE); //envio el byte d'stop per finalitzar la comm i2c
}

void llegirIMU(void)
{
    /* Making sure the last transaction has been completely sent out */
    while (MAP_I2C_masterIsStopSent(EUSCI_B0_BASE));

    MAP_Interrupt_enableSleepOnIsrExit();

    /* Send start and the first byte of the transmit buffer. */
    MAP_I2C_masterSendMultiByteStart(EUSCI_B0_BASE, ACCEL_XOUT_H_REG);

    /* Espero a que el flag de enviament es posi a 1 per a fer la lectura */
    while(!MAP_I2C_getInterruptStatus(EUSCI_B0_BASE,EUSCI_B_I2C_TRANSMIT_INTERRUPT0)); //wait fins que i2c acabi

    /* Sent the first byte, now we need to initiate the read */
    I2Cindex = 0;
    MAP_I2C_masterReceiveStart(EUSCI_B0_BASE);
    MAP_I2C_enableInterrupt(EUSCI_B0_BASE, EUSCI_B_I2C_RECEIVE_INTERRUPT0);

}

void saveIMU(void)
{
    /*Lectura de Dades de la imu */
    int k;
    str_dataIMU IMUrawData;
    str_dataIMU *p_IMU;
    uint8_t *p_Data; //punter per agafar un byte de la imu i colocarlo on toca de la paraula de 16bits

    p_IMU = &IMUrawData;
    p_Data = (uint8_t*) &(p_IMU -> AccelX); //poso a apuntar al primer byte de AccelX

    for (k=0; k<(NUM_OF_REC_BYTES); k++) //passem totes les dades a l'struct IMUrawData
        {
            p_Data[k] = RXDataI2C[k];
        }

    /* Tractament de dades de la imu */
    AccelX = (float) (p_IMU -> AccelX)/ACCEL_SENS_4G;
    AccelY = (float) (p_IMU -> AccelY)/ACCEL_SENS_4G;
    AccelZ = (float) (p_IMU -> AccelZ)/ACCEL_SENS_4G;
    Temp = (float) ((p_IMU -> Temp)/TEMP_CELSIUS_DIV) + TEMP_CELSIUS_PLUS;
    GyroX = (float) (p_IMU -> GyroX)/GYRO_SENS_1000;
    GyroY = (float) (p_IMU -> GyroY)/GYRO_SENS_1000;
    GyroZ = (float) (p_IMU -> GyroZ)/GYRO_SENS_1000;
}

/* ISR */

/* Modul dinterrrupcions per l'I2C */
void EUSCIB0_IRQHandler(void)
{
    uint_fast16_t status;

    status = MAP_I2C_getEnabledInterruptStatus(EUSCI_B0_BASE);

    /* Receives bytes into the receive buffer. If we have received all bytes,
     * send a STOP condition */
    if (status & EUSCI_B_I2C_RECEIVE_INTERRUPT0)
    {

        if (I2Cindex == NUM_OF_REC_BYTES - 2)
        {
            MAP_I2C_disableInterrupt(EUSCI_B0_BASE,
                    EUSCI_B_I2C_RECEIVE_INTERRUPT0);
            MAP_I2C_enableInterrupt(EUSCI_B0_BASE, EUSCI_B_I2C_STOP_INTERRUPT);

            /*
             * Switch order so that stop is being set during reception of last
             * byte read byte so that next byte can be read.
             */
            MAP_I2C_masterReceiveMultiByteStop(EUSCI_B0_BASE);
            RXDataI2C[I2Cindex++] = MAP_I2C_masterReceiveMultiByteNext(
                    EUSCI_B0_BASE);
        } else
        {
            RXDataI2C[I2Cindex++] = MAP_I2C_masterReceiveMultiByteNext(
            EUSCI_B0_BASE);
            //printf("%c",RXData[I2Cindex++]);
        }
    }
    else if (status & EUSCI_B_I2C_STOP_INTERRUPT)
    {
        MAP_Interrupt_disableSleepOnIsrExit();
        MAP_I2C_disableInterrupt(EUSCI_B0_BASE,
                                 EUSCI_B_I2C_STOP_INTERRUPT);
    }
}

/* Modul d'interrupcions per el pin 5.0 (interrupcions imu) */
void PORT5_IRQHandler(void)
{
    uint32_t status;

    status = MAP_GPIO_getEnabledInterruptStatus(GPIO_PORT_P5);
    MAP_GPIO_clearInterruptFlag(GPIO_PORT_P5, status); //netejo les flags de la interrupcio

    if (status & GPIO_PIN0)
    {
        imuFlag = 1;
    }
}
