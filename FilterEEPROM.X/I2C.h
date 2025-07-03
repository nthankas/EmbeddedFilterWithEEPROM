/*
* File: I2C.h
* Original Author: Max
* Created on February 22, 2018, 1:17 PM
* Revised for ece121 W2023 by scp
*/
#ifndef I2C_H
#define I2C_H
#define I2C_DEFAULT_RATE 100000
/**
* @Function I2C_Init(Rate)
* @param Rate - Clock rate for the I2C system
* @return The clock rate set for the I2C system, 0 if already inited
* @brief Initializes the I2C System for use with the intended peripheral
*/
unsigned int I2C_Init(unsigned int Rate);
/**
* @Function I2C_ReadRegister(unsigned char I2CAddress,unsigned char
deviceRegisterAddress)
* @param I2CAddresss - 7-bit address of I2C remote device
* @param deviceRegisterAddress - 8-bit address of register on remote device
* @return Value at Register or 0
* @brief Reads one device register on chosen I2C device
*/
unsigned char I2C_ReadRegister(unsigned char I2CAddress,unsigned char
deviceRegisterAddress);
/**
* @Function I2C_WriteReg(unsigned char I2CAddress, unsigned char
deviceRegisterAddress, char data)
* @param I2CAddresss - 7-bit address of I2C device wished to interact with
* @param deviceRegisterAddress - 8-bit address of register on device
* @param data - data to be written to remote device's register
* @return 0 if error and 1 if success
* @brief Writes one device register on remote I2C device
*/
unsigned char I2C_WriteReg(unsigned char I2CAddress, unsigned char
deviceRegisterAddress, char data);
/**
* @Function I2C_ReadInt(char I2CAddress, char deviceRegisterAddress, char
isBigEndian)
* @param I2CAddresss - 7-bit address of I2C device wished to interact with
* @param deviceRegisterAddress - 8-bit lower address of register on device
* @param isBigEndian - Boolean determining if device is big or little endian
* @return 0 if error and 1 if success
* @brief Reads two sequential registers to build a 16-bit value. isBigEndian
* whether the first bits are either the high or low bits of the value
*/
int I2C_ReadInt(char I2CAddress, char deviceRegisterAddress, char isBigEndian);

// added

void I2C_ReadPage(unsigned char I2CAddress, unsigned int deviceRegisterAddress, unsigned char *buffer, int len);
void I2C_WritePage(unsigned char I2CAddress, unsigned int deviceRegisterAddress, unsigned char *buffer, int len);

#endif /* I2C_H */
