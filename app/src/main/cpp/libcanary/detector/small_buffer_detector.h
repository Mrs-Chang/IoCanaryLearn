//
// Created by 高昶 on 2024/6/21.
//

#ifndef IOCANARYLEARN_SMALL_BUFFER_DETECTOR_H
#define IOCANARYLEARN_SMALL_BUFFER_DETECTOR_H

#include "detector.h"

namespace iocanary {

    class FileIOSmallBufferDetector : public FileIODetector {
    public:
        virtual void Detect(const IOCanaryEnv& env, const IOInfo& file_io_info, std::vector<Issue>& issues) override ;

        constexpr static const IssueType kType = IssueType::kIssueSmallBuffer;
    };
}

#endif //IOCANARYLEARN_SMALL_BUFFER_DETECTOR_H
