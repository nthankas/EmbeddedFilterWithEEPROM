#include <sys/attribs.h>
#include <stdio.h>
#include <xc.h>
#include "BOARD.h"
#include "uart.h"
#include "Protocol2.h"
#include "MessageIDS.h"
#include "I2C.h"

void NOP_delay(int ms) {
    for (int i = 0; i < ms; i++) {
        for (int j = 0; j < 8000; j++) {
            asm("nop");
        }
    }
}

unsigned int I2C_Init(unsigned int Rate) {
    if (I2C1BRG != 0) {
        return 0;
    }
    
    // clear all registers
    I2C1CON = 0;
    I2C1STAT = 0;
    I2C1ADD = 0;
    I2C1MSK = 0;
    I2C1TRN = 0;
    I2C1RCV = 0;
    I2C1BRG = 0;
    
    if (Rate == I2C_DEFAULT_RATE) {
        I2C1BRG = 0x00C5;
    }
    else if (Rate == 400000) {
        I2C1BRG = 0x002F;
    } else {
        return 0xFFFFFFFF;
    }
    
    I2C1CONbits.ON = 1; //enable
    return Rate;
    
}

unsigned char I2C_ReadRegister(unsigned char I2CAddress, unsigned char deviceRegisterAddress) {
    I2C1CONbits.SEN = 1;
    while (I2C1CONbits.SEN);
    
    I2C1TRN = (I2CAddress << 1)| 0;
    while (I2C1STATbits.TRSTAT);
    
    I2C1TRN = (deviceRegisterAddress >> 8) & 0xFF;
    while (I2C1STATbits.TRSTAT);
    I2C1TRN = deviceRegisterAddress & 0xFF;
    while (I2C1STATbits.TRSTAT);
    
    I2C1CONbits.RSEN = 1;
    while (I2C1CONbits.RSEN);
    
    I2C1TRN = (I2CAddress << 1) | 1;
    while (I2C1STATbits.TRSTAT);
    
    I2C1CONbits.RCEN = 1;
    while (!I2C1STATbits.RBF);
    
    unsigned char receivedData = I2C1RCV;
    I2C1CONbits.PEN = 1;
    while (I2C1CONbits.PEN);
    return receivedData; 

}

unsigned char I2C_WriteReg(unsigned char I2CAddress, unsigned char deviceRegisterAddress, char data) {
    I2C1CONbits.SEN = 1;
    while (I2C1CONbits.SEN);
    
    I2C1TRN = (I2CAddress << 1) | 0;
    while (I2C1STATbits.TRSTAT);
    
    I2C1TRN = (deviceRegisterAddress >> 8) & 0xFF;
    while (I2C1STATbits.TRSTAT);
    I2C1TRN = deviceRegisterAddress & 0xFF;
    while (I2C1STATbits.TRSTAT);
    
    I2C1TRN = data;
    while (I2C1STATbits.TRSTAT);
    
    I2C1CONbits.PEN = 1;
    while (I2C1CONbits.PEN);
    
    NOP_delay(5);
    return 1;
    
}

void I2C_ReadPage(unsigned char I2CAddress, unsigned int deviceRegisterAddress, unsigned char *buffer, int len) {
    I2C1CONbits.SEN = 1;
    while (I2C1CONbits.SEN);
    
    I2C1TRN = (I2CAddress << 1)| 0;
    while (I2C1STATbits.TRSTAT);
    
    I2C1TRN = (deviceRegisterAddress >> 8) & 0xFF;
    while (I2C1STATbits.TRSTAT);
    I2C1TRN = deviceRegisterAddress & 0xFF;
    while (I2C1STATbits.TRSTAT);
    
    I2C1CONbits.RSEN = 1;
    while (I2C1CONbits.RSEN);
    I2C1TRN = (I2CAddress << 1) | 1;
    while (I2C1STATbits.TRSTAT);
    
    for (int i = 0; i < len; i++) {
        I2C1CONbits.RCEN = 1;
        while (!I2C1STATbits.RBF);
        buffer[i] = I2C1RCV;
        
        if (i < len - 1) {
            I2C1CONbits.ACKDT = 0;
            I2C1CONbits.ACKEN = 1;
            while (I2C1CONbits.ACKEN);
        }
    }
    
    I2C1CONbits.ACKDT = 1;
    I2C1CONbits.ACKEN = 1;
    while (I2C1CONbits.ACKEN);
    
    I2C1CONbits.PEN = 1;
    while (I2C1CONbits.PEN);
}

void I2C_WritePage(unsigned char I2CAddress, unsigned int deviceRegisterAddress, unsigned char *buffer, int len) {
    I2C1CONbits.SEN = 1;
    while (I2C1CONbits.SEN);
    
    I2C1TRN = (I2CAddress << 1)| 0;
    while (I2C1STATbits.TRSTAT);
    
    I2C1TRN = (deviceRegisterAddress >> 8) & 0xFF;
    while (I2C1STATbits.TRSTAT);
    I2C1TRN = deviceRegisterAddress & 0xFF;
    while (I2C1STATbits.TRSTAT);
    
    for (int i = 0; i < len; i++) {
        I2C1TRN = buffer[i];
        while (I2C1STATbits.TRSTAT);
    }
    
    I2C1CONbits.PEN = 1;
    while (I2C1CONbits.PEN);
    
    NOP_delay(5);
    
}


