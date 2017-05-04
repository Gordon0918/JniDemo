MY_BUILD_EXECUTABLE := $(call my-dir)/build-executable.mk
include $(call all-subdir-makefiles)

LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE := daemon
MY_LOCAL_MODULE_FILENAME := libdaemon.so
LOCAL_SRC_FILES := daemon.c
LOCAL_LDLIBS    := -lm -llog
include $(MY_BUILD_EXECUTABLE)