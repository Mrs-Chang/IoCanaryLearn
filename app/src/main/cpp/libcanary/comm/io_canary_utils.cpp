//
// Created by 高昶 on 2024/6/19.
//
#include "io_canary_utils.h"
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>
#include <sstream>
#include <regex>

namespace iocanary {
    int64_t GetSysTimeMicros() {
        timeval tv;
        gettimeofday(&tv, 0);
        return (int64_t) tv.tv_sec * 1000000 + (int64_t) tv.tv_usec;
    }

    int64_t GetSysTimeMilliSecond() {
        return (int64_t) (GetSysTimeMicros() / 1000);
    }

    int64_t GetTickCountMicros() {
        struct timespec ts;
        int result = clock_gettime(CLOCK_BOOTTIME, &ts);
        if (result != 0) {
            return 0;
        }
        return (int64_t) ts.tv_sec * 1000000 + (int64_t) ts.tv_nsec / 1000;
    }

    int64_t GetTickCount() {
        return GetTickCountMicros() / 1000;
    }

    intmax_t GetMainThreadId() {
        static intmax_t pid = getpid();
        return pid;
    }

    intmax_t GetCurrentThreadId() {
        return gettid();
    }

    bool IsMainThread() {
        return GetMainThreadId() == GetCurrentThreadId();
    }

    std::string GetLatestStack(const std::string &stack, int count) {
        std::vector<std::string> lines;
        Split(stack, lines, '\n', count);

        std::regex re("^(.+)(\\(.+\\))$");
        std::string latest_stack;
        for (int i = 0; i < std::min(count, (int) lines.size()); i++) {
            std::cmatch cm;
            if (std::regex_match(lines[i].c_str(), cm, re) && 3 == cm.size()) {
                latest_stack = latest_stack + cm[1].str() + "\n";
            } else {
                latest_stack = latest_stack + lines[i] + "\n";
            }
        }

        return latest_stack;
    }

    void Split(const std::string &src_str, std::vector<std::string> &sv, const char delim, int cnt) {
        sv.clear();
        std::istringstream iss(src_str);
        std::string temp;

        while (std::getline(iss, temp, delim)) {
            sv.push_back(temp);
            if (cnt > 0 && sv.size() >= cnt) {
                break;
            }
        }
    }

    int GetFileSize(const char *file_path) {
        struct stat stat_buf;
        if (-1 == stat(file_path, &stat_buf)) {
            return -1;
        }
        return stat_buf.st_size;
    }

//    std::string MD5(std::string str) {
//        unsigned char sig[16] = {0};
//        MD5_buffer(str.c_str(), str.length(), sig);
//        char des[33] = {0};
//        MD5_sig_to_string((const char *) sig, des);
//        return std::string(des);
//    }
}