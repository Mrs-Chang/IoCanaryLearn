//
// Created by 高昶 on 2024/6/20.
//

#include "detector.h"
#include "io_canary_utils.h"

namespace iocanary {

    FileIODetector::~FileIODetector() {}

    Issue::Issue(IssueType type, IOInfo file_io_info) : type_(type), key_(GenKey(file_io_info)),
                                                        file_io_info_(file_io_info) {
        repeat_read_cnt_ = 0;
        stack = file_io_info.java_context_.stack_;
    }

    std::string Issue::GenKey(const IOInfo &file_io_info) {
        //return MD5(file_io_info.path_ + ":" + GetLatestStack(file_io_info.java_context_.stack_, 4));
        return "xxx";
    }

    bool FileIODetector::IsIssuePublished(const std::string &key) {
        return published_issue_set_.find(key) != published_issue_set_.end();
    }

    void FileIODetector::PublishIssue(const Issue &target, std::vector<Issue> &issues) {
        if (IsIssuePublished(target.key_)) {
            return;
        }

        issues.push_back(target);

        MarkIssuePublished(target.key_);
    }


    void FileIODetector::MarkIssuePublished(const std::string &key) {
        published_issue_set_.insert(key);
    }

}
