#include <stdio.h>
#include <unistd.h>

#include "rpiI2C.h"

int main (int argc, char *argv[])
{
    int d;
    i2c_init(1500);

    i2c_write(0x40, 100);

    return 0;
}
