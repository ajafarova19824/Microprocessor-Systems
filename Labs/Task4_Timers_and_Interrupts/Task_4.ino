#include <avr/io.h>
#include <avr/interrupt.h>

volatile uint8_t countdown = 0;

int main(void) {
    DDRD &= ~(1 << DDD2);
    PORTD |= (1 << PORTD2); // internal pull-up resistor
    DDRB |= (1 << DDB5);

    EICRA |= (1 << ISC01);
    EIMSK |= (1 << INT0);

    TCCR1B |= (1 << WGM12);
    OCR1A = 249;
    TIMSK1 |= (1 << OCIE1A);
    TCCR1B |= (1 << CS11) | (1 << CS10); // prescaler

    sei();

    while (1) {

    }
}

ISR(INT0_vect) {
    if (countdown == 0) {
        TCNT1 = 0;
        PORTB |= (1 << PORTB5); 
        countdown = 10;         
    }
}

ISR(TIMER1_COMPA_vect) {
    if (countdown > 0) {
        countdown--;
        if (countdown == 0) {
            PORTB &= ~(1 << PORTB5);
        }
    }
}