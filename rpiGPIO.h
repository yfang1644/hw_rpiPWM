#ifndef _RPIGPIO_H
#define _PRIGPIO_H

/* hardware related constants */
#define BCM2709_PERI_BASE_DEFAULT   0x3f000000
#define GPIO_BASE_OFFSET    0x200000

#define FSEL_OFFSET         0

#define INPUT               0
#define OUTPUT              0

#define ALT0        4       /* GPIO12/13 to pwm, GPIO2/3 tio i2c */
#define ALT5        2       /* GPIO18/19 to pwm */

#define BLOCK_SIZE   4096

unsigned int *mapRegAddr(unsigned int baseAddr);
int configPinAlt(int gpio, int mode);

#endif    /* _RPIGPIO_H */
