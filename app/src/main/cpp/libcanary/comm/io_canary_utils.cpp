//
// Created by 高昶 on 2024/6/19.
//
#include "io_canary_utils.h"
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/time.h>

namespace iocanary {
    int64_t GetSysTimeMicros() {
        timeval tv;
        gettimeofday(&tv, 0);
        return (int64_t) tv.tv_sec * 1000000 + (int64_t) tv.tv_usec;
    }

    int64_t GetSysTimeMilliSecond() {
        return (int64_t) (GetSysTimeMicros() / 1000);
    }
}