
#ifndef DEFINITIONS_H
#define DEFINITIONS_H


#define CONTROL_SIZE 3
typedef enum Control {FAN = 0,HEAT,COOL} control_t;

#define LOCKOUT_SECONDS 300
#define FAN_LOCKOUT_SECONDS 5
#define TIMEOUT_SECONDS 10

#define I2C_ADDR    0x33


#endif
