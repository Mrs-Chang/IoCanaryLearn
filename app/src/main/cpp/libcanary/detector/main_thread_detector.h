//
// Created by 高昶 on 2024/6/20.
//

#ifndef IOCANARYLEARN_MAIN_THREAD_DETECTOR_H
#define IOCANARYLEARN_MAIN_THREAD_DETECTOR_H
#include "detector.h"

namespace iocanary {

    class FileIOMainThreadDetector : public FileIODetector {
    public:
        virtual void Detect(const IOCanaryEnv& env, const IOInfo& file_io_info, std::vector<Issue>& issues) override ;

        constexpr static const IssueType kType = IssueType::kIssueMainThreadIO;
    };
}

#endif //IOCANARYLEARN_MAIN_THREAD_DETECTOR_H
