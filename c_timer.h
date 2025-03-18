#pragma once
#include <Windows.h>

class c_timer
{
public:
    c_timer();
    virtual ~c_timer();

public:
    float get_interval(); //��ȡ���ʱ��
    void reset(); //���¿�ʼ��ʱ

private:
    LARGE_INTEGER _perf_freq; //cpu����
    LARGE_INTEGER _perf_start;//��ʼ����
    LARGE_INTEGER _perf_now;//��ǰ����
};