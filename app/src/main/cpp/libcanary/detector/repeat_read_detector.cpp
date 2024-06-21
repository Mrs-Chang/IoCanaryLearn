//
// Created by 高昶 on 2024/6/21.
//

#include "repeat_read_detector.h"

namespace iocanary {

    RepeatReadInfo::RepeatReadInfo(const std::string &path, const std::string &java_stack,
                                   long java_thread_id, long op_size,
                                   int file_size) : path_(path), java_stack_(java_stack), java_thread_id_(java_thread_id) ,
                                                    op_size_(op_size), file_size_(file_size), op_timems(GetTickCount()) {
        repeat_cnt_ = 1;
    }

    bool RepeatReadInfo::operator==(const RepeatReadInfo &target) const {
        return target.path_ == path_
               && target.java_thread_id_ == java_thread_id_
               && target.java_stack_ == java_stack_
               && target.file_size_ == file_size_
               && target.op_size_ == op_size_;
    }

    void RepeatReadInfo::IncRepeatReadCount() {
        repeat_cnt_ ++;
    }

    int RepeatReadInfo::GetRepeatReadCount() {
        return repeat_cnt_;
    }

    std::string RepeatReadInfo::GetStack() {
        return java_stack_;
    }

    void FileIORepeatReadDetector::Detect(const IOCanaryEnv &env,
                                          const IOInfo &file_io_info,
                                          std::vector<Issue>& issues) {
        const std::string& path = file_io_info.path_;
        if (observing_map_.find(path) == observing_map_.end()) {
            if (file_io_info.max_continual_rw_cost_time_μs_ < env.kPossibleNegativeThreshold) {
                return;
            }

            observing_map_.insert(std::make_pair(path, std::vector<RepeatReadInfo>()));
        }

        std::vector<RepeatReadInfo>& repeat_infos = observing_map_[path];
        if (file_io_info.op_type_ == FileOpType::kWrite) {
            repeat_infos.clear();
            return;
        }

        RepeatReadInfo repeat_read_info(file_io_info.path_, file_io_info.java_context_.stack_, file_io_info.java_context_.thread_id_,
                                        file_io_info.op_size_, file_io_info.file_size_);

        if (repeat_infos.size() == 0) {
            repeat_infos.push_back(repeat_read_info);
            return;
        }

        if((GetTickCount() - repeat_infos[repeat_infos.size() - 1].op_timems) > 17) {   //17ms todo astrozhou add to params
            repeat_infos.clear();
        }

        bool found = false;
        int repeatCnt;
        for (auto& info : repeat_infos) {
            if (info == repeat_read_info) {
                found = true;

                info.IncRepeatReadCount();

                repeatCnt = info.GetRepeatReadCount();
                break;
            }
        }

        if (!found) {
            repeat_infos.push_back(repeat_read_info);
            return;
        }

        if (repeatCnt >= env.GetRepeatReadThreshold()) {
            Issue issue(kType, file_io_info);
            issue.repeat_read_cnt_ = repeatCnt;
            issue.stack = repeat_read_info.GetStack();
            PublishIssue(issue, issues);
        }

    }

}
