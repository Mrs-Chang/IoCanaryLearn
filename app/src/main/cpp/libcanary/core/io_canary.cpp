//
// Created by 高昶 on 2024/6/20.
//

#include "io_canary.h"
#include <thread>
#include "detector.h"
#include "main_thread_detector.h"
#include "repeat_read_detector.h"
#include "small_buffer_detector.h"

namespace iocanary {
    IOCanary &IOCanary::Get() {
        static IOCanary kInstance;
        return kInstance;
    }

    IOCanary::IOCanary() {
        exit_ = false;
        std::thread detect_thread(&IOCanary::Detect, this);
        detect_thread.detach();
    }

    void IOCanary::SetConfig(IOCanaryConfigKey key, long val) {
        env_.SetConfig(key, val);
    }


    void IOCanary::SetIssuedCallback(OnPublishIssueCallback issued_callback) {
        issued_callback_ = issued_callback;
    }

    void IOCanary::RegisterDetector(DetectorType type) {
        switch (type) {
            case DetectorType::kDetectorMainThreadIO:
                detectors_.push_back(new FileIOMainThreadDetector());
                break;
            case DetectorType::kDetectorSmallBuffer:
                detectors_.push_back(new FileIOSmallBufferDetector());
                break;
            case DetectorType::kDetectorRepeatRead:
                detectors_.push_back(new FileIORepeatReadDetector());
                break;
            default:
                break;
        }
    }

    void IOCanary::OnOpen(const char *pathname, int flags, mode_t mode,
                          int open_ret, const JavaContext &java_context) {
        collector_.OnOpen(pathname, flags, mode, open_ret, java_context);
    }

    void IOCanary::OnRead(int fd, const void *buf, size_t size,
                          ssize_t read_ret, long read_cost) {
        collector_.OnRead(fd, buf, size, read_ret, read_cost);
    }

    void IOCanary::OnWrite(int fd, const void *buf, size_t size,
                           ssize_t write_ret, long write_cost) {
        collector_.OnWrite(fd, buf, size, write_ret, write_cost);
    }

    void IOCanary::OnClose(int fd, int close_ret) {
        std::shared_ptr<IOInfo> info = collector_.OnClose(fd, close_ret);
        if (info == nullptr) {
            return;
        }

        OfferFileIOInfo(info);
    }

    void IOCanary::OfferFileIOInfo(std::shared_ptr<IOInfo> file_io_info) {
        std::unique_lock<std::mutex> lock(queue_mutex_);
        queue_.push_back(file_io_info);
        queue_cv_.notify_one();
        lock.unlock();
    }

    int IOCanary::TakeFileIOInfo(std::shared_ptr<IOInfo> &file_io_info) {
        std::unique_lock<std::mutex> lock(queue_mutex_);

        while (queue_.empty()) {
            queue_cv_.wait(lock);
            if (exit_) {
                return -1;
            }
        }

        file_io_info = queue_.front();
        queue_.pop_front();
        return 0;
    }

    void IOCanary::Detect() {
        std::vector<Issue> published_issues;
        std::shared_ptr<IOInfo> file_io_info;
        while (true) {
            published_issues.clear();

            int ret = TakeFileIOInfo(file_io_info);

            if (ret != 0) {
                break;
            }

            for (auto detector: detectors_) {
                detector->Detect(env_, *file_io_info, published_issues);
            }

            if (issued_callback_ && !published_issues.empty()) {
                issued_callback_(published_issues);
            }

            file_io_info = nullptr;
        }
    }

    IOCanary::~IOCanary() {
        std::unique_lock<std::mutex> lock(queue_mutex_);
        exit_ = true;
        lock.unlock();
        queue_cv_.notify_one();

        detectors_.clear();
    }
}
