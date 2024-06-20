//
// Created by 高昶 on 2024/6/20.
//
#include "main_thread_detector.h"
#include "io_canary_utils.h"

namespace iocanary {

    void FileIOMainThreadDetector::Detect(const IOCanaryEnv &env, const IOInfo &file_io_info,
                                          std::vector<Issue>& issues) {

        //__android_log_print(ANDROID_LOG_ERROR, "FileIOMainThreadDetector", "Detect  main-thread-id：%d, thread-id:%d max_continual_rw_cost_time_μs_:%d threshold:%d"
        //      , env.GetJavaMainThreadID(), file_io_info.java_context_.thread_id_, file_io_info.max_continual_rw_cost_time_μs_, env.GetMainThreadThreshold());

        if (GetMainThreadId() == file_io_info.java_context_.thread_id_) {
            int type = 0;
            if (file_io_info.max_once_rw_cost_time_μs_ > IOCanaryEnv::kPossibleNegativeThreshold) {
                type = 1;
            }
            if(file_io_info.max_continual_rw_cost_time_μs_ > env.GetMainThreadThreshold()) {
                type |= 2;
            }

            if (type != 0) {
                Issue issue(kType, file_io_info);
                issue.repeat_read_cnt_ = type;  //use repeat to record type
                PublishIssue(issue, issues);
            }
        }
    }
}