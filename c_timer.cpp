#include "c_timer.h"

c_timer::c_timer() {
    //cpu ticks
    QueryPerformanceFrequency(&_perf_freq);
    reset();
}
c_timer::~c_timer() {

}

void c_timer::reset() {
    QueryPerformanceCounter(&_perf_start);
}

float c_timer::get_interval() {
    QueryPerformanceCounter(&_perf_now);
    float interval = (((_perf_now.QuadPart - _perf_start.QuadPart) * 1000.0f) / _perf_freq.QuadPart);
    return interval;
}
