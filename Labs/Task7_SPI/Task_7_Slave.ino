#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sfr_defs.h>

// Minimal UART for Serial Monitor output (9600 baud, 16 MHz)
void UART_Init(void) {
    UBRR0H = 0;
    UBRR0L = 103;              // 16000000/(16*9600) - 1 = 103
    UCSR0B = (1 << TXEN0);
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);  // 8-bit, 1 stop, no parity
}

void UART_Transmit(uint8_t data) {
    while (!(UCSR0A & (1 << UDRE0)));
    UDR0 = data;
}

void UART_PrintDecimal(uint8_t val) {
    if (val >= 100) UART_Transmit('0' + val/100);
    if (val >= 10)  UART_Transmit('0' + (val/10)%10);
    UART_Transmit('0' + val%10);
    UART_Transmit('\r');
    UART_Transmit('\n');
}

void SPI_SlaveInit(void) {
    DDRB |= (1 << PB4);        // MISO = output; MOSI, SCK, SS = inputs
    SPCR = (1 << SPE) | (1 << SPIE);  // SPI enable + interrupt enable, slave mode
    sei();
}

ISR(SPI_STC_vect) {            // fires when byte fully received
    uint8_t received = SPDR;   // reading SPDR clears SPIF
    UART_PrintDecimal(received);
}

int main(void) {
    UART_Init();
    SPI_SlaveInit();
    while (1);                 // everything handled in ISR
}