
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_ARM_MODE := arm
ifeq ($(BUILD_WITH_NEON),1)
LOCAL_ARM_NEON := true
endif

LOCAL_MODULE := libdca

LOCAL_CFLAGS += \
    -DHAVE_CONFIG_H

LOCAL_C_INCLUDES += \
    $(LOCAL_PATH) \
    $(LOCAL_PATH)/include

LOCAL_SRC_FILES := \
    libdca/bitstream.c \
    libdca/downmix.c \
    libdca/parse.c

include $(BUILD_STATIC_LIBRARY)

