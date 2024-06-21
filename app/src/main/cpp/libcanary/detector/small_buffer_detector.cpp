//
// Created by 高昶 on 2024/6/21.
//

#include "small_buffer_detector.h"

namespace iocanary {

    void FileIOSmallBufferDetector::Detect(const IOCanaryEnv &env, const IOInfo &file_io_info,
                                           std::vector<Issue>& issues) {

        if (file_io_info.op_cnt_ > env.kSmallBufferOpTimesThreshold && (file_io_info.op_size_ / file_io_info.op_cnt_) < env.GetSmallBufferThreshold()
            && file_io_info.max_continual_rw_cost_time_μs_ >= env.kPossibleNegativeThreshold) {

            PublishIssue(Issue(kType, file_io_info), issues);
        }
    }
}
