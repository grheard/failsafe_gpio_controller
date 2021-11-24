# Fail-Safe GPIO Controller

## Specifications

* 1 fan releay
* 1 heat relay
* 1 cooling relay
* i2c interface
* i2c watchdog to turn off gpio on missed write frequency for heating and cooling gpios in the on state
* 5 minute lockout timer anytime heating and cooling gpios toggle from the active to inactive state
* 5 minute lockout timer is active after power on, software watchdog, and i2c watchdog activations
* i2c write commands turning on any gpio must repeat within 1s to keep the gpio from timing out
* i2c read command to fetch gpio, timer, and watchdog status