
#include <avr/interrupt.h>
#include <avr/io.h>

#include "definitions.h"
#include "gpio.h"
#include "led.h"


#define USI_PORT    PORTB
#define USI_DDR     DDRB
#define USI_PIN     PINB
#define SDA         PB0
#define SCL         PB2


enum GpioControl
{
    GPIO_NO_CHANGE,
    GPIO_OFF,
    GPIO_ON
};

typedef enum I2CState
{
    START
    ,ADDRESS
    ,SEND
    ,SEND_WAIT
    ,SEND_ACK
    ,RECV
    ,RECV_ACK
} i2c_state_t;

i2c_state_t state = START;


#define DATA_SIZE 4

uint8_t data_index = 0;
uint8_t data[DATA_SIZE];


#define SET_USI_TO_SEND_ACK() \
{ \
    USIDR = 0; \
    USI_DDR |= _BV(SDA); \
    USISR = _BV(USIOIF) | _BV(USIPF) | _BV(USIDC) | 0x0E; \
}


#define SET_USI_TO_SEND_NAK() \
{ \
    USIDR = 0xff; \
    USI_DDR |= _BV(SDA); \
    USISR = _BV(USIOIF) | _BV(USIPF) | _BV(USIDC) | 0x0E; \
}


#define SET_USI_TO_READ_ACK() \
{ \
    USI_DDR &= ~_BV(SDA); \
    USIDR = 0; \
    USISR = _BV(USIOIF) | _BV(USIPF) | _BV(USIDC) | 0x0E; \
}


#define SET_USI_TO_START_CONDITION_MODE() \
{ \
    USICR = _BV(USISIE) | _BV(USIWM1) | _BV(USICS1); \
    USISR = _BV(USIOIF) | _BV(USIPF) | _BV(USIDC); \
}


#define SET_USI_TO_SEND_DATA() \
{ \
    USI_DDR |=  _BV(SDA); \
    USISR = _BV(USIOIF) | _BV(USIPF) | _BV(USIDC); \
}


#define SET_USI_TO_READ_DATA() \
{ \
    USI_DDR &= ~_BV(SDA); \
    USISR = _BV(USIOIF) | _BV(USIPF) | _BV(USIDC); \
}



void i2c_init(void)
{
    state = START;
    USI_PORT |= _BV(SCL) | _BV(SDA);
    USI_DDR |= _BV(SCL);
    USI_DDR &= ~_BV(SDA);
    USISR = _BV(USISIF) | _BV(USIOIF) | _BV(USIPF) | _BV(USIDC);
    USICR = _BV(USISIE) | _BV(USIWM1) | _BV(USICS1);
}


ISR(USI_STRT_vect)
{
    while ( ((USI_PIN & _BV(SCL)) != 0) && ((USISR & _BV(USIPF)) == 0)) {}

    state = ADDRESS;
    USICR = _BV(USISIE) | _BV(USIOIE) | _BV(USIWM1) | _BV(USIWM0) | _BV(USICS1);
    USISR = _BV(USISIF) | _BV(USIOIF) | _BV(USIPF);
}


ISR(USI_OVF_vect)
{
    switch (state)
    {
        case START:
            // We should not be here.
            SET_USI_TO_START_CONDITION_MODE();
            break;

        case ADDRESS:
            if ((USIDR >> 1) == I2C_ADDR)
            {
                led_flash(50);
                if ((USIDR & 1) != 0)
                {
                    uint8_t idx = 0;
                    data[idx++] = MCUSR;
                    for (control_t control = FAN; control <= COOL; control++)
                    {
                        data[idx++] = gpio_status(control);
                    }
                    state = SEND;
                }
                else
                {
                    state = RECV_ACK;
                }
                data_index = 0;
                SET_USI_TO_SEND_ACK();
            }
            else
            {
                state = START;
                SET_USI_TO_START_CONDITION_MODE();
            }
            break;

        case SEND_WAIT:
            if (USIDR != 0)
            {
                // NAK received.
                state = START;
                SET_USI_TO_START_CONDITION_MODE();
                break;
            }
            // Fall into SEND if a SEND ACK was received.
        case SEND:
            if (data_index < DATA_SIZE)
            {
                USIDR = data[data_index];
                data_index++;
                state = SEND_ACK;
                SET_USI_TO_SEND_DATA();
                led_flash(50);
            }
            else
            {
                state = START;
                SET_USI_TO_START_CONDITION_MODE();
            }
            break;

        case SEND_ACK:
            state = SEND_WAIT;
            SET_USI_TO_READ_ACK();
            break;

        case RECV:
            if (data_index < DATA_SIZE)
            {
                data[data_index] = USIDR;
                data_index++;
                if (data_index == DATA_SIZE)
                {
                    uint8_t idx = 0;
                    MCUSR ^= data[idx++] & 0xf;
                    for (control_t control = FAN; control <= COOL; control++)
                    {
                        switch (data[idx++])
                        {
                            case GPIO_ON:
                                gpio_set(control);
                                break;

                            case GPIO_OFF:
                                gpio_clr(control);
                                break;

                            case GPIO_NO_CHANGE:
                            default:
                                break;
                        }
                    }
                }
            }
            state = RECV_ACK;
            SET_USI_TO_SEND_ACK();
            led_flash(50);
            break;

        case RECV_ACK:
            state = RECV;
            SET_USI_TO_READ_DATA();
            break;
    }
}
