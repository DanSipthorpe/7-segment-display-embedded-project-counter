#define _XTAL_FREQ 4000000 // 4MHz crystal
#include <xc.h>

// Define control pins
#define DATA_PIN RB2
#define CLOCK_PIN RB1
#define LATCH_PIN RB3
volatile unsigned char paused = 0; // Pause flag

// Lookup table for numbers 0?5 (active-low)
const unsigned char display_data[] = {
    0b11000000, // 0
    0b11111001, // 1
    0b10100100, // 2
    0b10110000, // 3
    0b10011001, // 4
    0b10010010  // 5
};

void shiftOut(unsigned char data) {
    for (int i = 7; i >= 0; i--) {
        DATA_PIN = (data >> i) & 1;
        CLOCK_PIN = 1;
        __delay_us(1);
        CLOCK_PIN = 0;
    }

    // Latch data to output
    LATCH_PIN = 1;
    __delay_us(1);
    LATCH_PIN = 0;
}


void __interrupt() isr(void) {
    if (INTF) {
        __delay_ms(50); // Wait for bouncing to settle

        if (PORTBbits.RB0 == 0) { // Check if button is still pressed
            paused = !paused;     // Toggle pause
        }

        INTF = 0; // Clear interrupt flag
    }
}


void main(void) {
    TRISB = 0x01;   // RB0 as input, rest of PORTB as output
    OPTION_REGbits.INTEDG = 0; // Interrupt on falling edge (button press)
    INTCONbits.INTE = 1;       // Enable RB0 interrupt
    INTCONbits.GIE = 1;        // Enable global interrupts

    PORTB = 0x00;

    char i = 0;
    char direction = 1;

    while (1) {
        if (!paused) {
            shiftOut(display_data[i]);
            __delay_ms(500);

            if (direction) {
                i++;
                if (i == 5) direction = 0;
            } else {
                i--;
                if (i == 0) direction = 1;
            }
        }
    }
}

