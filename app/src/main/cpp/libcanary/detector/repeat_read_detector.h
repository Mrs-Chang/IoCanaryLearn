//
// Created by 高昶 on 2024/6/21.
//

#ifndef IOCANARYLEARN_REPEAT_READ_DETECTOR_H
#define IOCANARYLEARN_REPEAT_READ_DETECTOR_H
#include <unordered_map>
#include "detector.h"

namespace iocanary {
    class RepeatReadInfo {
    public:
        RepeatReadInfo(const std::string& path, const std::string& java_stack, long java_thread_id
                , long op_size, int file_size);

        bool operator== (const RepeatReadInfo& target) const;

        void IncRepeatReadCount();
        int GetRepeatReadCount();
        std::string GetStack();

    public:
        const std::string path_;
        const std::string java_stack_;
        const long java_thread_id_;
        const long op_size_;
        const int file_size_;

        int repeat_cnt_;
        int64_t op_timems;
    };

    class FileIORepeatReadDetector : public FileIODetector {
    public:
        virtual void Detect(const IOCanaryEnv& env, const IOInfo& file_io_info, std::vector<Issue>& issues) override ;

        constexpr static const IssueType kType = IssueType::kIssueRepeatRead;
    private:
        std::unordered_map<std::string, std::vector<RepeatReadInfo>> observing_map_;
    };
}


#endif //IOCANARYLEARN_REPEAT_READ_DETECTOR_H
