//
// Created by 高昶 on 2024/6/19.
//

#ifndef MATRIX_IO_CANARY_IO_INFO_COLLECTOR_H
#define MATRIX_IO_CANARY_IO_INFO_COLLECTOR_H

#include <jni.h>
#include <unistd.h>
#include <string>
#include <map>
#include <memory>
#include <mutex>
#include <unordered_map>

#include "io_canary_utils.h"

namespace iocanary {

    typedef enum {
        kInit = 0,
        kRead ,
        kWrite
    } FileOpType;

    class JavaContext {
    public:
        JavaContext(intmax_t thread_id , const std::string& thread_name, const std::string& stack)
                : thread_id_(thread_id), thread_name_(thread_name), stack_(stack) {
        }

        const intmax_t thread_id_;
        const std::string thread_name_;
        const std::string stack_;
    };

    class IOInfo {
    public:
        IOInfo(const std::string path, const JavaContext java_context)
                : start_time_μs_(GetSysTimeMicros()),op_type_(kInit)
                , path_(path), java_context_(java_context) {
        }

        const std::string path_;
        const JavaContext java_context_;

        int64_t start_time_μs_;
        FileOpType op_type_ = kInit;
        int op_cnt_ = 0;
        long buffer_size_ = 0;
        long op_size_ = 0;
        long rw_cost_us_ = 0;
        long max_continual_rw_cost_time_μs_ = 0;
        long max_once_rw_cost_time_μs_ = 0;
        long current_continual_rw_time_μs_ = 0;
        int64_t last_rw_time_μs_ = 0;
        long file_size_ = 0;

        long total_cost_μs_ = 0;
    };

    // A singleton to collect and generate operation info
    class IOInfoCollector {
    public:
        void OnOpen(const char *pathname, int flags, mode_t mode, int open_ret, const JavaContext& java_context);
        void OnRead(int fd, const void *buf, size_t size, ssize_t read_ret, long read_cost);
        void OnWrite(int fd, const void *buf, size_t size, ssize_t write_ret, long write_cost);
        std::shared_ptr<IOInfo> OnClose(int fd, int close_ret);

    private:
        //constexpr static const char* kTag = "IOCanary.native.FileIOInfoCollector";
        constexpr static const int kContinualThreshold = 8*1000;//in μs， half of 16.6667

        void CountRWInfo(int fd, const FileOpType& file_op_type, long op_size, long rw_cost);

        std::unordered_map<int, std::shared_ptr<IOInfo>> info_map_;
    };
}

#endif //MATRIX_IO_CANARY_IO_INFO_COLLECTOR_H
