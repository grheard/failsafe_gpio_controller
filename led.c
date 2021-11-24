
#include <avr/io.h>
#include <avr/interrupt.h>


uint16_t counts = 0;


void led_init(void)
{
    counts = 0;

    DDRB |= _BV(PB1);
    PORTB |= _BV(PB1);

    TCCR0 = _BV(CS01); // CK / 8 = 500us at 4MHz
    TIMSK |= _BV(TOIE0);
}


void led_flash(uint8_t ms)
{
    if (ms == 0)
    {
        counts = 0;
        PORTB |= _BV(PB1);
    }
    else
    {
        PORTB &= ~_BV(PB1);
        counts = (uint16_t)ms << 1;
    }
}


ISR(TIMER0_OVF0_vect)
{
    if (counts != 0)
    {
        counts--;
        if (counts == 0)
        {
            PORTB |= _BV(PB1);
        }
    }
}