#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

unsigned long long currentSeconds() {
    timespec spec;
    clock_gettime(CLOCK_THREAD_CPUTIME_ID, &spec);
    return (unsigned long long)(static_cast<float>(spec.tv_sec) * 1e9 + static_cast<float>(spec.tv_nsec));
}
