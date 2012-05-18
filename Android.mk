ifeq ($(BOARD_HAS_ATH_WLAN), true)
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
endif

ifeq ($(BOARD_HAS_ATH_WLAN_AR6004), true)
LOCAL_PATH := $(call my-dir)
DLKM_DIR := build/dlkm

export BUILD_ATH6KL_VER_35=1

include $(CLEAR_VARS)
LOCAL_MODULE             := cfg80211.ko
LOCAL_MODULE_TAGS        := eng
LOCAL_MODULE_PATH        := $(TARGET_OUT)/lib/modules/ath6kl-3.5
include $(DLKM_DIR)/AndroidKernelModule.mk

include $(CLEAR_VARS)
LOCAL_MODULE             := ath6kl_usb.ko
LOCAL_MODULE_TAGS        := eng
LOCAL_MODULE_KBUILD_NAME := wlan.ko
LOCAL_MODULE_PATH        := $(TARGET_OUT)/lib/modules/ath6kl-3.5
include $(DLKM_DIR)/AndroidKernelModule.mk
endif
