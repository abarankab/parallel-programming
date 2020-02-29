#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include "defs.h"

ull currentSeconds() {
    timespec spec;
    clock_gettime(CLOCK_THREAD_CPUTIME_ID, &spec);
    return spec.tv_sec * 1e9 + spec.tv_nsec;
}
