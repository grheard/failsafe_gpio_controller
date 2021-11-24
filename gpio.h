
#ifndef GPIO_H
#define GPIO_H

#include "definitions.h"


typedef enum GpioState {OFF,ON,LOCKED} gpio_state_t;


void gpio_init(void);

gpio_state_t gpio_status(control_t gpio);

void gpio_set(control_t gpio);
void gpio_clr(control_t gpio);


#endif
