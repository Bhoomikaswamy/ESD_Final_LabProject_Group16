#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "hw_memap.h"
#include "hw_types.h"
#include "gpio.h"
#include "pin_map.h"
#include "sysctl.h"
#include "uart.h"
#include "ssi.h"
#include "timer.h"
#include "rom_map.h"
#include "interrupt.h"
#include "uartstdio.h"
#include "rfid.h"

/*PIN Connections:
 * Used SSI2 (Module 2)
 *
 * To use another Module or Reset Pin, the variables, NRSTPD and chipSelectPin should be changed to the Pin Mask.
 * Also, in MFRC552_cpp, the definitions of CHIPSELECT_BASE, NRSTPD_BASE and SSI_BASE must be changed to the respective, Port Base used.
 * Finally, the respective chipSelectPin and NRSTPD, GPIO Base and Pin should be enabled in InitSSI function.
 *
 * Further versions should auto-change this values.
 *
 * SDA / CS / FSS ------------ PB5
 * SCK  / CLK     ------------ PB4
 * MOSI / TX      ------------ PB7
 * MISO /  RX     ------------ PB6
 * RST            ------------ PF0 *
 *
 */

#define redLED   0x00000002
#define blueLED  0x00000004
#define greenLED 0x00000008

void initLeds();
void dumpHex(unsigned char* buffer, int len);

uint8_t Version;
uint8_t AntennaGain;
uint8_t status;
uint32_t readTeste;
unsigned char str[MAX_LEN];
unsigned char cardID[CARD_LENGTH];
unsigned char RxData[MAX_LEN];
unsigned char TxData[MAX_LEN];


void InitConsole(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);

    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);

    UARTClockSourceSet(UART0_BASE, UART_CLOCK_PIOSC);

    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    UARTStdioConfig(0, 115200, 16000000);
}


void InitSSI(){
    uint32_t junkAuxVar;

    SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI2);

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB); //SDA
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF); //reset

    GPIOPinConfigure(GPIO_PB4_SSI2CLK);
    GPIOPinConfigure(GPIO_PB5_SSI2FSS);
    GPIOPinConfigure(GPIO_PB6_SSI2RX);
    GPIOPinConfigure(GPIO_PB7_SSI2TX);

    GPIOPinTypeSSI(GPIO_PORTB_BASE, GPIO_PIN_4 | GPIO_PIN_6 | GPIO_PIN_7);

    GPIOPinTypeGPIOOutput(CHIP_SELECT_BASE, CHIP_SELECT_PIN); //chipSelectPin
    GPIOPinTypeGPIOOutput(NRSTPD_BASE, NRSTPD_PIN); //NRSTPD

    SSIConfigSetExpClk(SSI2_BASE, SysCtlClockGet(), SSI_FRF_MOTO_MODE_0, SSI_MODE_MASTER, 4000000, 8);

    SSIEnable(SSI2_BASE);

    while(SSIDataGetNonBlocking(SSI2_BASE, &junkAuxVar));

    UARTprintf("SSI Enabled! SPI Mode!  \nData: 8bits.\n\n");

}


int main(void)
{
    SysCtlClockSet(SYSCTL_SYSDIV_2_5 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ); //80MHz

    InitConsole();
    initLeds();

    InitSSI();

    unsigned char key[6], i;
    for (i = 0; i < 6; i++)
        key[i] = 0xFF;

    GPIOPinWrite(CHIP_SELECT_BASE, CHIP_SELECT_PIN, 0);
    GPIOPinWrite(NRSTPD_BASE, NRSTPD_PIN, NRSTPD_PIN);

    MFRC552_Init();

    Version = MFRC552_ReadReg(VersionReg);
    AntennaGain = MFRC552_ReadReg(PICC_REQIDL) & (0x70);

    UARTprintf("Version: 0x%x \n", Version);
    UARTprintf("Antenna Gain: 0x%x \n\n", AntennaGain);

    while(1)
    {
        status = MFRC552_Request(PICC_REQIDL, str);
        if(status == MI_OK)
        {
            UARTprintf("Cartao 0x%0X Detectado! \n", (str[0] << 8) + str[1]); //Card Detected
            status = MFRC552_Anticoll(str);
            if(status == MI_OK)
            {
                memcpy(cardID, str, CARD_LENGTH);
                UARTprintf("ID: ");
                dumpHex((unsigned char*)cardID, CARD_LENGTH);
                if(cardID[0] == 0x36 || cardID[1] == 0xE2 || cardID[2] == 0x45 ||
                        cardID[3] == 0x43 || cardID[0] == 0xD2)
                    GPIOPinWrite(GPIO_PORTF_BASE, greenLED, greenLED);
                else
                    GPIOPinWrite(GPIO_PORTF_BASE, blueLED, blueLED);

                strcpy(TxData, "1234567890");

                status = MFRC552_Auth(0x60, 1, key, cardID);
                if(status == MI_ERR)
                {
                    UARTprintf("FALHA NA AUTENTICACAO: 0x%X\n", MFRC552_ReadReg(ErrorReg));
                    GPIOPinWrite(GPIO_PORTF_BASE, redLED, redLED);
                }
                status = MFRC552_WriteBlock(1, TxData);
                if(status == MI_ERR)
                {
                    UARTprintf("FALHA NA ESCRITA: 0x%X\n", MFRC552_ReadReg(ErrorReg));
                    GPIOPinWrite(GPIO_PORTF_BASE, redLED, redLED);
                }

                SysCtlDelay(SysCtlClockGet()/6); //Delay

                status = MFRC552_Auth(0x60, 1, key, cardID);
                if(status == MI_ERR)
                {
                    UARTprintf("FALHA NA AUTENTICACAO: 0x%X\n", MFRC552_ReadReg(ErrorReg));
                    GPIOPinWrite(GPIO_PORTF_BASE, redLED, redLED);
                }
                status = MFRC552_ReadBlock(1, RxData);
                if(status == MI_ERR)
                {
                    UARTprintf("FALHA NA LEITURA: 0x%X\n", MFRC552_ReadReg(ErrorReg));
                    GPIOPinWrite(GPIO_PORTF_BASE, redLED, redLED);
                }
                else if(status == MI_OK)
                    dumpHex((unsigned char*)RxData, 10);
            }

            else if(status == MI_ERR)
            {
                UARTprintf("Colisao detectada!!!\n", MFRC552_ReadReg(ErrorReg));
                GPIOPinWrite(GPIO_PORTF_BASE, redLED, redLED);
            }

            SysCtlDelay(SysCtlClockGet()/3); //Delay
            GPIOPinWrite(GPIO_PORTF_BASE, 0x0E, 0);
            UARTprintf("\n\nAproxime o cartao/tag\n");
        }
    }
}

void initLeds(){
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3);
}

void dumpHex(unsigned char* buffer, int len){
    int i;

    for(i=0; i < len; i++)
        UARTprintf("0x%X ", buffer[i]);
    UARTprintf("\n\n");
}
