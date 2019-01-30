#include <stdint.h>
#include <math.h>

char tmp[25];

void f2s(float f, char *s) {
    char diff = 0;
    char *t = tmp;
    uint16_t i = (uint16_t)floor(f*100.0);
    do {
        *t = '0' + (i%10);
        if (i) i /= 10;
        t++;
        diff = t-tmp;
        if (diff==2) {
            *t = '.';
            t++;
        }
    } while (i || diff<3);
    for (t--;t!=tmp;t--,s++)
        *s = *t;
    *s = *t;
    *(++s) = 0;
}

void ui2s(uint32_t i, char *s) {
    char *t = tmp;
    do {
        *t = '0' + (i%10);
        i /= 10;
        t++;
    } while (i);
    for (t--;t!=tmp;t--,s++)
        *s = *t;
    *s = *t;
    *(++s) = 0;
}

