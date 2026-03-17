
#include <stdint.h>
#include "Protocol2.h"
#include "BOARD.h"
#include "uart.h"
#include "MessageIDs.h"
#include "ADCFilter.h"

// Private Functions


typedef enum {
    HEADD,
    LEN,
    ID,
    PAYLOAD,
    TAILL,
    CHECKSUM,
    END1,
    END2
} PacketBuilderState;

typedef struct {
    uint8_t head;
    uint8_t tail;
    packet buffer[PACKETBUFFERSIZE];
    uint8_t isFull;
} PacketBuffer;

PacketBuffer packBuff = {0};

PacketBuilderState packetBuilderState = HEADD;
uint8_t packetBuilderIndex = 0;
char debugger[200];


uint8_t BuildRxPacket(packet* rxPacket, unsigned char reset) {
    if (reset) {
        packetBuilderState = HEADD;
    }
    int c;
    
    while ((c = GetChar()) == -1);
    unsigned char val = (unsigned char)c;
            switch (packetBuilderState) {
            case HEADD:
                if (val == HEAD) {
                    packetBuilderState = LEN;
                }
                break;
            case LEN:
            {
                if (val > 0 && val < MAXPAYLOADLENGTH) {
                    rxPacket->len = val;
                    packetBuilderState = ID;
                } else if (val == -1) {
                    packetBuilderState = LEN;
                } else {
                    packetBuilderState = HEADD;
                    Protocol_SendDebugMessage("Invalid Length Value received while building packet");
                }
                break;
            }
            case ID:
                   rxPacket->checkSum = val;
                   rxPacket->ID = val;
                   packetBuilderIndex = 0;
                   packetBuilderState = rxPacket->len == 1 ? TAILL : PAYLOAD;
                break;
            case PAYLOAD:
                rxPacket->payLoad[packetBuilderIndex] = val;
                rxPacket->checkSum = Protocol_CalcIterativeChecksum(val, rxPacket->checkSum);
                packetBuilderIndex++;
                if (packetBuilderIndex >= rxPacket->len - 1) {
                    packetBuilderState = TAILL;
                }
                break;
            case TAILL:
                if (val == TAIL) {
                    packetBuilderState = CHECKSUM;
                } else {
                    packetBuilderState = HEADD;
                    Protocol_SendDebugMessage("Invalid tail received while building packet");
                }
                break;
            case CHECKSUM:
                if (val == rxPacket->checkSum) {
                    packetBuilderState = END1;
                } else {
                    packetBuilderState = HEADD;
                    Protocol_SendDebugMessage("Invalid checksum received while building packet");
                }
                break;
            case END1:
                if (val == '\r') {
                    packetBuilderState = END2;
                } else {
                    packetBuilderState = HEADD;
                    Protocol_SendDebugMessage("No '\\r' received while building packet");
                }
                break;
            case END2:
                packetBuilderState = HEADD;
                if (val == '\n') {
                    return 1;
                } else {
                    Protocol_SendDebugMessage("No '\\n' received while building packet");
                }
                break;
            default:
                packetBuilderState = HEADD;
                break;
        }   
    return 0;
}

unsigned char Protocol_CalcIterativeChecksum(unsigned char charIn, unsigned char curChecksum) {
    curChecksum = (curChecksum >> 1) + (curChecksum  << 7);
    
    curChecksum += charIn;
    return curChecksum;
}


// Public Functions

uint8_t Protocol_Init(unsigned long baudrate) {
    Uart_Init(baudrate);
    LEDS_INIT();
    flushPacketBuffer();
    return 1;
}

uint8_t Protocol_QueuePacket(packet packet) {
    if (!packBuff.isFull) {
        packBuff.buffer[packBuff.tail] = packet;
        packBuff.tail = (packBuff.tail + 1) % PACKETBUFFERSIZE;
        if (packBuff.tail == packBuff.head) {
            packBuff.isFull = 1;
        }
        return 0;
    }
    return 1;
}

uint8_t Protocol_GetInPacket(packet** rtn) {
    if (packBuff.isFull || packBuff.head != packBuff.tail) {
        *rtn = &packBuff.buffer[packBuff.head];
        packBuff.head = (packBuff.head + 1) % PACKETBUFFERSIZE;
        if (packBuff.isFull) {
            packBuff.isFull = 0;
        }
        return 1;
    }
    return 0;
}

uint8_t Protocol_SendDebugMessage(char *Message) {
    uint8_t len = 0;
    while (len <= MAXPAYLOADLENGTH && Message[len] != '\0') {len++;}
    return Protocol_SendPacket(len, ID_DEBUG, Message);
}


uint8_t Protocol_SendPacket(uint8_t len, MessageIDS_t ID, void *Payload) {
    
    // Send HEAD
    if (PutChar(HEAD) == -1) {
        return 0;
    }

    // Send Length
    if (PutChar(len + 1) == -1) {
        return 0;
    }

    // Send ID
    if (PutChar(ID) == -1) {
        return 0;
    }

    uint8_t checksum = ID;    

    for (int i = 0; i < len; i++) { 
        unsigned char c = ((char*)Payload)[i];
        checksum = Protocol_CalcIterativeChecksum(c, checksum);
        PutChar(c);
    }

    // Send TAIL
    if (PutChar(TAIL) == -1) {
        return 0;
    }

    // Send Checksum
    if (PutChar(checksum) == -1) {
        return 0;
    }

    // Send End of Packet
    if (PutChar('\r') == -1) {
        return 0;
    }
    if (PutChar('\n') == -1) {
        return 0;
    }

    return SUCCESS;
}


uint8_t Protocol_ParsePacket(packet packet) {
    switch (packet.ID) {
        case ID_PING:
              {unsigned int pinger =
                        ((unsigned int)packet.payLoad[3]) |
                        ((unsigned int)packet.payLoad[2] << 8) |
                        ((unsigned int)packet.payLoad[1] << 16) |
                        ((unsigned int)packet.payLoad[0] << 24);
                pinger /= 2;
                pinger = Protocol_IntEndednessConversion(pinger);
                Protocol_SendPacket(3, ID_PONG, &pinger);
                break;
              }
        case ID_PONG:
            break;
        default:
            break;
    }
}



void flushPacketBuffer(void) {
    packBuff.head = 0;
    packBuff.tail = 0;      
    packBuff.isFull = 0;
}

uint8_t Protocol_ReadNextPacketID(void) {
    if (packBuff.isFull || packBuff.head != packBuff.tail) {
        return packBuff.buffer[packBuff.head].ID;
    }
    return 0;
}

unsigned short Protocol_ShortEndednessConversion(unsigned short inVariable) {
    return ((inVariable & 0xFF00) >> 8) | (((inVariable & 0x00FF) << 8));
}

unsigned int Protocol_IntEndednessConversion(unsigned int inVariable) {
    unsigned short b = inVariable & 0x0000FFFF;
    return (Protocol_ShortEndednessConversion(b) << 16) | Protocol_ShortEndednessConversion((inVariable & 0xFFFF0000) >> 16);
}

