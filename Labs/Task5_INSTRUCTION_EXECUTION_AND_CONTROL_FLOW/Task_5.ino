#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>


void action0(void);
void action1(void);
void action2(void);
void action3(void);


extern "C" void modeA_entry(void) __attribute__((noreturn, used, externally_visible));
extern "C" void modeB_entry(void) __attribute__((noreturn, used, externally_visible));
extern "C" void modeC_entry(void) __attribute__((noreturn, used, externally_visible));


volatile uint16_t g_on_time = 500;
volatile uint16_t g_off_time = 500;


volatile uint8_t action_index = 0;


typedef void (*action_fn)(void);


action_fn action_table[4] = { action0, action1, action2, action3 };


static void delay_ms_var(uint16_t ms) {
    while (ms--) _delay_ms(1);
}


static void wait_release_and_debounce(void) {

    while (!(PINB & (1 << PB0)));

    _delay_ms(50);
}


void action0(void) {

    PORTB |= (1 << PB5);
    delay_ms_var(g_on_time);

    PORTB &= ~(1 << PB5);
    delay_ms_var(g_off_time);

    asm volatile("jmp mode_loop");
}


void action1(void) {

    PORTB &= ~(1 << PB5);
    delay_ms_var(g_off_time);

    PORTB |= (1 << PB5);
    delay_ms_var(g_on_time);

    asm volatile("rjmp mode_loop");
}


void action2(void) {

    PORTB |= (1 << PB5);
    _delay_ms(30);

    PORTB &= ~(1 << PB5);
    delay_ms_var(g_on_time + g_off_time);

    asm volatile("rjmp mode_loop");
}


void action3(void) {

    action_index = 0;

    asm volatile("rjmp mode_loop");
}


static void run_mode(void) {

    asm volatile("mode_loop:");

    while (1) {

        if (!(PINB & (1 << PB0))) {

            action_index++;
            action_index %= 4;

            wait_release_and_debounce();
        }

        asm volatile(
            "movw r30, %0\n\t"
            "ijmp\n\t"
            :
            : "r"(action_table[action_index])
        );
    }
}


extern "C" void modeA_entry(void) {

    g_on_time = 1000;
    g_off_time = 1000;

    run_mode();

    while (1) { }
}


extern "C" void modeB_entry(void) {

    while (1) {

        if (!(PINB & (1 << PB0))) {

            action_index++;
            action_index %= 4;

            wait_release_and_debounce();
        }

        PORTB |= (1 << PB5); _delay_ms(120);
        PORTB &= ~(1 << PB5); _delay_ms(120);

        PORTB |= (1 << PB5); _delay_ms(120);
        PORTB &= ~(1 << PB5); _delay_ms(600);

        asm volatile(
            "movw r30, %0\n\t"
            "ijmp\n\t"
            :
            : "r"(action_table[action_index])
        );
    }
}


extern "C" void modeC_entry(void) {

    g_on_time = 50;
    g_off_time = 50;

    run_mode();

    while (1) { }
}


int main(void) {

    DDRB |= (1 << PB5);
    DDRB &= ~(1 << PB0);

    PORTB |= (1 << PB0);


    asm volatile(
        "wait_btn:\n\t"
        "sbic %0, 0\n\t"
        "rjmp wait_btn\n\t"
        :
        : "I"(_SFR_IO_ADDR(PINB))
    );

    PORTB |= (1 << PB5);
    _delay_ms(200);
    PORTB &= ~(1 << PB5);


    uint8_t presses = 0;
    uint16_t timer = 0;

    while (timer < 2000) {

        if (!(PINB & (1 << PB0))) {

            presses++;
            wait_release_and_debounce();
        }

        _delay_ms(1);
        timer++;
    }

    if (presses >= 3) {

        asm volatile("jmp modeC_entry");

    } else if (presses == 2) {

        asm volatile("jmp modeB_entry");

    } else {

        asm volatile("jmp modeA_entry");
    }

    while (1) { }
}