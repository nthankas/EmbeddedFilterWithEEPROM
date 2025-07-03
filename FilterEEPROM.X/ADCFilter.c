#include <sys/attribs.h>
#include <stdio.h>
#include "ADCFilter.h"
#include "BOARD.h"
#include "FreeRunningTimer.h"
#include "MessageIDs.h"
#include "Protocol2.h" 

#define ADCTest


unsigned int ms;
unsigned int last;
short currChan = 0;
int ADCReadings[2];

static struct {
    signed short tail;
    signed short head;
    signed short data[4][FILTERLENGTH];
} ADCBuffer;

static struct {
    signed short tail;
    signed short head;
    signed short data[4][FILTERLENGTH];
} FilterBuffer;

int ADCFilter_Init(void) {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < FILTERLENGTH; j++) {
            ADCBuffer.data[i][j] = 0;
            FilterBuffer.data[i][j] = 0;
            FilterBuffer.head = 0;
        }
    }
    
    AD1CON1 = 0;
    AD1CON2 = 0;
    AD1CON3 = 0;
    
    TRISBbits.TRISB2 = 1;
    TRISBbits.TRISB4 = 1;
    TRISBbits.TRISB8 = 1;
    TRISBbits.TRISB10 = 1;
    
    AD1PCFGbits.PCFG2 = 0;
    AD1PCFGbits.PCFG4 = 0;
    AD1PCFGbits.PCFG8 = 0;
    AD1PCFGbits.PCFG10 = 0;
    
    AD1CSSLbits.CSSL2 = 1;
    AD1CSSLbits.CSSL4 = 1;
    AD1CSSLbits.CSSL8 = 1;
    AD1CSSLbits.CSSL10 = 1;
    
    AD1CON1bits.ASAM = 1;
    AD1CON1bits.SSRC = 0b111;
    AD1CON1bits.FORM = 0b000;
    
    AD1CON2bits.VCFG = 0b000;
    AD1CON2bits.CSCNA = 1;
    AD1CON2bits.SMPI = 0b00011;
    
    AD1CON2bits.BUFM = 0;
    
    AD1CON3bits.ADRC = 0;
    AD1CON3bits.ADCS = 0b10101101;
    AD1CON3bits.SAMC = 0b10000;
    
    IPC6bits.AD1IP = 7;
    IPC6bits.AD1IS = 3;
    IFS1bits.AD1IF = 0;
    IEC1bits.AD1IE = 1;
     
    AD1CON1bits.ON = 1;
    return SUCCESS;
    
}

void __ISR(_ADC_VECTOR) ADCIntHandler(void) {
    IFS1bits.AD1IF = 0;
   
    ADCBuffer.data[0][ADCBuffer.head] = ADC1BUF0; 
    ADCBuffer.data[1][ADCBuffer.head] = ADC1BUF1;    
    ADCBuffer.data[2][ADCBuffer.head] = ADC1BUF2;    
    ADCBuffer.data[3][ADCBuffer.head] = ADC1BUF3;
    ADCBuffer.head = (ADCBuffer.head + 1) % FILTERLENGTH;


}

short ADCFilter_RawReading(short pin) {
    return ADCBuffer.data[pin][ADCBuffer.head];
}

int ADCFilter_SetWeights(short pin, short weights[]) {
    for (int i = 0; i < FILTERLENGTH; i++) {
        FilterBuffer.data[pin][i] = weights[i];
    }
    return SUCCESS;
}

short ADCFilter_FilteredReading(short pin) {
    return ADCFilter_ApplyFilter(FilterBuffer.data[pin], ADCBuffer.data[pin], ADCBuffer.head);
}

short ADCFilter_ApplyFilter(short filter[], short values[], short startIndex) {
    int sum = 0;
    for (int i = 0; i < FILTERLENGTH; i++) {
        sum += filter[i] * values[startIndex];
        startIndex--;
        if (startIndex < 0) {
            startIndex = 31;
        }
    }
    return (short)sum;
} 

#ifdef ADCTest

int main(void) {
    BOARD_Init();
    Protocol_Init(115200);
    ADCFilter_Init();
    FreeRunningTimer_Init();
    char debug[128];
    sprintf(debug, "ADCFilter Test Harness");
    Protocol_SendDebugMessage(debug);
    
    ms = FreeRunningTimer_GetMilliSeconds();
    last = ms;
    
            packet testpack = {0};

    
    while (1) {
        
        if(BuildRxPacket(&testpack, 0)){
            if (testpack.ID == ID_ADC_FILTER_VALUES) {
                if (testpack.payLoad[1] == 0xCC) {
                char high[64];
                for (int i = 0; i < 64; i++) {
                    high[i] = testpack.payLoad[i];
                }
                short highP[32];
                for (int j = 0; j < 32; j++) {
                    highP[j] = Protocol_ShortEndednessConversion((short)((high[2*j] << 8) | high[2*j + 1]));
                }
                
                ADCFilter_SetWeights(currChan, highP);
                Protocol_SendPacket(sizeof(highP) + 1, ID_ADC_FILTER_VALUES_RESP, (unsigned char*)highP);
                
                char debugger[200];
                sprintf(debugger, "result %u", testpack.payLoad[1]);
                Protocol_SendDebugMessage(debugger);
                
            
            }
            else if (testpack.payLoad[1] == 0x49) {
                char band[64];
                for (int i = 0; i < 64; i++) {
                    band[i] = testpack.payLoad[i];
                }
                short band2[32];
                for (int j = 0; j < 32; j++) {
                    band2[j] = Protocol_ShortEndednessConversion((short)((band[2*j] << 8) | band[2*j + 1]));
                }
                
                ADCFilter_SetWeights(currChan, band2);
                Protocol_SendPacket(sizeof(band2) + 1, ID_ADC_FILTER_VALUES_RESP, (unsigned char*)band2);
                
                char debugger[200];
                sprintf(debugger, "result %u", testpack.payLoad[1]);
                Protocol_SendDebugMessage(debugger);
            }
            else {
                char low[64];
                for (int i = 0; i < 64; i++) {
                    low[i] = testpack.payLoad[i];
                }
                short low2[32];
                for (int j = 0; j < 32; j++) {
                    low2[j] = Protocol_ShortEndednessConversion((short)((low[2*j] << 8) | low[2*j + 1]));
                }
                
                ADCFilter_SetWeights(currChan, low2);
                Protocol_SendPacket(sizeof(low2) + 1, ID_ADC_FILTER_VALUES_RESP, (unsigned char*)low2);
                
                char debugger[200];
                sprintf(debugger, "result %u", testpack.payLoad[2]);
                Protocol_SendDebugMessage(debugger);
            }
            }
            else if (testpack.ID == ID_ADC_SELECT_CHANNEL) {
                currChan = testpack.payLoad[0];
                Protocol_SendPacket(2, ID_ADC_SELECT_CHANNEL_RESP, testpack.payLoad);
            }
            else if (Protocol_QueuePacket(testpack)){
                Protocol_SendDebugMessage("Packet Queue full");
            }
        }
      ms = FreeRunningTimer_GetMilliSeconds();
        if ((ms - last) >= 1000) {
            ADCReadings[0] = Protocol_ShortEndednessConversion(ADCFilter_RawReading(currChan));
            ADCReadings[1] = Protocol_ShortEndednessConversion(ADCFilter_FilteredReading(currChan));
            unsigned char rdn[2];
            rdn[3] = (ADCReadings[0] >> 8) & 0x00FF;
            rdn[2] = (ADCReadings[0]) & 0x00FF;
            rdn[1] = (ADCReadings[1] >> 8) & 0x00FF;
            rdn[0] = (ADCReadings[1]) & 0x00FF;
            Protocol_SendPacket(32, ID_ADC_READING, rdn);
            last = ms;
            
      }
        
  
      packet* inpack;
        if (Protocol_GetInPacket(&inpack)){
            Protocol_ParsePacket(*inpack);
        }
      
    }
}

#endif