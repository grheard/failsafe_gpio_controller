
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>

#include "gpio.h"


uint8_t watchdog_counts[CONTROL_SIZE] = {0,0,0};
uint16_t lockout_counts[CONTROL_SIZE] = {FAN_LOCKOUT_SECONDS,LOCKOUT_SECONDS,LOCKOUT_SECONDS};


ISR(TIMER1_OVF1_vect)
{
    uint8_t idx;
    for (idx = 0; idx < CONTROL_SIZE; idx++)
    {
        if (lockout_counts[idx] != 0)
        {
            lockout_counts[idx]--;
        }
        else if (gpio_status(idx) == ON)
        {
            if (watchdog_counts[idx] != 0)
            {
                watchdog_counts[idx]--;
            }
            if (watchdog_counts[idx] == 0)
            {
                gpio_clr(idx);
            }
        }
    }

    wdt_reset();
}


void gpio_init(void)
{
    PORTA &= ~(_BV(PA2) | _BV(PA1) | _BV(PA0));
    DDRA |= _BV(PA2) | _BV(PA1) | _BV(PA0);

    TCCR1A = 0x00;
    TCCR1B = 0x0f; // CK/16384 1.04s per OVF at 4MHz
    TIMSK |= _BV(TOIE1);

    wdt_enable(WDTO_2S); // Watchdog enabled, 2s.
}


gpio_state_t gpio_status(control_t gpio)
{
    gpio_state_t state = OFF;

    if (lockout_counts[gpio] != 0)
    {
        state = LOCKED;
    }
    else
    {
        switch (gpio)
        {
            case FAN:
                state = (bit_is_set(PORTA,PA0)) ? ON : OFF;
                break;
            case HEAT:
                state = (bit_is_set(PORTA,PA1)) ? ON : OFF;
                break;
            case COOL:
                state = (bit_is_set(PORTA,PA2)) ? ON : OFF;
                break;
        }
    }
    return state;
}


void gpio_set(control_t gpio)
{
    if (gpio_status(gpio) != LOCKED)
    {
        switch (gpio)
        {
            case FAN:
                PORTA |= _BV(PA0);
                break;
            case HEAT:
                // Don't allow the heat relay if cool is already on.
                if (gpio_status(COOL) == ON)
                {
                    return;
                }
                PORTA |= _BV(PA1);
                break;
            case COOL:
                // Don't allow the cool relay if heat is already on.
                if (gpio_status(HEAT) == ON)
                {
                    return;
                }
                PORTA |= _BV(PA2);
                break;
        }

        watchdog_counts[gpio] = TIMEOUT_SECONDS;
    }
}


void gpio_clr(control_t gpio)
{
    if (gpio_status(gpio) == ON)
    {
        watchdog_counts[gpio] = 0;
        lockout_counts[gpio] = LOCKOUT_SECONDS;
        switch (gpio)
        {
            case FAN:
                lockout_counts[gpio] = FAN_LOCKOUT_SECONDS;
                PORTA &= ~_BV(PA0);
                break;
            case HEAT:
                PORTA &= ~_BV(PA1);
                break;
            case COOL:
                PORTA &= ~_BV(PA2);
                break;
        }
    }
}
