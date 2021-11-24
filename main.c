
#include <avr/interrupt.h>

#include "gpio.h"
#include "led.h"
#include "i2c.h"


int main(void)
{
    gpio_init();
    led_init();
    i2c_init();

    led_flash(250);
    sei();
    for(;;) {;}
}