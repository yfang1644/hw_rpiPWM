#include <stdio.h>
#include <unistd.h>

#include "rpiPWM.h"

int main (int argc, char *argv[])
{
    int d;
    pwm_init();

    pwm_enable(0, 1);
    setFrequency(0, 10.0);

    for(d = 1; d < 90; d+=1) {
        setDutyCycle(0, (float)d);
        usleep(200000);
    }

    pwm_enable(0, 0);
    pwm_stop();
    return 0;
}
