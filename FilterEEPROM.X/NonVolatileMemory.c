#include <sys/attribs.h>
#include <stdio.h>
#include <xc.h>
#include "BOARD.h"
#include "uart.h"
#include "Protocol2.h"
#include "MessageIDS.h"
#include "I2C.h"
#include "NonVolatileMemory.h"

// #define NVMtest

#ifdef NVMtest

int main(void) {
    BOARD_Init();
    Protocol_Init(115200);
    I2C_Init(I2C_DEFAULT_RATE);

    // I2C address for the NVM device (modify as needed)
    unsigned char I2CAddr = 0b1010000; 

    packet testpack = {0};
    Protocol_SendDebugMessage("Debug: nvm test activated");
   
    while (1) {
        if (BuildRxPacket(&testpack, 0)) {

            if (testpack.ID == ID_NVM_READ_BYTE) {
                // Use payload indices 0-3 for the 32-bit address.
                unsigned int deviceRegisterAddr = ((unsigned int)testpack.payLoad[0] << 24) |
                                                  ((unsigned int)testpack.payLoad[1] << 16) |
                                                  ((unsigned int)testpack.payLoad[2] << 8)  |
                                                  ((unsigned int)testpack.payLoad[3]);
                unsigned char data = I2C_ReadRegister(I2CAddr, deviceRegisterAddr);
                Protocol_SendPacket(1, ID_NVM_READ_BYTE_RESP, &data);
            }
            else if (testpack.ID == ID_NVM_WRITE_BYTE) {
                // Payload: bytes 0-3 = address, byte 4 = data.
                unsigned int deviceRegisterAddr = ((unsigned int)testpack.payLoad[0] << 24) |
                                                  ((unsigned int)testpack.payLoad[1] << 16) |
                                                  ((unsigned int)testpack.payLoad[2] << 8)  |
                                                  ((unsigned int)testpack.payLoad[3]);
                char data = testpack.payLoad[4];
                I2C_WriteReg(I2CAddr, deviceRegisterAddr, data);
                    unsigned char dat;
                Protocol_SendPacket(1, ID_NVM_WRITE_BYTE_ACK, &dat);
            }
            else if (testpack.ID == ID_NVM_READ_PAGE) {
                // Payload: bytes 0-3 = address.
                unsigned int deviceRegisterAddr = ((unsigned int)testpack.payLoad[0] << 32) |
                                                  ((unsigned int)testpack.payLoad[1] << 16) |
                                                  ((unsigned int)testpack.payLoad[2] << 8)  |
                                                  ((unsigned int)testpack.payLoad[3]);
                unsigned char page[64];
                I2C_ReadPage(I2CAddr, deviceRegisterAddr, page, 64);
                Protocol_SendPacket(64, ID_NVM_READ_PAGE_RESP, page);
            }
            else if (testpack.ID == ID_NVM_WRITE_PAGE) {
                // Payload: bytes 0-3 = address, remaining bytes = page data.
                unsigned int deviceRegisterAddr = ((unsigned int)testpack.payLoad[0] << 24) |
                                                  ((unsigned int)testpack.payLoad[1] << 16) |
                                                  ((unsigned int)testpack.payLoad[2] << 8)  |
                                                  ((unsigned int)testpack.payLoad[3]);
                // Total payload is (testpack.len - 1) bytes; subtract 4 bytes for the address.
                int len = testpack.len - 4;
                I2C_WritePage(I2CAddr, deviceRegisterAddr, testpack.payLoad+4, len);
                unsigned char dat;

                Protocol_SendPacket(1, ID_NVM_WRITE_PAGE_ACK, &dat);
            }
             else if (Protocol_QueuePacket(testpack)){
                Protocol_SendDebugMessage("Packet Queue full");
            }
        }
        
         packet* inpack;
        if (Protocol_GetInPacket(&inpack)){
            Protocol_ParsePacket(*inpack);
        }
        
    }

    return 0;
}

#endif

