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

#define redLED   0x00000002
#define blueLED  0x00000004
#define greenLED 0x00000008
#define orangeLED 0x0000000A // Combination of red and green

void initLeds();
void dumpHex(unsigned char* buffer, int len);

uint8_t Version;
uint8_t AntennaGain;
uint8_t status;
uint32_t readTeste;

unsigned char str[MAX_LEN];
unsigned char cardID[CARD_LENGTH];
unsigned char storedCard[CARD_LENGTH] = {0}; // Stores the current locking card
bool isLocked = false; // Lock state (true = locked, false = unlocked)

// Initialize Console
void InitConsole(void) {
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    UARTClockSourceSet(UART0_BASE, UART_CLOCK_PIOSC);
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
    UARTStdioConfig(0, 115200, 16000000);
}

// Initialize SSI
void InitSSI() {
    uint32_t junkAuxVar;
    SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI2);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB); // SDA
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF); // Reset
    GPIOPinConfigure(GPIO_PB4_SSI2CLK);
    GPIOPinConfigure(GPIO_PB5_SSI2FSS);
    GPIOPinConfigure(GPIO_PB6_SSI2RX);
    GPIOPinConfigure(GPIO_PB7_SSI2TX);
    GPIOPinTypeSSI(GPIO_PORTB_BASE, GPIO_PIN_4 | GPIO_PIN_6 | GPIO_PIN_7);
    GPIOPinTypeGPIOOutput(CHIP_SELECT_BASE, CHIP_SELECT_PIN); // chipSelectPin
    GPIOPinTypeGPIOOutput(NRSTPD_BASE, NRSTPD_PIN); // NRSTPD
    SSIConfigSetExpClk(SSI2_BASE, SysCtlClockGet(), SSI_FRF_MOTO_MODE_0, SSI_MODE_MASTER, 4000000, 8);
    SSIEnable(SSI2_BASE);
    while (SSIDataGetNonBlocking(SSI2_BASE, &junkAuxVar));
    UARTprintf("SSI Enabled! SPI Mode!  \nData: 8bits.\n\n");
}

// Main Function
int main(void) {
    SysCtlClockSet(SYSCTL_SYSDIV_2_5 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ); // 80MHz
    InitConsole();
    initLeds();
    InitSSI();

    unsigned char key[6];
    int i;
    for (i = 0; i < 6; i++)
        key[i] = 0xFF;

    GPIOPinWrite(CHIP_SELECT_BASE, CHIP_SELECT_PIN, 0);
    GPIOPinWrite(NRSTPD_BASE, NRSTPD_PIN, NRSTPD_PIN);

    MFRC552_Init();
    Version = MFRC552_ReadReg(VersionReg);
    AntennaGain = MFRC552_ReadReg(PICC_REQIDL) & (0x70);
    UARTprintf("Version: 0x%x \n", Version);
    UARTprintf("Antenna Gain: 0x%x \n\n", AntennaGain);

    while (1) {
        status = MFRC552_Request(PICC_REQIDL, str);
        if (status == MI_OK) {
            UARTprintf("Card Detected: 0x%0X \n", (str[0] << 8) + str[1]);
            status = MFRC552_Anticoll(str);
            if (status == MI_OK) {
                memcpy(cardID, str, CARD_LENGTH);
                UARTprintf("ID: ");
                dumpHex((unsigned char*)cardID, CARD_LENGTH);

                if (isLocked) {
                    if (memcmp(storedCard, cardID, CARD_LENGTH) == 0) {
                        isLocked = false;
                        memset(storedCard, 0, CARD_LENGTH); // Clear stored card
                        UARTprintf("System Unlocked!\n");
                        GPIOPinWrite(GPIO_PORTF_BASE, greenLED, greenLED); // Green LED
                    } else {
                        UARTprintf("Unauthorized Card!\n");
                        GPIOPinWrite(GPIO_PORTF_BASE, redLED, redLED); // Red LED
                    }
                } else {
                    memcpy(storedCard, cardID, CARD_LENGTH); // Store new card
                    isLocked = true;
                    UARTprintf("System Locked!\n");
                    GPIOPinWrite(GPIO_PORTF_BASE, orangeLED, orangeLED); // Orange LED
                }

                // Reset LEDs after a delay
                SysCtlDelay(SysCtlClockGet() / 6); // Delay
                GPIOPinWrite(GPIO_PORTF_BASE, 0x0E, 0); // Turn off all LEDs
            }
        }
    }
}

// Initialize LEDs
void initLeds() {
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3);
}

// Dump Hexadecimal
void dumpHex(unsigned char* buffer, int len) {
    int i;
    for (i = 0; i < len; i++)
        UARTprintf("0x%X ", buffer[i]);
    UARTprintf("\n\n");
}
