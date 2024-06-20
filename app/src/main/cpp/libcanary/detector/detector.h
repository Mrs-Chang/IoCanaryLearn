//
// Created by 高昶 on 2024/6/20.
//

#ifndef IOCANARYLEARN_DETECTOR_H
#define IOCANARYLEARN_DETECTOR_H

#include <set>
#include <vector>
#include "io_canary_env.h"
#include "io_info_collector.h"

namespace iocanary {

    typedef enum {
        kDetectorMainThreadIO = 0,
        kDetectorSmallBuffer,
        kDetectorRepeatRead
    } DetectorType;

    typedef enum {
        kIssueMainThreadIO = 1,
        kIssueSmallBuffer,
        kIssueRepeatRead
    } IssueType;

    class Issue {
    public:
        Issue(IssueType type, IOInfo file_io_info);

        const IssueType type_;
        const IOInfo file_io_info_;
        const std::string key_;

        int repeat_read_cnt_;
        std::string stack;
    private:
        static std::string GenKey(const IOInfo& file_io_info);
    };

    class FileIODetector {
    public:
        virtual ~FileIODetector();

        virtual void Detect(const IOCanaryEnv& env, const IOInfo& file_io_info, std::vector<Issue>& issues) = 0;

    protected:
        void PublishIssue(const Issue& target, std::vector<Issue>& issues);

    private:
        void MarkIssuePublished(const std::string& key);
        bool IsIssuePublished(const std::string& key);

        std::set<std::string> published_issue_set_;
    };
}

#endif //IOCANARYLEARN_DETECTOR_H
