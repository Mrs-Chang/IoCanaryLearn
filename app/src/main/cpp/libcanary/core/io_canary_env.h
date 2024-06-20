//
// Created by 高昶 on 2024/6/20.
//

#ifndef IOCANARYLEARN_IO_CANARY_ENV_H
#define IOCANARYLEARN_IO_CANARY_ENV_H

namespace iocanary {
    enum IOCanaryConfigKey {
        kMainThreadThreshold = 0,
        kSmallBufferThreshold,
        kRepeatReadThreshold,

        //!!kConfigKeysLen always the last one!!
        kConfigKeysLen
    };

    class IOCanaryEnv {
    public:
        IOCanaryEnv();

        void SetConfig(IOCanaryConfigKey key, long val);

        long GetJvaMainThreadID() const;

        long GetMainThreadThreshold() const;

        long GetSmallBufferThreshold() const;

        long GetRepeatReadThreshold() const;

        constexpr static const int kPossibleNegativeThreshold = 13 * 1000;
        constexpr static const int kSmallBufferOpTimesThreshold = 20;

    private:
        long GetConfig(IOCanaryConfigKey key) const;

        constexpr static const int kDefaultMainThreadTriggerThreshold = 500 * 1000;
        constexpr static const int kDefaultBufferSmallThreshold = 4096;
        constexpr static const int kDefaultRepeatReadThreshold = 5;

        long configs_[IOCanaryConfigKey::kConfigKeysLen];
    };
}

#endif //IOCANARYLEARN_IO_CANARY_ENV_H
