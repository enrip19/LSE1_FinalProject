/*
 * i2cLib.h
 *
 *  Created on: 29 abr. 2020
 *      Author: enric
 */

#ifndef LIB_I2CLIB_H_
#define LIB_I2CLIB_H_

/* DriverLib Defines */
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>

/* Standard Defines */
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

/* Slave Address for I2C Slave */
#define NUM_OF_REC_BYTES    14 //buffer d'entrada (ja que esta programat
#define PIN_VCC_VIRTUAL     BIT3

/* definicions IMU */
#define SLAVE_ADDRESS       0x68 //adreça i2c de la imu
//registres de config
#define PWR_MGMT_1          0x6B //adr del reg de config de Power Management de la imu (per deshabilitar mode sleep)
#define INT_ENABLE_REG          0x38 //adr del reg de config de interrupcions de la imu
#define ACCEL_CONFIG_REG    0x1C //adr del reg de config de l'accel de la imu
#define GYRO_CONFIG_REG     0x1B //adr del reg de config del gyro de la imu
#define WHO_AM_I_REG        0x75 //adr del reg que conte la adr i2c de la config de la imu
#define ACCEL_XOUT_H_REG    0x3B //adr del reg de config del byte de mes pes de dades de l'accel
//començare a llegir en mode burst a partir d'aquest ultim (per aixo no defineixo els demes, ja que ho fa la imu sola)

/* sensibilitats IMU */
#define FS_SEL_0            0x00
#define FS_SEL_1            0x08
#define FS_SEL_2            0x10
#define FS_SEL_3            0x18
//Accelerometre
#define ACCEL_SENS_2G       16384
#define ACCEL_SENS_4G       8192
#define ACCEL_SENS_8G       4096
#define ACCEL_SENS_16       2048

//Temperatura
#define TEMP_CELSIUS_DIV    340
#define TEMP_CELSIUS_PLUS   36.53

//Gyroscopi
#define GYRO_SENS_250       131
#define GYRO_SENS_500       65.5
#define GYRO_SENS_1000      32.8
#define GYRO_SENS_2000      16.4

//vars I2C
static uint8_t RXDataI2C[NUM_OF_REC_BYTES];
static volatile uint32_t I2Cindex;
static volatile bool stopSent;

//var interrupcio imu
bool imuFlag;

//defineixo struct per tractar dades de la imu
typedef struct {
    int16_t AccelX;
    int16_t AccelY;
    int16_t AccelZ;
    int16_t Temp;
    int16_t GyroX;
    int16_t GyroY;
    int16_t GyroZ;
} str_dataIMU;

//vars llegides de la imu
float AccelX, AccelY, AccelZ;
float Temp;
float GyroX, GyroY, GyroZ;

/* Funcions */

void init_I2C(void);

void hard_reset_imu();

void init_IMU_int(void);

void config_IMU(void);

void llegirIMU(void);

void saveIMU(void);



#endif /* LIB_I2CLIB_H_ */
