#ifndef TIMER_H
#define TIMER_H

#include <time.h>

typedef struct {
    clock_t start_time;
    clock_t end_time;
} Timer;

void start_timer(Timer *timer) {
    timer->start_time = clock();
}

void stop_timer(Timer *timer) {
    timer->end_time = clock();
}

double get_elapsed_time(Timer *timer) {
    return ((double)(timer->end_time - timer->start_time)) / CLOCKS_PER_SEC;
}

#endif // TIMER_H