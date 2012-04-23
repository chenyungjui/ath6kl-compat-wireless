ifeq ($(BUILD_ATH6KL_VER_35), 1)
obj-m		= ath6kl_usb.o

ath6kl_usb-y += drivers/net/wireless/ath/ath6kl-3.5/debug.o
ath6kl_usb-y += drivers/net/wireless/ath/ath6kl-3.5/htc_pipe.o
ath6kl_usb-y += drivers/net/wireless/ath/ath6kl-3.5/bmi.o
ath6kl_usb-y += drivers/net/wireless/ath/ath6kl-3.5/cfg80211.o
ath6kl_usb-y += drivers/net/wireless/ath/ath6kl-3.5/cfg80211_btcoex.o
ath6kl_usb-y += drivers/net/wireless/ath/ath6kl-3.5/init.o
ath6kl_usb-y += drivers/net/wireless/ath/ath6kl-3.5/main.o
ath6kl_usb-y += drivers/net/wireless/ath/ath6kl-3.5/txrx.o
ath6kl_usb-y += drivers/net/wireless/ath/ath6kl-3.5/wmi.o
ath6kl_usb-y += drivers/net/wireless/ath/ath6kl-3.5/wmi_btcoex.o
ath6kl_usb-y += drivers/net/wireless/ath/ath6kl-3.5/usb.o
ath6kl_usb-y += drivers/net/wireless/ath/ath6kl-3.5/testmode.o
ath6kl_usb-y += drivers/net/wireless/ath/ath6kl-3.5/rttm.o
ath6kl_usb-y += drivers/net/wireless/ath/ath6kl-3.5/diag.o
ath6kl_usb-y += drivers/net/wireless/ath/ath6kl-3.5/htcoex.o

obj-m += ath6kl_sdio.o
ath6kl_sdio-y += drivers/net/wireless/ath/ath6kl-3.5/debug.o
ath6kl_sdio-y += drivers/net/wireless/ath/ath6kl-3.5/hif.o
ath6kl_sdio-y += drivers/net/wireless/ath/ath6kl-3.5/htc.o
ath6kl_sdio-y += drivers/net/wireless/ath/ath6kl-3.5/bmi.o
ath6kl_sdio-y += drivers/net/wireless/ath/ath6kl-3.5/cfg80211.o
ath6kl_sdio-y += drivers/net/wireless/ath/ath6kl-3.5/cfg80211_btcoex.o
ath6kl_sdio-y += drivers/net/wireless/ath/ath6kl-3.5/init.o
ath6kl_sdio-y += drivers/net/wireless/ath/ath6kl-3.5/main.o
ath6kl_sdio-y += drivers/net/wireless/ath/ath6kl-3.5/txrx.o
ath6kl_sdio-y += drivers/net/wireless/ath/ath6kl-3.5/wmi.o
ath6kl_sdio-y += drivers/net/wireless/ath/ath6kl-3.5/wmi_btcoex.o
ath6kl_sdio-y += drivers/net/wireless/ath/ath6kl-3.5/sdio.o
ath6kl_sdio-y += drivers/net/wireless/ath/ath6kl-3.5/testmode.o
ath6kl_sdio-y += drivers/net/wireless/ath/ath6kl-3.5/rttm.o
ath6kl_sdio-y += drivers/net/wireless/ath/ath6kl-3.5/diag.o
ath6kl_sdio-y += drivers/net/wireless/ath/ath6kl-3.5/htcoex.o
else
obj-m := wlan.o

wlan-y += drivers/net/wireless/ath/ath6kl/debug.o
wlan-y += drivers/net/wireless/ath/ath6kl/debugfs_pri.o
wlan-y += drivers/net/wireless/ath/ath6kl/hif.o
wlan-y += drivers/net/wireless/ath/ath6kl/htc.o
wlan-y += drivers/net/wireless/ath/ath6kl/bmi.o
wlan-y += drivers/net/wireless/ath/ath6kl/cfg80211.o
wlan-y += drivers/net/wireless/ath/ath6kl/cfg80211_btcoex.o
wlan-y += drivers/net/wireless/ath/ath6kl/pm.o
wlan-y += drivers/net/wireless/ath/ath6kl/init.o
wlan-y += drivers/net/wireless/ath/ath6kl/main.o
wlan-y += drivers/net/wireless/ath/ath6kl/txrx.o
wlan-y += drivers/net/wireless/ath/ath6kl/wmi.o
wlan-y += drivers/net/wireless/ath/ath6kl/wmi_btcoex.o
wlan-y += drivers/net/wireless/ath/ath6kl/sdio.o
wlan-y += drivers/net/wireless/ath/ath6kl/msm.o
wlan-y += drivers/net/wireless/ath/ath6kl/softmac.o

wlan-y +=  drivers/net/wireless/ath/ath6kl/testmode.o
wlan-y += drivers/net/wireless/ath/ath6kl/wmiconfig.o
endif

obj-$(CONFIG_CFG80211) += cfg80211.o

cfg80211-y += net/wireless/core.o net/wireless/sysfs.o net/wireless/radiotap.o net/wireless/util.o net/wireless/reg.o net/wireless/scan.o net/wireless/nl80211.o
cfg80211-y += net/wireless/mlme.o net/wireless/ibss.o net/wireless/sme.o net/wireless/chan.o net/wireless/ethtool.o net/wireless/mesh.o
cfg80211-$(CONFIG_CFG80211_DEBUGFS) += net/wireless/debugfs.o
cfg80211-y += net/wireless/wext-compat.o net/wireless/wext-sme.o
cfg80211-y += net/wireless/regdb.o

$(obj)/net/wireless/regdb.c: net/wireless/db.txt net/wireless/genregdb.awk
	@$(AWK) -f $(PWD)/$(src)/net/wireless/genregdb.awk < $< > $@

clean-files := net/wireless/regdb.c

ccflags-y += -DCONFIG_ATH6KL_DEBUG
ccflags-y += -DCONFIG_NL80211_TESTMODE
ccflags-y += -DCONFIG_CFG80211_DEFAULT_PS
ccflags-y += -DCONFIG_CFG80211_WEXT
ccflags-y += -DCONFIG_CFG80211_INTERNAL_REGDB
ccflags-y += -D__CHECK_ENDIAN__

ccflags-y += -I../external/compat-wireless/include
ccflags-y += -include include/linux/ieee80211.h
ccflags-y += -include include/linux/nl80211.h
ccflags-y += -include include/net/cfg80211.h
ccflags-y += -include net/wireless/regdb.h
ccflags-y += -include include/linux/compat-2.6.h
ccflags-y += -include ../../$(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include/linux/version.h
