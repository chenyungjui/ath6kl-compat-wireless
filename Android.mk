LOCAL_PATH := $(call my-dir)
DLKM_DIR := build/dlkm

include $(CLEAR_VARS)
LOCAL_MODULE             := cfg80211.ko
LOCAL_MODULE_TAGS        := eng
LOCAL_MODULE_PATH        := $(TARGET_OUT)/lib/modules/ath6kl
include $(DLKM_DIR)/AndroidKernelModule.mk

include $(CLEAR_VARS)
LOCAL_MODULE             := ath6kl_sdio.ko
LOCAL_MODULE_KBUILD_NAME := wlan.ko
LOCAL_MODULE_TAGS        := eng
LOCAL_MODULE_PATH        := $(TARGET_OUT)/lib/modules/ath6kl
include $(DLKM_DIR)/AndroidKernelModule.mk
