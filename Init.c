/*
 * Init.c
 *
 *  Created on: 27-Nov-2024
 *      Author: Bhoomika Swamy
 */

#include <stdint.h>
#include <stdbool.h>
#include "tm4c123gh6pm.h"
#include "definitions.h"

void GPIO_PortA_Initialisation(void){

// SPI(0) pin connections: PA2:-SCK(0) PA3:-CS(0) PA4:-MISO(0) PA5:-MOSI(0)

    SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R0;// Enable clock for Port A
    while ((SYSCTL_PRGPIO_R & SYSCTL_PRGPIO_R0) == 0);          // Allow time for clock to stabilize
    GPIO_PORTA_LOCK_R = GPIO_LOCK_KEY;      // Unlock Port A
    GPIO_PORTA_CR_R |= 0x3c;                 // Commit changes,1-enable (PA7-PA0 = 00111100)
    GPIO_PORTA_DEN_R |= 0x3c;                // Digital function enable, 1-enable (PA7-PA0 = 00111100)
    GPIO_PORTA_DIR_R |= 0x2c;                // Set output/input, 1-output (PA7-PA0 = 00101100)
    GPIO_PORTA_PUR_R |= 0x0c;                // Enable pull-up resistor, 1-enable (PA7-PA0 = 00001100)
    GPIO_PORTA_DATA_R = 0x00;               // Reset the data register (PA7-PA0 = 00000000)

}

void GPIO_PortB_Initialisation(void){

// SPI(2) pin connections: PB4:-SCK(2) PB5:-CS(2) PB6:-MISO(2) PB7:-MOSI(2)

    SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R1;// Enable clock for Port B
    while ((SYSCTL_PRGPIO_R & SYSCTL_PRGPIO_R1) == 0);          // Allow time for clock to stabilize
    GPIO_PORTB_LOCK_R = GPIO_LOCK_KEY;      // Unlock Port B
    GPIO_PORTB_CR_R |= 0xf0;                 // Commit changes,1-enable (PB7-PB0 = 11110000)
    GPIO_PORTB_DEN_R |= 0xf0;                // Digital function enable, 1-enable (PB7-PB0 =11110000)
    GPIO_PORTB_DIR_R |= 0xb0;                // Set output/input, 1-output (PB7-PB0 = 10110000)
    GPIO_PORTB_PUR_R |= 0x30;                // Enable pull-up resistor, 1-enable (PB7-PB0 = 00110000)
    GPIO_PORTB_DATA_R = 0x00;               // Reset the data register (PB7-PB0 = 00000000)

}

void GPIO_PortF_Initialisation(void){

// PORTF, PF7-PF0, PF4-SW1, PF3-green, PF2-blue, PF1-red, PF0-SW2

    SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R5;// Enable clock for Port F
    while ((SYSCTL_PRGPIO_R & SYSCTL_PRGPIO_R5) == 0);          // Allow time for clock to stabilize
    GPIO_PORTF_LOCK_R = GPIO_LOCK_KEY;      // Unlock Port F
    GPIO_PORTF_CR_R |= 0x1f;                 // Commit changes,1-enable (PF7-PF0 = 00011111)
    GPIO_PORTF_DEN_R |= 0x1f;                // Digital function enable, 1-enable (PF7-PF0 = 00011111)
    GPIO_PORTF_DIR_R |= 0x0e;                // Set output/input, 1-output (PF7-PF0 = 00001110)
    GPIO_PORTF_PUR_R |= 0x11;                // Enable pull-up resistor, 1-enable (PF7-PF0 = 00010001)
    GPIO_PORTF_DATA_R = 0x00;               // Reset the data register (PF7-PF0 = 00000000)

}

// SPI(0) pin connections: PA2:-SCK(0) PA3:-CS(0) PA4:-MISO(0) PA5:-MOSI(0)

void SPI_0_Initialisation(void){

    SYSCTL_RCGCSSI_R |= SYSCTL_RCGCSSI_R0;  // Enable clock for SPI(0) module
    while ((SYSCTL_PRSSI_R & SYSCTL_PRSSI_R0) == 0)          // Allow time for clock to stabilize
    GPIO_PORTA_AFSEL_R |= 0x3c;              // Alternate function,1-enable (PA7-PA0 = 00111100)
    GPIO_PORTA_PCTL_R |= (GPIO_PORTA_PCTL_R & 0xff0000ff) | 0x00222200; // Set PA2, PA3, PA4 and PA5 to SPI (0)
    SSI0_CR1_R = 0x00;                      // SPI control, disable fully
    SSI0_CC_R = 0x00;                       // Clock configuration - system clock
    SSI0_CPSR_R = 0x02;                     // Clock divisor pre-scalar, 2
    SSI2_CR0_R = 0x707;                     // 16 bit-all zero(reserved), 8 bit-0x07(serial clock rate 1Mbps), SPH-0, SPO-0, 2 bit-0(mode freescale), 4 bit-7(8bit data)
    //SSI0_CR0_R = 0x07c7;                    // SPI settings, 16 bit-all zero(reserved), 8 bit-0x07(serial clock rate 1Mbps), SPH-1, SPO-1, 2 bit-0(mode freescale), 4 bit-7(8bit data)
    SSI0_CR1_R = 0x02;                      // SPI control, 00010 - endTxInterrupt, reserved, MasterOperation, SPIEnable, LoopbackDisable

}

// SPI(2) pin connections: PB4:-SCK(2) PB5:-CS(2) PB6:-MISO(2) PB7:-MOSI(2)

void SPI_2_Initialisation(void){

    SYSCTL_RCGCSSI_R |= SYSCTL_RCGCSSI_R2;  // Enable clock for SPI(2) module
    while ((SYSCTL_PRSSI_R & SYSCTL_PRSSI_R2) == 0)          // Allow time for clock to stabilize
    GPIO_PORTB_AFSEL_R |= 0xf0;              // Alternate function,1-enable (PB7-PB0 = 11110000)
    GPIO_PORTB_PCTL_R |= (GPIO_PORTB_PCTL_R & 0x0000ffff) | 0x22220000; // Set PB4, PB5, PB6 and PB7 to SPI (2)
    SSI2_CR1_R = 0x00;                      // SPI control, disable fully
    SSI2_CC_R = 0x00;                       // Clock configuration - system clock
    SSI2_CPSR_R = 0x02;                     // Clock divisor pre-scalar, 2
    SSI2_CR0_R = 0x1307;
    //SSI2_CR0_R = 0x707;                     // working; 16 bit-all zero(reserved), 8 bit-0x07(serial clock rate 1Mbps), SPH-0, SPO-0, 2 bit-0(mode freescale), 4 bit-7(8bit data)
    //SSI2_CR0_R = 0x747;                   // 16 bit-all zero(reserved), 8 bit-0x07(serial clock rate 1Mbps), SPH-0, SPO-1, 2 bit-0(mode freescale), 4 bit-7(8bit data)
    SSI2_CR1_R = 0x02;                      // SPI control, 00010 - endTxInterrupt, reserved, MasterOperation, SPIEnable, LoopbackDisable

}

// Configure PA0 (RX) and PA1 (TX)

void UART0_Initialisation(void) {

    SYSCTL_RCGCUART_R |= SYSCTL_RCGC1_UART0;     // Enable UART0 clock
    while ((SYSCTL_PRUART_R & SYSCTL_PRUART_R0) == 0)          // Allow time for clock to stabilize

    GPIO_PORTA_LOCK_R = GPIO_LOCK_KEY;      // Unlock Port A
    GPIO_PORTA_CR_R |= 0x03;                 // Commit changes,1-enable (PA7-PA0 = 00000011)
    GPIO_PORTA_DEN_R |= 0x03;                // Digital enable PA0 and PA1
    GPIO_PORTA_AFSEL_R |= 0x03;              // Enable alternate function for PA0 and PA1
    GPIO_PORTA_AMSEL_R |= 0x00;              // Turnoff analog function
    GPIO_PORTA_PCTL_R |= 0xFFFFFF11;         // Set PCTL for PA0 and PA1

    // Configure UART0 for 9600 baud rate, 8 data bits, odd parity
    UART0_CTL_R = 0x00;                     // Disable UART before configuration
    UART0_IBRD_R = 104;                     // Integer part of BRD = 16MHz / (16 * 9600) = 104
    UART0_FBRD_R = 11;                      // Fractional part of BRD = 0.16 * 64 + 0.5 = 11
    UART0_CC_R = 0x00;                      // UART clock set to system clock
    UART0_LCRH_R = 0x60;                    // 8 bits, odd parity
    //UART0_LCRH_R = 0x72;                    // 8 bits, odd parity
    UART0_CTL_R = 0x301;                    // Enable UART
}
