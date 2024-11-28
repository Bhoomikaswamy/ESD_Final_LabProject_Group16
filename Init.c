#include "tm4c123gh6pm.h"
#include <stdint.h>
#include <stdbool.h>
#include "init.h"

// UART0 Initialization
void UART0_Init(void) {
    SYSCTL_RCGCUART_R |= 0x01;        // Enable clock for UART0
    SYSCTL_RCGCGPIO_R |= 0x01;        // Enable clock for GPIO Port A

    while ((SYSCTL_PRUART_R & 0x01) == 0); // Wait for UART0 ready

    UART0_CTL_R &= ~0x01;             // Disable UART0 during configuration
    UART0_IBRD_R = 104;               // Integer part of BRD (16MHz/9600)
    UART0_FBRD_R = 11;                // Fractional part of BRD
    UART0_LCRH_R = 0x70;              // 8-bit, no parity, 1-stop bit
    UART0_CC_R = 0x0;                 // Use system clock
    UART0_CTL_R = 0x301;              // Enable UART0, TXE, RXE

    GPIO_PORTA_AFSEL_R |= 0x03;       // Enable alternate function for PA0, PA1
    GPIO_PORTA_PCTL_R = (GPIO_PORTA_PCTL_R & 0xFFFFFF00) | 0x00000011;
    GPIO_PORTA_DEN_R |= 0x03;         // Enable digital function for PA0, PA1
}

// UART send string
void UART0_SendString(const char* str) {
    while (*str) {
        while ((UART0_FR_R & 0x20) != 0); // Wait until TX buffer is not full
        UART0_DR_R = *str++;
    }
}

// SPI Initialization
void SPI_Init(void) {
    SYSCTL_RCGCSSI_R |= SYSCTL_RCGCSSI_R0;  // Enable clock for SSI0
    SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R0; // Enable clock for GPIO Port A

    while ((SYSCTL_PRGPIO_R & SYSCTL_PRGPIO_R0) == 0); // Wait for stabilization

    GPIO_PORTA_AFSEL_R |= 0x3C;             // Enable alternate functions on PA2-PA5
    GPIO_PORTA_PCTL_R = (GPIO_PORTA_PCTL_R & 0xFF0000FF) | 0x00222200;
    GPIO_PORTA_DEN_R |= 0x3C;               // Enable digital on PA2-PA5
    GPIO_PORTA_DIR_R |= 0x2C;               // Set PA2, PA3, PA5 as outputs
    GPIO_PORTA_DIR_R &= ~0x10;              // Set PA4 as input (MISO)

    SSI0_CR1_R &= ~SSI_CR1_SSE;             // Disable SSI0 during configuration
    SSI0_CR1_R = 0x0;                       // Master mode
    SSI0_CC_R = 0x0;                        // System clock
    SSI0_CPSR_R = 0x10;                     // Clock prescale (16)
    SSI0_CR0_R = SSI_CR0_DSS_8 |            // 8-bit data size
                 SSI_CR0_FRF_MOTO |         // Freescale SPI frame format
                 SSI_CR0_SPH | SSI_CR0_SPO; // SPI mode
    SSI0_CR1_R |= SSI_CR1_SSE;              // Enable SSI0
}

// SPI transfer function
uint8_t SPI_transfer(uint8_t data) {
    SSI0_DR_R = data;                      // Send data
    while (SSI0_SR_R & SSI_SR_BSY);        // Wait for transmission complete
    return (uint8_t)(SSI0_DR_R & 0xFF);    // Return received data
}
