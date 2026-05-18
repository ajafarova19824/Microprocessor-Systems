#include <avr/io.h>
#include <util/delay.h>

#define SLAVE_ADDR 0x08

void i2c_init() {
    DDRC &= ~((1 << DDC4) | (1 << DDC5)); // Set A4 and A5 as inputs 
    PORTC |= (1 << PORTC4) | (1 << PORTC5); // Enable internal pull-ups

    // Enable TWI in Power Reduction Register
    PRR &= ~(1 << PRTWI);
    
    // 2. Set Bit Rate (Standard Mode 100kHz at 16MHz)
    TWSR &= ~((1 << TWPS0) | (1 << TWPS1)); // Prescaler = 1 
    TWBR = 72; // SCL Freq = F_CPU / (16 + 2 * TWBR * Prescaler) 
    
    // 3. Enable TWI
    TWCR = (1 << TWEN); 
}

void i2c_start() {
    // Clear TWINT, send START condition, enable TWI
    TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN); 
    while (!(TWCR & (1 << TWINT))); // Wait for flag 
}

void i2c_stop() {
    // Clear TWINT, send STOP condition, enable TWI
    TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN);
    // Note: STOP does not reset TWINT, so we don't wait for it here.
}

void i2c_write(uint8_t data) {
    TWDR = data; // Load data 
    TWCR = (1 << TWINT) | (1 << TWEN); // Start transmission
    while (!(TWCR & (1 << TWINT))); // Wait for completion 
}

uint8_t i2c_read_nack() {
    TWCR = (1 << TWINT) | (1 << TWEN); // Start read, no ACK expected
    while (!(TWCR & (1 << TWINT))); 
    return TWDR;
}

void setup() {
    DDRD &= ~(1 << DDD2);   // Button pin 2 input
    PORTD |= (1 << PORTD2); // Pull-up
    DDRB |= (1 << DDB5);    // LED pin 13 output
    i2c_init();
}

void loop() {
    uint8_t masterButton = !(PIND & (1 << PIND2));

    // Transaction 1: Write Button State to Slave
    i2c_start();
    i2c_write((SLAVE_ADDR << 1) | 0); // Address + Write (0) 
    i2c_write(masterButton);
    i2c_stop();

    _delay_ms(10);

    // Transaction 2: Request Button State from Slave
    i2c_start();
    i2c_write((SLAVE_ADDR << 1) | 1); // Address + Read (1) 
    uint8_t slaveButton = i2c_read_nack();
    i2c_stop();

    if (slaveButton) PORTB |= (1 << PORTB5);
    else PORTB &= ~(1 << PORTB5);

    _delay_ms(50);
}