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

obj-$(CONFIG_CFG80211) += cfg80211.o

cfg80211-y += net/wireless/core.o net/wireless/sysfs.o net/wireless/radiotap.o net/wireless/util.o net/wireless/reg.o net/wireless/scan.o net/wireless/nl80211.o
cfg80211-y += net/wireless/mlme.o net/wireless/ibss.o net/wireless/sme.o net/wireless/chan.o net/wireless/ethtool.o net/wireless/mesh.o
cfg80211-$(CONFIG_CFG80211_DEBUGFS) += net/wireless/debugfs.o
cfg80211-y += net/wireless/wext-compat.o net/wireless/wext-sme.o
cfg80211-$(CONFIG_CFG80211_INTERNAL_REGDB) += net/wireless/regdb.o

$(obj)/net/wireless/regdb.c: $(src)/net/wireless/db.txt $(src)/net/wireless/genregdb.awk
	@$(AWK) -f $(src)/net/wireless/genregdb.awk < $< > $@

clean-files := net/wireless/regdb.c

ccflags-y += -DCONFIG_ATH6KL_DEBUG
ccflags-y += -DCONFIG_NL80211_TESTMODE
ccflags-y += -DCONFIG_CFG80211_DEFAULT_PS
ccflags-y += -DCONFIG_CFG80211_WEXT
ccflags-y += -D__CHECK_ENDIAN__

ccflags-y += -I../external/compat-wireless/include
ccflags-y += -include include/linux/ieee80211.h
ccflags-y += -include include/linux/nl80211.h
ccflags-y += -include include/net/cfg80211.h
ccflags-y += -include include/linux/compat-2.6.h
ccflags-y += -include ../../$(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include/linux/version.h
