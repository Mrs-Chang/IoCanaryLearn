
#include <jni.h>
#include <string>
#include <android/log.h>
#include "libxhook/xhook_ext.h"
#include <assert.h>
#include <algorithm>
#include "libcanary/comm/io_canary_utils.h"
#include "libcanary/core/io_canary.h"

#define LOG_TAG "JNI_TAG"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)

namespace iocanary {
    static const char *const kTag = "IOCanary.JNI";

    static int (*original_open)(const char *pathname, int flags, mode_t mode);

    static int (*original_open64)(const char *pathname, int flags, mode_t mode);

    static ssize_t (*original_read)(int fd, void *buf, size_t size);

    static ssize_t (*original_read_chk)(int fd, void *buf, size_t count, size_t buf_size);

    static ssize_t (*original_write)(int fd, const void *buf, size_t size);

    static ssize_t (*original_write_chk)(int fd, const void *buf, size_t count, size_t buf_size);

    static int (*original_close)(int fd);

    static int (*original_android_fdsan_close_with_tag)(int fd, uint64_t ownerId);

    static bool kInitSuc = false;
    static JavaVM *kJvm;

    static jclass kJavaBridgeClass;
    static jmethodID kMethodIDOnIssuePublish;

    static jclass kJavaContextClass;
    static jmethodID kMethodIDGetJavaContext;
    static jfieldID kFieldIDStack;
    static jfieldID kFieldIDThreadName;

    static jclass kIssueClass;
    static jmethodID kMethodIDIssueConstruct;

    static jclass kListClass;
    static jmethodID kMethodIDListConstruct;
    static jmethodID kMethodIDListAdd;

    const static char *TARGET_MODULES[] = {
            "libopenjdkjvm.so",
            "libjavacore.so",
            "libopenjdk.so"
    };

    const static size_t TARGET_MODULE_COUNT = sizeof(TARGET_MODULES) / sizeof(char *);

    extern "C" {
    static JNIEnv *getJNIEnv() {
        //ensure kJvm init
        assert(kJvm != NULL);

        JNIEnv *env = NULL;
        int ret = kJvm->GetEnv((void **) &env, JNI_VERSION_1_6);
        if (ret != JNI_OK) {
            __android_log_print(ANDROID_LOG_ERROR, kTag, "getJNIEnv !JNI_OK: %d", ret);
        }

        return env;
    }

    char *jstringToChars(JNIEnv *env, jstring jstr) {
        if (jstr == nullptr) {
            return nullptr;
        }

        jboolean isCopy = JNI_FALSE;
        const char *str = env->GetStringUTFChars(jstr, &isCopy);
        char *ret = strdup(str);
        env->ReleaseStringUTFChars(jstr, str);
        return ret;
    }

    void OnIssuePublish(const std::vector<Issue> &published_issues) {
        if (!kInitSuc) {
            __android_log_print(ANDROID_LOG_ERROR, kTag, "OnIssuePublish kInitSuc false");
            return;
        }

        JNIEnv *env;
        bool attached = false;
        jint j_ret = kJvm->GetEnv((void **) &env, JNI_VERSION_1_6);
        if (j_ret == JNI_EDETACHED) {
            jint jAttachRet = kJvm->AttachCurrentThread(&env, nullptr);
            if (jAttachRet != JNI_OK) {
                __android_log_print(ANDROID_LOG_ERROR, kTag,
                                    "onIssuePublish AttachCurrentThread !JNI_OK");
                return;
            } else {
                attached = true;
            }
        } else if (j_ret != JNI_OK || env == NULL) {
            return;
        }

        jthrowable exp = env->ExceptionOccurred();
        if (exp != NULL) {
            __android_log_print(ANDROID_LOG_INFO, kTag,
                                "checkCanCallbackToJava ExceptionOccurred, return false");
            env->ExceptionDescribe();
            return;
        }

        jobject j_issues = env->NewObject(kListClass, kMethodIDListConstruct);

        for (const auto &issue: published_issues) {
            jint type = issue.type_;
            jstring path = env->NewStringUTF(issue.file_io_info_.path_.c_str());
            jlong file_size = issue.file_io_info_.file_size_;
            jint op_cnt = issue.file_io_info_.op_cnt_;
            jlong buffer_size = issue.file_io_info_.buffer_size_;
            jlong op_cost_time = issue.file_io_info_.rw_cost_us_ / 1000;
            jint op_type = issue.file_io_info_.op_type_;
            jlong op_size = issue.file_io_info_.op_size_;
            jstring thread_name = env->NewStringUTF(
                    issue.file_io_info_.java_context_.thread_name_.c_str());
            jstring stack = env->NewStringUTF(issue.stack.c_str());
            jint repeat_read_cnt = issue.repeat_read_cnt_;

            jobject issue_obj = env->NewObject(kIssueClass, kMethodIDIssueConstruct, type, path,
                                               file_size, op_cnt, buffer_size,
                                               op_cost_time, op_type, op_size, thread_name, stack,
                                               repeat_read_cnt);

            env->CallBooleanMethod(j_issues, kMethodIDListAdd, issue_obj);

            env->DeleteLocalRef(issue_obj);
            env->DeleteLocalRef(stack);
            env->DeleteLocalRef(thread_name);
            env->DeleteLocalRef(path);
        }

        env->CallStaticVoidMethod(kJavaBridgeClass, kMethodIDOnIssuePublish, j_issues);

        env->DeleteLocalRef(j_issues);

        if (attached) {
            kJvm->DetachCurrentThread();
        }
    }

    static void DoProxyOpenLogic(const char *pathname, int flags, mode_t mode, int ret) {
        JNIEnv *env = NULL;
        kJvm->GetEnv((void **) &env, JNI_VERSION_1_6);
        if (env == NULL || !kInitSuc) {
            __android_log_print(ANDROID_LOG_ERROR, kTag, "ProxyOpen env null or kInitSuc:%d",
                                kInitSuc);
        } else {
            jobject java_context_obj = env->CallStaticObjectMethod(kJavaBridgeClass,
                                                                   kMethodIDGetJavaContext);
            if (NULL == java_context_obj) {
                return;
            }

            jstring j_stack = (jstring) env->GetObjectField(java_context_obj, kFieldIDStack);
            jstring j_thread_name = (jstring) env->GetObjectField(java_context_obj,
                                                                  kFieldIDThreadName);

            char *thread_name = jstringToChars(env, j_thread_name);
            char *stack = jstringToChars(env, j_stack);
            JavaContext java_context(GetCurrentThreadId(), thread_name == NULL ? "" : thread_name,
                                     stack == NULL ? "" : stack);
            free(stack);
            free(thread_name);

            iocanary::IOCanary::Get().OnOpen(pathname, flags, mode, ret, java_context);

            env->DeleteLocalRef(java_context_obj);
            env->DeleteLocalRef(j_stack);
            env->DeleteLocalRef(j_thread_name);
        }
    }

    int ProxyOpen(const char *pathname, int flags, mode_t mode) {
        if (!IsMainThread()) {
            return original_open(pathname, flags, mode);
        }

        int ret = original_open(pathname, flags, mode);

        if (ret != -1) {
            DoProxyOpenLogic(pathname, flags, mode, ret);
        }

        return ret;
    }

    int ProxyOpen64(const char *pathname, int flags, mode_t mode) {
        if (!IsMainThread()) {
            return original_open64(pathname, flags, mode);
        }

        int ret = original_open64(pathname, flags, mode);

        if (ret != -1) {
            DoProxyOpenLogic(pathname, flags, mode, ret);
        }

        return ret;
    }

    ssize_t ProxyRead(int fd, void *buf, size_t size) {
        if (!IsMainThread()) {
            return original_read(fd, buf, size);
        }

        int64_t start = GetTickCountMicros();

        size_t ret = original_read(fd, buf, size);

        long read_cost_us = GetTickCountMicros() - start;

        __android_log_print(ANDROID_LOG_DEBUG, kTag,
                            "ProxyRead fd:%d buf:%p size:%d ret:%d cost:%d", fd, buf, size, ret,
                            read_cost_us);

        iocanary::IOCanary::Get().OnRead(fd, buf, size, ret, read_cost_us);

        return ret;
    }

    ssize_t ProxyReadChk(int fd, void *buf, size_t count, size_t buf_size) {
        if (!IsMainThread()) {
            return original_read_chk(fd, buf, count, buf_size);
        }

        int64_t start = GetTickCountMicros();

        ssize_t ret = original_read_chk(fd, buf, count, buf_size);

        long read_cost_us = GetTickCountMicros() - start;

//        __android_log_print(ANDROID_LOG_DEBUG, kTag,
//                            "ProxyRead fd:%d buf:%p size:%d ret:%d cost:%d", fd, buf, size, ret,
//                            read_cost_us);

        iocanary::IOCanary::Get().OnRead(fd, buf, count, ret, read_cost_us);

        return ret;
    }

    ssize_t ProxyWrite(int fd, const void *buf, size_t size) {
        if (!IsMainThread()) {
            return original_write(fd, buf, size);
        }

        int64_t start = GetTickCountMicros();

        size_t ret = original_write(fd, buf, size);

        long write_cost_us = GetTickCountMicros() - start;

        __android_log_print(ANDROID_LOG_DEBUG, kTag,
                            "ProxyWrite fd:%d buf:%p size:%d ret:%d cost:%d", fd, buf, size,
                            ret, write_cost_us);

        iocanary::IOCanary::Get().OnWrite(fd, buf, size, ret, write_cost_us);

        return ret;
    }

    ssize_t ProxyWriteChk(int fd, const void *buf, size_t count, size_t buf_size) {
        if (!IsMainThread()) {
            return original_write_chk(fd, buf, count, buf_size);
        }

        int64_t start = GetTickCountMicros();

        ssize_t ret = original_write_chk(fd, buf, count, buf_size);

        long write_cost_us = GetTickCountMicros() - start;

        //__android_log_print(ANDROID_LOG_DEBUG, kTag, "ProxyWrite fd:%d buf:%p size:%d ret:%d cost:%d", fd, buf, size, ret, write_cost_us);

        iocanary::IOCanary::Get().OnWrite(fd, buf, count, ret, write_cost_us);

        return ret;
    }

    int ProxyClose(int fd) {
        if (!IsMainThread()) {
            return original_close(fd);
        }

        int ret = original_close(fd);

        __android_log_print(ANDROID_LOG_DEBUG, kTag, "ProxyClose fd:%d ret:%d", fd, ret);
        iocanary::IOCanary::Get().OnClose(fd, ret);

        return ret;
    }

    int Proxy_android_fdsan_close_with_tag(int fd, uint64_t ownerId) {
        if (!IsMainThread()) {
            return original_android_fdsan_close_with_tag(fd, ownerId);
        }
        int ret = original_android_fdsan_close_with_tag(fd, ownerId);
        iocanary::IOCanary::Get().OnClose(fd, ret);
        return ret;
    }

    JNIEXPORT void JNICALL
    Java_com_chang_iocanarylearn_IOCanaryJniBridge_enableDetector(JNIEnv *env, jclass clazz,
                                                                  jint detector_type) {
        iocanary::IOCanary::Get().RegisterDetector(static_cast<DetectorType>(detector_type));
    }

    JNIEXPORT void JNICALL
    Java_com_chang_iocanarylearn_IOCanaryJniBridge_setConfig(JNIEnv *env, jclass clazz, jint key,
                                                             jlong val) {
        iocanary::IOCanary::Get().SetConfig(static_cast<IOCanaryConfigKey>(key), val);
    }

    JNIEXPORT jboolean JNICALL
    Java_com_chang_iocanarylearn_IOCanaryJniBridge_doHook(JNIEnv *env, jclass clazz) {
        __android_log_print(ANDROID_LOG_INFO, kTag, "doHook");

        for (int i = 0; i < TARGET_MODULE_COUNT; ++i) {
            const char *so_name = TARGET_MODULES[i];
            __android_log_print(ANDROID_LOG_INFO, kTag, "try to hook function in %s.", so_name);

            void *soinfo = xhook_elf_open(so_name);
            if (!soinfo) {
                __android_log_print(ANDROID_LOG_WARN, kTag, "Failure to open %s, try next.",
                                    so_name);
                continue;
            }

            xhook_got_hook_symbol(soinfo, "open", (void *) ProxyOpen, (void **) &original_open);
            xhook_got_hook_symbol(soinfo, "open64", (void *) ProxyOpen64,
                                  (void **) &original_open64);

            bool is_libjavacore = (strstr(so_name, "libjavacore.so") != nullptr);
            if (is_libjavacore) {
                if (xhook_got_hook_symbol(soinfo, "read", (void *) ProxyRead,
                                          (void **) &original_read) != 0) {
                    __android_log_print(ANDROID_LOG_WARN, kTag,
                                        "doHook hook read failed, try __read_chk");
                    if (xhook_got_hook_symbol(soinfo, "__read_chk", (void *) ProxyReadChk,
                                              (void **) &original_read_chk) != 0) {
                        __android_log_print(ANDROID_LOG_WARN, kTag,
                                            "doHook hook failed: __read_chk");
                        xhook_elf_close(soinfo);
                        return JNI_FALSE;
                    }
                }

                if (xhook_got_hook_symbol(soinfo, "write", (void *) ProxyWrite,
                                          (void **) &original_write) != 0) {
                    __android_log_print(ANDROID_LOG_WARN, kTag,
                                        "doHook hook write failed, try __write_chk");
                    if (xhook_got_hook_symbol(soinfo, "__write_chk", (void *) ProxyWriteChk,
                                              (void **) &original_write_chk) != 0) {
                        __android_log_print(ANDROID_LOG_WARN, kTag,
                                            "doHook hook failed: __write_chk");
                        xhook_elf_close(soinfo);
                        return JNI_FALSE;
                    }
                }
            }

            xhook_got_hook_symbol(soinfo, "close", (void *) ProxyClose, (void **) &original_close);
            xhook_got_hook_symbol(soinfo, "android_fdsan_close_with_tag",
                                  (void *) Proxy_android_fdsan_close_with_tag,
                                  (void **) &original_android_fdsan_close_with_tag);

            xhook_elf_close(soinfo);
        }

        __android_log_print(ANDROID_LOG_INFO, kTag, "doHook done.");
        return JNI_TRUE;
    }

    JNIEXPORT jboolean JNICALL
    Java_com_chang_iocanarylearn_IOCanaryJniBridge_doUnHook(JNIEnv *env, jclass clazz) {
        __android_log_print(ANDROID_LOG_INFO, kTag, "doUnHook");
        for (int i = 0; i < TARGET_MODULE_COUNT; ++i) {
            const char *so_name = TARGET_MODULES[i];
            void *soinfo = xhook_elf_open(so_name);
            if (!soinfo) {
                continue;
            }
            xhook_got_hook_symbol(soinfo, "open", (void *) original_open, nullptr);
            xhook_got_hook_symbol(soinfo, "open64", (void *) original_open64, nullptr);
            xhook_got_hook_symbol(soinfo, "read", (void *) original_read, nullptr);
            xhook_got_hook_symbol(soinfo, "write", (void *) original_write, nullptr);
            xhook_got_hook_symbol(soinfo, "__read_chk", (void *) original_read_chk, nullptr);
            xhook_got_hook_symbol(soinfo, "__write_chk", (void *) original_write_chk, nullptr);
            xhook_got_hook_symbol(soinfo, "close", (void *) original_close, nullptr);

            xhook_elf_close(soinfo);
        }
        return JNI_TRUE;
    }

    static bool InitJniEnv(JavaVM *vm) {
        kJvm = vm;
        JNIEnv *env = NULL;
        if (kJvm->GetEnv((void **) &env, JNI_VERSION_1_6) != JNI_OK) {
            __android_log_print(ANDROID_LOG_ERROR, kTag, "InitJniEnv GetEnv !JNI_OK");
            return false;
        }

        jclass temp_cls = env->FindClass("com/chang/iocanarylearn/IOCanaryJniBridge");
        if (temp_cls == NULL) {
            __android_log_print(ANDROID_LOG_ERROR, kTag, "InitJniEnv kJavaBridgeClass NULL");
            return false;
        }
        kJavaBridgeClass = reinterpret_cast<jclass>(env->NewGlobalRef(temp_cls));

        jclass temp_java_context_cls = env->FindClass(
                "com/chang/iocanarylearn/IOCanaryJniBridge$JavaContext");
        if (temp_java_context_cls == NULL) {
            __android_log_print(ANDROID_LOG_ERROR, kTag, "InitJniEnv kJavaBridgeClass NULL");
            return false;
        }
        kJavaContextClass = reinterpret_cast<jclass>(env->NewGlobalRef(temp_java_context_cls));
        kFieldIDStack = env->GetFieldID(kJavaContextClass, "stack", "Ljava/lang/String;");
        kFieldIDThreadName = env->GetFieldID(kJavaContextClass, "threadName", "Ljava/lang/String;");
        if (kFieldIDStack == NULL || kFieldIDThreadName == NULL) {
            __android_log_print(ANDROID_LOG_ERROR, kTag, "InitJniEnv kJavaContextClass field NULL");
            return false;
        }

        kMethodIDOnIssuePublish = env->GetStaticMethodID(kJavaBridgeClass, "onIssuePublish",
                                                         "(Ljava/util/ArrayList;)V");
        if (kMethodIDOnIssuePublish == NULL) {
            __android_log_print(ANDROID_LOG_ERROR, kTag, "InitJniEnv kMethodIDOnIssuePublish NULL");
            return false;
        }

        kMethodIDGetJavaContext = env->GetStaticMethodID(kJavaBridgeClass, "getJavaContext",
                                                         "()Lcom/chang/iocanarylearn/IOCanaryJniBridge$JavaContext;");
        if (kMethodIDGetJavaContext == NULL) {
            __android_log_print(ANDROID_LOG_ERROR, kTag, "InitJniEnv kMethodIDGetJavaContext NULL");
            return false;
        }

        jclass temp_issue_cls = env->FindClass("com/chang/iocanarylearn/IOIssue");
        if (temp_issue_cls == NULL) {
            __android_log_print(ANDROID_LOG_ERROR, kTag, "InitJniEnv kIssueClass NULL");
            return false;
        }
        kIssueClass = reinterpret_cast<jclass>(env->NewGlobalRef(temp_issue_cls));

        kMethodIDIssueConstruct = env->GetMethodID(kIssueClass, "<init>",
                                                   "(ILjava/lang/String;JIJJIJLjava/lang/String;Ljava/lang/String;I)V");
        if (kMethodIDIssueConstruct == NULL) {
            __android_log_print(ANDROID_LOG_ERROR, kTag, "InitJniEnv kMethodIDIssueConstruct NULL");
            return false;
        }

        jclass list_cls = env->FindClass("java/util/ArrayList");
        kListClass = reinterpret_cast<jclass>(env->NewGlobalRef(list_cls));
        kMethodIDListConstruct = env->GetMethodID(list_cls, "<init>", "()V");
        kMethodIDListAdd = env->GetMethodID(list_cls, "add", "(Ljava/lang/Object;)Z");

        return true;
    }

    JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
        __android_log_print(ANDROID_LOG_DEBUG, kTag, "JNI_OnLoad");
        kInitSuc = false;


        if (!InitJniEnv(vm)) {
            return -1;
        }

        iocanary::IOCanary::Get().SetIssuedCallback(OnIssuePublish);

        kInitSuc = true;
        __android_log_print(ANDROID_LOG_DEBUG, kTag, "JNI_OnLoad done");
        return JNI_VERSION_1_6;
    }

    JNIEXPORT void JNICALL JNI_OnUnload(JavaVM *vm, void *reserved) {
        __android_log_print(ANDROID_LOG_DEBUG, kTag, "JNI_OnUnload done");
        JNIEnv *env;
        kJvm->GetEnv((void **) &env, JNI_VERSION_1_6);
        if (env != NULL) {
            if (kIssueClass) {
                env->DeleteGlobalRef(kIssueClass);
            }
            if (kJavaBridgeClass) {
                env->DeleteGlobalRef(kJavaBridgeClass);
            }
            if (kListClass) {
                env->DeleteGlobalRef(kListClass);
            }
        }
    }

    }
}