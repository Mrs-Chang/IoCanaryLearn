//
// Created by 高昶 on 2024/6/20.
//

#include "io_canary_env.h"

namespace iocanary {

    IOCanaryEnv::IOCanaryEnv() {
        configs_[IOCanaryConfigKey::kMainThreadThreshold] = kDefaultMainThreadTriggerThreshold;
        configs_[IOCanaryConfigKey::kSmallBufferThreshold] = kDefaultBufferSmallThreshold;
        configs_[IOCanaryConfigKey::kRepeatReadThreshold] = kDefaultRepeatReadThreshold;
    }

    void IOCanaryEnv::SetConfig(IOCanaryConfigKey key, long val) {
        if (key >= IOCanaryConfigKey::kConfigKeysLen) {
            return;
        }

        configs_[key] = val;
    }

    long IOCanaryEnv::GetMainThreadThreshold() const {
        return GetConfig(IOCanaryConfigKey::kMainThreadThreshold);
    }

    long IOCanaryEnv::GetSmallBufferThreshold() const {
        return GetConfig(IOCanaryConfigKey::kSmallBufferThreshold);
    }

    long IOCanaryEnv::GetRepeatReadThreshold() const {
        return GetConfig(IOCanaryConfigKey::kRepeatReadThreshold);
    }

    long IOCanaryEnv::GetConfig(IOCanaryConfigKey key) const {
        if (key >= IOCanaryConfigKey::kConfigKeysLen) {
            return -1;
        }

        return configs_[key];
    }
}