//
// Created by 高昶 on 2024/6/20.
//

#ifndef IOCANARYLEARN_IO_CANARY_H
#define IOCANARYLEARN_IO_CANARY_H

#include <condition_variable>
#include <memory>
#include <deque>
#include "io_info_collector.h"
#include "detector.h"

namespace iocanary {
    typedef void(*OnPublishIssueCallback) (const std::vector<Issue>& published_issues);
    class IOCanary {
    public:
        IOCanary(const IOCanary&) = delete;
        IOCanary& operator=(IOCanary const&) = delete;

        static IOCanary& Get();

        void RegisterDetector(DetectorType type);
        void SetConfig(IOCanaryConfigKey key, long val);
        void SetJavaMainThreadId(long main_thread_id);

        void SetIssuedCallback(OnPublishIssueCallback issued_callback);

        void OnOpen(const char *pathname, int flags, mode_t mode, int open_ret, const JavaContext& java_context);
        void OnRead(int fd, const void *buf, size_t size, ssize_t read_ret, long read_cost);
        void OnWrite(int fd, const void *buf, size_t size, ssize_t write_ret, long write_cost);
        void OnClose(int fd, int close_ret);

    private:
        IOCanary();
        ~IOCanary();

        void OfferFileIOInfo(std::shared_ptr<IOInfo> file_io_info);
        int TakeFileIOInfo(std::shared_ptr<IOInfo>& file_io_info);
        void Detect();

        bool exit_;

        IOCanaryEnv env_;
        OnPublishIssueCallback issued_callback_;
        std::vector<FileIODetector*> detectors_;

        IOInfoCollector collector_;
        std::deque<std::shared_ptr<IOInfo>> queue_;
        std::mutex queue_mutex_;
        std::condition_variable queue_cv_;
    };

}

#endif //IOCANARYLEARN_IO_CANARY_H
