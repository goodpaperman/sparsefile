#pragma once
#include <Windows.h>

class c_timer
{
public:
    c_timer();
    virtual ~c_timer();

public:
    float get_interval(); //获取间隔时间
    void reset(); //重新开始计时

private:
    LARGE_INTEGER _perf_freq; //cpu精度
    LARGE_INTEGER _perf_start;//开始计数
    LARGE_INTEGER _perf_now;//当前计数
};