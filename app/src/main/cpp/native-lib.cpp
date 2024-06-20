
#include <jni.h>
#include <string>
#include <android/log.h>
#include "libget/getutil.h"
#include "libcount/countutil.h"
#include "libcanary/comm/io_canary_utils.h"

#define LOG_TAG "JNI_TAG"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)

extern "C"
JNIEXPORT jstring JNICALL
Java_com_chang_iocanarylearn_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    LOGI("Hello from C++");
    LOGI("get1_action: %s", get1_action());
    LOGI("add_action: %d", add_action(1, 2));
    return env->NewStringUTF(hello.c_str());
}


extern "C"
JNIEXPORT jlong JNICALL
Java_com_chang_iocanarylearn_MainActivity_getTimeFromJNI(JNIEnv *env, jobject) {

    return 1;
}