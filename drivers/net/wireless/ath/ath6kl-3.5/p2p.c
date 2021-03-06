/*
 * Copyright (c) 2004-2012 Atheros Communications Inc.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include "core.h"
#include "htc-ops.h"
#include "debug.h"

struct p2p_ps_info *ath6kl_p2p_ps_init(struct ath6kl_vif *vif)
{
	struct p2p_ps_info *p2p_ps;

	p2p_ps = kzalloc(sizeof(struct p2p_ps_info), GFP_KERNEL);
	if (!p2p_ps) {
		ath6kl_err("failed to alloc memory for p2p_ps\n");
		return NULL;
	}

	p2p_ps->vif = vif;
	spin_lock_init(&p2p_ps->p2p_ps_lock);

	ath6kl_dbg(ATH6KL_DBG_POWERSAVE,
		   "p2p_ps init (vif %p) type %d\n",
		   vif,
		   vif->wdev.iftype);
		
	return p2p_ps;
}

void ath6kl_p2p_ps_deinit(struct ath6kl_vif *vif)
{
	struct p2p_ps_info *p2p_ps = vif->p2p_ps_info_ctx;

	if (p2p_ps) {
		spin_lock(&p2p_ps->p2p_ps_lock);
		if (p2p_ps->go_last_beacon_app_ie)
			kfree(p2p_ps->go_last_beacon_app_ie);

		if (p2p_ps->go_last_noa_ie)
			kfree(p2p_ps->go_last_noa_ie);

		if (p2p_ps->go_working_buffer)
			kfree(p2p_ps->go_working_buffer);
		spin_unlock(&p2p_ps->p2p_ps_lock);
		
		kfree(p2p_ps);
	}

	vif->p2p_ps_info_ctx = NULL;

	ath6kl_dbg(ATH6KL_DBG_POWERSAVE,
		   "p2p_ps deinit (vif %p)\n",
		   vif);
		
	return;
}

int ath6kl_p2p_ps_reset_noa(struct p2p_ps_info *p2p_ps)
{
	if ((!p2p_ps) || 
	    (p2p_ps->vif->wdev.iftype != NL80211_IFTYPE_P2P_GO)) {
	    ath6kl_err("failed to reset P2P-GO noa\n");
	    return -1;
	}

	ath6kl_dbg(ATH6KL_DBG_POWERSAVE,
		   "p2p_ps reset NoA (vif %p) index %d\n",
		   p2p_ps->vif,
		   p2p_ps->go_noa.index);

	spin_lock(&p2p_ps->p2p_ps_lock);
	p2p_ps->go_flags &= ~ATH6KL_P2P_PS_FLAGS_NOA_ENABLED;
	p2p_ps->go_noa.index++;
	p2p_ps->go_noa_enable_idx = 0;
	memset(p2p_ps->go_noa.noas,
		0,
		sizeof(struct ieee80211_p2p_noa_descriptor) * 
			ATH6KL_P2P_PS_MAX_NOA_DESCRIPTORS);
	spin_unlock(&p2p_ps->p2p_ps_lock);
	
	return 0;
}

int ath6kl_p2p_ps_setup_noa(struct p2p_ps_info *p2p_ps,
			    int noa_id,
			    u8 count_type,
			    u32 interval,
			    u32 start_offset,
			    u32 duration)
{
	struct ieee80211_p2p_noa_descriptor *noa_descriptor;
		
	if ((!p2p_ps) || 
	    (p2p_ps->vif->wdev.iftype != NL80211_IFTYPE_P2P_GO)) {
	    ath6kl_err("failed to setup P2P-GO noa\n");
	    return -1;
	}

	ath6kl_dbg(ATH6KL_DBG_POWERSAVE,
		   "p2p_ps setup NoA (vif %p) idx %d ct %d intval %x so %x dur %x\n",
		   p2p_ps->vif,
		   noa_id,
		   count_type,
		   interval,
		   start_offset,
		   duration);

	spin_lock(&p2p_ps->p2p_ps_lock);
	if (noa_id < ATH6KL_P2P_PS_MAX_NOA_DESCRIPTORS) {
		noa_descriptor = &p2p_ps->go_noa.noas[noa_id];

		noa_descriptor->count_or_type = count_type;
		noa_descriptor->interval = interval;
		noa_descriptor->start_or_offset = start_offset;
		noa_descriptor->duration = duration;
	} else {
		spin_unlock(&p2p_ps->p2p_ps_lock);
		ath6kl_err("wrong NoA index %d\n", noa_id);

		return -2;
	}

	p2p_ps->go_noa_enable_idx |= (1 << noa_id);
	p2p_ps->go_flags |= ATH6KL_P2P_PS_FLAGS_NOA_ENABLED;
	spin_unlock(&p2p_ps->p2p_ps_lock);

	return 0;
}

int ath6kl_p2p_ps_reset_opps(struct p2p_ps_info *p2p_ps)
{
	if ((!p2p_ps) || 
	    (p2p_ps->vif->wdev.iftype != NL80211_IFTYPE_P2P_GO)) {
	    ath6kl_err("failed to reset P2P-GO OppPS\n");
	    return -1;
	}

	ath6kl_dbg(ATH6KL_DBG_POWERSAVE,
		   "p2p_ps reset OppPS (vif %p) index %d\n",
		   p2p_ps->vif,
		   p2p_ps->go_noa.index);

	spin_lock(&p2p_ps->p2p_ps_lock);
	p2p_ps->go_flags &= ~ATH6KL_P2P_PS_FLAGS_OPPPS_ENABLED;
	p2p_ps->go_noa.index++;
	p2p_ps->go_noa.ctwindow_opps_param = 0;
	spin_unlock(&p2p_ps->p2p_ps_lock);
	
	return 0;
}

int ath6kl_p2p_ps_setup_opps(struct p2p_ps_info *p2p_ps, 
			     u8 enabled,
			     u8 ctwindows)
{
	if ((!p2p_ps) || 
	    (p2p_ps->vif->wdev.iftype != NL80211_IFTYPE_P2P_GO)) {
	    ath6kl_err("failed to setup P2P-GO noa\n");
	    return -1;
	}

	WARN_ON(enabled && (!(ctwindows & 0x7f)));

	ath6kl_dbg(ATH6KL_DBG_POWERSAVE,
		   "p2p_ps setup OppPS (vif %p) enabled %d ctwin %d\n",
		   p2p_ps->vif,
		   enabled,
		   ctwindows);

	spin_lock(&p2p_ps->p2p_ps_lock);
	if (enabled)
		p2p_ps->go_noa.ctwindow_opps_param = (0x80 | (ctwindows & 0x7f));
	else
		p2p_ps->go_noa.ctwindow_opps_param = 0;
	p2p_ps->go_flags |= ATH6KL_P2P_PS_FLAGS_OPPPS_ENABLED;
	spin_unlock(&p2p_ps->p2p_ps_lock);

	return 0;
}

int ath6kl_p2p_ps_update_notif(struct p2p_ps_info *p2p_ps)
{
	struct ath6kl_vif *vif;
	struct ieee80211_p2p_noa_ie *noa_ie;
	struct ieee80211_p2p_noa_descriptor *noa_descriptor;
	int i, idx, len, ret = 0;
	u8 *buf;

	WARN_ON(!p2p_ps);
	WARN_ON(!p2p_ps->go_last_beacon_app_ie_len);

	vif = p2p_ps->vif;

	/* 
	 * FIXME : No availabe NL80211 event to update to supplicant so far.
	 *         Instead, we try to set back to target here.
	 */
	spin_lock(&p2p_ps->p2p_ps_lock);
	if ((p2p_ps->go_flags & ATH6KL_P2P_PS_FLAGS_NOA_ENABLED) ||
	    (p2p_ps->go_flags & ATH6KL_P2P_PS_FLAGS_OPPPS_ENABLED)) {
		WARN_ON(((p2p_ps->go_flags & ATH6KL_P2P_PS_FLAGS_NOA_ENABLED) &&
			 (!p2p_ps->go_noa_enable_idx)));

		len = p2p_ps->go_last_beacon_app_ie_len +
		      sizeof(struct ieee80211_p2p_noa_ie);

		buf = kmalloc(len, GFP_ATOMIC);
		if (buf == NULL) {
			spin_unlock(&p2p_ps->p2p_ps_lock);

			return -ENOMEM;
		}

		/* Append NoA IE after user's IEs. */
		memcpy(buf, 
			p2p_ps->go_last_beacon_app_ie,
			p2p_ps->go_last_beacon_app_ie_len);

		noa_ie = (struct ieee80211_p2p_noa_ie *)(buf + p2p_ps->go_last_beacon_app_ie_len);
		noa_ie->element_id = WLAN_EID_VENDOR_SPECIFIC;
		noa_ie->oui = cpu_to_be32((WLAN_OUI_WFA << 8) | (WLAN_OUI_TYPE_WFA_P2P));
		noa_ie->attr = IEEE80211_P2P_ATTR_NOTICE_OF_ABSENCE;
		noa_ie->noa_info.index = p2p_ps->go_noa.index;
		noa_ie->noa_info.ctwindow_opps_param = p2p_ps->go_noa.ctwindow_opps_param;

		idx = 0;
		for (i = 0; i < ATH6KL_P2P_PS_MAX_NOA_DESCRIPTORS; i++) {
			if (p2p_ps->go_noa_enable_idx & (1 << i)) {
				noa_descriptor = &noa_ie->noa_info.noas[idx++];
				noa_descriptor->count_or_type = p2p_ps->go_noa.noas[i].count_or_type;
				noa_descriptor->duration = cpu_to_le32(p2p_ps->go_noa.noas[i].duration);
				noa_descriptor->interval = cpu_to_le32(p2p_ps->go_noa.noas[i].interval);
				noa_descriptor->start_or_offset = cpu_to_le32(p2p_ps->go_noa.noas[i].start_or_offset);
			}
				
		}
		
		/* Update length */
		noa_ie->attr_len = cpu_to_le16(2 + (sizeof(struct ieee80211_p2p_noa_descriptor) * idx));
		noa_ie->len = noa_ie->attr_len + 
			      4 + 1 + 2; /* OUI, attr, attr_len */
		len = p2p_ps->go_last_beacon_app_ie_len + (noa_ie->len + 2);

		/* Backup NoA IE for origional code path if need. */
		p2p_ps->go_last_noa_ie_len = 0;
		if (p2p_ps->go_last_noa_ie)
			kfree(p2p_ps->go_last_noa_ie);
		p2p_ps->go_last_noa_ie = kmalloc(noa_ie->len + 2, GFP_ATOMIC);
		if (p2p_ps->go_last_noa_ie) {
			p2p_ps->go_last_noa_ie_len = noa_ie->len + 2;
			memcpy(p2p_ps->go_last_noa_ie,
				noa_ie,
				p2p_ps->go_last_noa_ie_len);
		}

		spin_unlock(&p2p_ps->p2p_ps_lock);

		ath6kl_dbg(ATH6KL_DBG_POWERSAVE,
			   "p2p_ps update app IE (vif %p) flags %x idx %d noa_ie->len %d len %d\n",
			   vif,
			   p2p_ps->go_flags,
			   idx,
			   noa_ie->len,
			   len);
	} else {
		/* Remove NoA IE. */
		p2p_ps->go_last_noa_ie_len = 0;
		if (p2p_ps->go_last_noa_ie) {
			kfree(p2p_ps->go_last_noa_ie);
			p2p_ps->go_last_noa_ie = NULL;
		}

		buf = kmalloc(p2p_ps->go_last_beacon_app_ie_len, GFP_ATOMIC);
		if (buf == NULL) {
			spin_unlock(&p2p_ps->p2p_ps_lock);

			return -ENOMEM;
		}

		/* Back to origional Beacon IEs. */
		len = p2p_ps->go_last_beacon_app_ie_len;
		memcpy(buf, 
			p2p_ps->go_last_beacon_app_ie,
			len);

		spin_unlock(&p2p_ps->p2p_ps_lock);

		ath6kl_dbg(ATH6KL_DBG_POWERSAVE,
			   "p2p_ps update app IE (vif %p) flags %x beacon_ie %p len %d\n",
			   vif,
			   p2p_ps->go_flags,
			   p2p_ps->go_last_beacon_app_ie,
			   p2p_ps->go_last_beacon_app_ie_len);
	}

	if (down_interruptible(&vif->ar->sem)) {
		ath6kl_err("busy, couldn't get access\n");
		ret = -ERESTARTSYS;
		goto done;
	}

	/* 
	 * Only need to update Beacon's IE. The ProbeResp'q IE is settled 
	 * while sending. 
	 */	
	ret = ath6kl_wmi_set_appie_cmd(vif->ar->wmi, 
				       vif->fw_vif_idx,
				       WMI_FRAME_BEACON,
				       buf,
				       len);

	up(&vif->ar->sem);

done:
	kfree(buf);

	return ret;
}

/* 
 * FIXME : This's too bad solution to hook user's IEs then appended it in 
 *         next ath6kl_p2p_ps_update_notif() call.
 */
void ath6kl_p2p_ps_user_app_ie(struct p2p_ps_info *p2p_ps, 
	 		       u8 mgmt_frm_type,
	 		       u8 **ie, 
			       int *len)
{
	if ((!p2p_ps) || 
	    (p2p_ps->vif->wdev.iftype != NL80211_IFTYPE_P2P_GO)) {
	    ath6kl_err("Not need to hook user's app IE!\n");
	    return;
	}

	ath6kl_dbg(ATH6KL_DBG_POWERSAVE,
		   "p2p_ps hook app IE (vif %p) flags %x mgmt_frm_type %d len %d \n",
		   p2p_ps->vif,
		   p2p_ps->go_flags,
		   mgmt_frm_type,
		   *len);

	if (mgmt_frm_type == WMI_FRAME_BEACON) {
		WARN_ON((*len) == 0);
		
		spin_lock(&p2p_ps->p2p_ps_lock);
		p2p_ps->go_last_beacon_app_ie_len = 0;
		if (p2p_ps->go_last_beacon_app_ie)
			kfree(p2p_ps->go_last_beacon_app_ie);

		p2p_ps->go_last_beacon_app_ie = kmalloc(*len, GFP_ATOMIC);
		if (p2p_ps->go_last_beacon_app_ie == NULL) {
			spin_unlock(&p2p_ps->p2p_ps_lock);
			return;
		}

		/* Update to the latest one. */
		p2p_ps->go_last_beacon_app_ie_len = *len;
		memcpy(p2p_ps->go_last_beacon_app_ie, *ie, *len);

		spin_unlock(&p2p_ps->p2p_ps_lock);

		/* TODO : Need filter if the user's IEs include NoA or not? */

	} else if (mgmt_frm_type == WMI_FRAME_PROBE_RESP) {
		/* Assume non-zero means P2P node. */
		if ((*len) == 0) 
			return;
	}

	/* Hack : Change ie/len to let caller use the new one. */
	spin_lock(&p2p_ps->p2p_ps_lock);
	if ((p2p_ps->go_flags & ATH6KL_P2P_PS_FLAGS_NOA_ENABLED) ||
	    (p2p_ps->go_flags & ATH6KL_P2P_PS_FLAGS_OPPPS_ENABLED)){
#if 0
		/* Bypass it if don't care this. */
		ath6kl_err("this setting (frame type %d) will not include NoA IE!\n", 
	    			mgmt_frm_type);
#else
		/*
		 * Append the last NoA IE to *ie and also update *len to let caller
		 * use the new one.
		 */
		WARN_ON(!p2p_ps->go_last_noa_ie);

		if (p2p_ps->go_working_buffer)
			kfree(p2p_ps->go_working_buffer);
		p2p_ps->go_working_buffer = kmalloc((p2p_ps->go_last_noa_ie_len + *len), 
						      GFP_ATOMIC); 
		if (p2p_ps->go_working_buffer) {
			if (*len)
				memcpy(p2p_ps->go_working_buffer, *ie, *len);
			memcpy(p2p_ps->go_working_buffer + (*len),
			       p2p_ps->go_last_noa_ie, 
			       p2p_ps->go_last_noa_ie_len);

			if (mgmt_frm_type == WMI_FRAME_PROBE_RESP) {
				/* caller will release it. */
				kfree(*ie);
				*ie = p2p_ps->go_working_buffer;
				p2p_ps->go_working_buffer = NULL; 
			} else 
				*ie = p2p_ps->go_working_buffer;
			*len += p2p_ps->go_last_noa_ie_len;
		}

		ath6kl_dbg(ATH6KL_DBG_POWERSAVE,
			   "p2p_ps change app IE len -> %d\n",
			   *len);
#endif
	}
	spin_unlock(&p2p_ps->p2p_ps_lock);

	return;
}

int ath6kl_p2p_utils_trans_porttype(enum nl80211_iftype type, 
				    u8 *opmode, 
				    u8 *subopmode)
{	
	int ret = 0;
	
	switch(type) {   
	case NL80211_IFTYPE_STATION:
		*opmode = HI_OPTION_FW_MODE_BSS_STA;
		*subopmode = HI_OPTION_FW_SUBMODE_NONE;
		break;
	case NL80211_IFTYPE_P2P_DEVICE:
		*opmode = HI_OPTION_FW_MODE_BSS_STA;
		*subopmode = HI_OPTION_FW_SUBMODE_P2PDEV;
		break;
	case NL80211_IFTYPE_P2P_CLIENT:
		*opmode = HI_OPTION_FW_MODE_BSS_STA;
		*subopmode = HI_OPTION_FW_SUBMODE_P2PCLIENT;
		break;
	case NL80211_IFTYPE_P2P_GO:
		*opmode = HI_OPTION_FW_MODE_BSS_STA;
		*subopmode = HI_OPTION_FW_SUBMODE_P2PGO;
		break;
	case NL80211_IFTYPE_UNSPECIFIED:
	case NL80211_IFTYPE_ADHOC:
	case NL80211_IFTYPE_AP:
	case NL80211_IFTYPE_AP_VLAN:
	case NL80211_IFTYPE_WDS:
	case NL80211_IFTYPE_MONITOR:
	case NL80211_IFTYPE_MESH_POINT:
	default:
		ath6kl_err("error interface type %d\n", type);
		ret = -1;
		break;
	}

	return ret;
}

int ath6kl_p2p_utils_init_port(struct ath6kl_vif *vif,
			       enum nl80211_iftype type)
{
	struct ath6kl *ar = vif->ar;
	u8 fw_vif_idx = vif->fw_vif_idx;
	u8 opmode, subopmode;
	long left;

#if 0	/* TODO */
	ath6kl_info("ath6kl : don't care firmware port status, idx %d type %d\n", 
			vif->fw_vif_idx, type);
	return 0;
#endif

	/* 
	 * Only need to do this if virtual interface used but bypass vif_max=2 case.
	 * This case suppose be used only for special P2P purpose that is without 
	 * dedicated P2P-Device.
	 * 
         * 1st interface always be created by driver init phase and WMI interface 
         * not yet ready. Actually, we don't need to reset it because current design 
         * always STA interface in firmware and host sides.
	 */
	if ((ar->vif_max > 2) && fw_vif_idx) {
		/* 
		 * WAR : NL80211 no really have P2P_DEVICE type and current 
		 *       wpa_supplicant use P2P_CLIENT as interface type then 
		 *       transfer to P2P_GO/P2P_CLIENT after P2P connection done.
		 *       Current design always treat last virtual interface as 
		 *       dedicated P2P_DEVICE.
		 */
		if (fw_vif_idx == (ar->vif_max - 1))
			type = NL80211_IFTYPE_P2P_DEVICE;

		if (ath6kl_p2p_utils_trans_porttype(type, &opmode, &subopmode) == 0) {
			/* Delete it first. */
			if (type != NL80211_IFTYPE_P2P_DEVICE) {
				set_bit(PORT_STATUS_PEND, &vif->flags);
				if (ath6kl_wmi_del_port_cmd(ar->wmi, 
	 						    fw_vif_idx,
	 						    fw_vif_idx)) 
					return -EIO;

				left = wait_event_interruptible_timeout(ar->event_wq,
									!test_bit(PORT_STATUS_PEND,
										  &vif->flags),
									WMI_TIMEOUT/10);
				WARN_ON(left <= 0);
			}
			
			/* Only support exectly id-to-id mapping. */
			set_bit(PORT_STATUS_PEND, &vif->flags);
			if (ath6kl_wmi_add_port_cmd(ar->wmi,
						    vif,
						    opmode,
						    subopmode)) 
				return -EIO;

			left = wait_event_interruptible_timeout(ar->event_wq,
							 	!test_bit(PORT_STATUS_PEND,
									  &vif->flags),
								 WMI_TIMEOUT/10);
			WARN_ON(left <= 0);

		} else
			return -ENOTSUPP;
	}

	return 0;
}

int ath6kl_p2p_utils_check_port(struct ath6kl_vif *vif,
				u8 port_id)
{
	if (test_bit(PORT_STATUS_PEND, &vif->flags)) {
		WARN_ON(vif->fw_vif_idx != port_id);

		clear_bit(PORT_STATUS_PEND, &vif->flags);
		wake_up(&vif->ar->event_wq);
	}

	return 0;
}

struct ath6kl_p2p_flowctrl *ath6kl_p2p_flowctrl_conn_list_init(struct ath6kl *ar)
{
	struct ath6kl_p2p_flowctrl *p2p_flowctrl;
	struct ath6kl_fw_conn_list *fw_conn;
	int i;

	p2p_flowctrl = kzalloc(sizeof(struct ath6kl_p2p_flowctrl), GFP_KERNEL);
	if (!p2p_flowctrl) {
		ath6kl_err("failed to alloc memory for p2p_flowctrl\n");
		return NULL;
	}

	p2p_flowctrl->ar = ar;
	spin_lock_init(&p2p_flowctrl->p2p_flowctrl_lock);
	p2p_flowctrl->sche_type = P2P_FLOWCTRL_SCHE_TYPE_CONNECTION;

	for (i = 0; i < NUM_CONN; i++) {
	        fw_conn = &p2p_flowctrl->fw_conn_list[i];
	        INIT_LIST_HEAD(&fw_conn->conn_queue);
	        INIT_LIST_HEAD(&fw_conn->re_queue);
	        fw_conn->connect_status = 0;
	        fw_conn->previous_can_send = true;
		fw_conn->connId = ATH6KL_P2P_FLOWCTRL_NULL_CONNID;
		fw_conn->parent_connId = ATH6KL_P2P_FLOWCTRL_NULL_CONNID;
		memset(fw_conn->mac_addr, 0, ETH_ALEN);

		fw_conn->sche_tx = 0;
		fw_conn->sche_re_tx = 0;
		fw_conn->sche_re_tx_aging = 0;
		fw_conn->sche_tx_queued = 0;
	}
	
	ath6kl_dbg(ATH6KL_DBG_FLOWCTRL,
		   "p2p_flowctrl init (ar %p) NUM_CONN %d type %d\n",
		   ar,
		   NUM_CONN,
		   p2p_flowctrl->sche_type);
		
	return p2p_flowctrl;
}

void ath6kl_p2p_flowctrl_conn_list_deinit(struct ath6kl *ar)
{
	struct ath6kl_p2p_flowctrl *p2p_flowctrl = ar->p2p_flowctrl_ctx;
	struct ath6kl_fw_conn_list *fw_conn;
	int i;
	
	if (p2p_flowctrl) {
		/*
		 *  TODO : It's better to check whether any conn_queue/re_queue 
		 *         need to reclaim.
		 */

		/* check memory leakage */
		for (i = 0; i < NUM_CONN; i++) {
			spin_lock_bh(&p2p_flowctrl->p2p_flowctrl_lock);
			fw_conn = &p2p_flowctrl->fw_conn_list[i];
			if (fw_conn->sche_tx_queued != 0) {
				ath6kl_err("memory leakage ? [%d] tx %d re_tx %d re_tx_aging %d tx_queued %d\n",
						i,
						fw_conn->sche_tx,
						fw_conn->sche_re_tx,
						fw_conn->sche_re_tx_aging,
						fw_conn->sche_tx_queued);
				WARN_ON(1);
			}
			spin_unlock_bh(&p2p_flowctrl->p2p_flowctrl_lock);
		}

		kfree(p2p_flowctrl);
	}

	ar->p2p_flowctrl_ctx = NULL;

	ath6kl_dbg(ATH6KL_DBG_FLOWCTRL,
		   "p2p_flowctrl deinit (ar %p)\n",
		   ar);
		
	return;
}

void ath6kl_p2p_flowctrl_conn_list_cleanup(struct ath6kl *ar)
{
	struct ath6kl_p2p_flowctrl *p2p_flowctrl = ar->p2p_flowctrl_ctx;
	struct ath6kl_fw_conn_list *fw_conn;
	struct htc_packet *packet, *tmp_pkt;
	struct list_head container;
	int i, reclaim = 0;

	WARN_ON(!p2p_flowctrl);

	INIT_LIST_HEAD(&container);

	for (i = 0; i < NUM_CONN; i++) {
		spin_lock_bh(&p2p_flowctrl->p2p_flowctrl_lock);
		fw_conn = &p2p_flowctrl->fw_conn_list[i];
		if (!list_empty(&fw_conn->re_queue)) {
			list_for_each_entry_safe(packet, tmp_pkt, &fw_conn->re_queue, list) {
				list_del(&packet->list);
				packet->status = 0;
				list_add_tail(&packet->list, &container);
				fw_conn->sche_tx_queued--;
				reclaim++;
			}
		}

		if (!list_empty(&fw_conn->conn_queue)) {
			list_for_each_entry_safe(packet, tmp_pkt, &fw_conn->conn_queue, list) {
				list_del(&packet->list);
				packet->status = 0;
				list_add_tail(&packet->list, &container);
				fw_conn->sche_tx_queued--;
				reclaim++;
			}
		}
		spin_unlock_bh(&p2p_flowctrl->p2p_flowctrl_lock);
	}

	ath6kl_tx_complete(ar->htc_target, &container);

	ath6kl_dbg(ATH6KL_DBG_FLOWCTRL,
		   "p2p_flowctrl cleanup (ar %p) reclaim %d\n",
		   ar,
		   reclaim);

	return;
}

static inline bool _check_can_send(struct ath6kl_fw_conn_list *fw_conn)
{
        bool can_send = false;

        do {
                if (fw_conn->ocs)
                        break;

                can_send = true;
        } while(false);

        return can_send;
}

void ath6kl_p2p_flowctrl_tx_schedule(struct ath6kl *ar)
{
	struct ath6kl_p2p_flowctrl *p2p_flowctrl = ar->p2p_flowctrl_ctx;
	struct ath6kl_fw_conn_list *fw_conn;
	struct htc_packet *packet, *tmp_pkt;
	int i;

	WARN_ON(!p2p_flowctrl);

	for (i = 0; i < NUM_CONN; i++) {
		spin_lock_bh(&p2p_flowctrl->p2p_flowctrl_lock);

		fw_conn = &p2p_flowctrl->fw_conn_list[i];
		/* Bypass this fw_conn if it not yet used. */
		if (fw_conn->connId == ATH6KL_P2P_FLOWCTRL_NULL_CONNID) {
			spin_unlock_bh(&p2p_flowctrl->p2p_flowctrl_lock);
			continue;
		}
			
		if (_check_can_send(fw_conn)) { 
			if (!list_empty(&fw_conn->re_queue)) {
				list_for_each_entry_safe(packet, tmp_pkt, &fw_conn->re_queue, list) {
					list_del(&packet->list);
					if (packet == NULL) 
						continue;

					if (packet->endpoint >= ENDPOINT_MAX) 
						continue;

					fw_conn->sche_re_tx--;
					fw_conn->sche_tx_queued--;

					ath6kl_htc_tx(ar->htc_target, packet);
				}
			}

			if (!list_empty(&fw_conn->conn_queue)) {
					list_for_each_entry_safe(packet, tmp_pkt, &fw_conn->conn_queue, list) {
					list_del(&packet->list);
					if (packet == NULL)
						continue;

					if (packet->endpoint >= ENDPOINT_MAX)
						continue;

					fw_conn->sche_tx++;
					fw_conn->sche_tx_queued--;

					ath6kl_htc_tx(ar->htc_target, packet);
				}
			}
		}
		spin_unlock_bh(&p2p_flowctrl->p2p_flowctrl_lock);

		ath6kl_dbg(ATH6KL_DBG_FLOWCTRL,
			   "p2p_flowctrl schedule (ar %p) connId %d tx %d re_tx %d\n",
			   ar,
			   i,
			   fw_conn->sche_tx,
			   fw_conn->sche_re_tx);
	}

	return;
}

int ath6kl_p2p_flowctrl_tx_schedule_pkt(struct ath6kl *ar,
				        void *pkt)
{
	struct ath6kl_p2p_flowctrl *p2p_flowctrl = ar->p2p_flowctrl_ctx;
	struct ath6kl_fw_conn_list *fw_conn;
	struct ath6kl_cookie *cookie = (struct ath6kl_cookie *)pkt;
	int connId = cookie->htc_pkt.connid;
	int ret = 0;

	WARN_ON(!p2p_flowctrl);

	if (connId == ATH6KL_P2P_FLOWCTRL_NULL_CONNID) {
		ath6kl_err("p2p_flowctrl tx schedule packet fail, NULL connId, just send??\n");

		return 1;	/* Just send it */
		//return -1;	/* Drop it */
	}

	spin_lock_bh(&p2p_flowctrl->p2p_flowctrl_lock);
	fw_conn = &p2p_flowctrl->fw_conn_list[connId];
	if (!_check_can_send(fw_conn)) {
		list_add_tail(&cookie->htc_pkt.list, &fw_conn->conn_queue);
		fw_conn->sche_tx_queued++;
		spin_unlock_bh(&p2p_flowctrl->p2p_flowctrl_lock);

		goto result;
	} else if (!list_empty(&fw_conn->conn_queue)) {
		list_add_tail(&cookie->htc_pkt.list, &fw_conn->conn_queue);
		fw_conn->sche_tx_queued++;
		spin_unlock_bh(&p2p_flowctrl->p2p_flowctrl_lock);

		ath6kl_p2p_flowctrl_tx_schedule(ar);

		goto result;
	} else
		ret = 1;
	spin_unlock_bh(&p2p_flowctrl->p2p_flowctrl_lock);

result:
	ath6kl_dbg(ATH6KL_DBG_FLOWCTRL,
		   "p2p_flowctrl schedule pkt (ar %p) %s\n",
		   ar,
		   ((ret == 0) ? "queue" : "send"));

	return ret;
}

void ath6kl_p2p_flowctrl_state_change(struct ath6kl *ar)
{
	struct ath6kl_p2p_flowctrl *p2p_flowctrl = ar->p2p_flowctrl_ctx;
	struct ath6kl_fw_conn_list *fw_conn;
	struct htc_packet *packet, *tmp_pkt;
	struct htc_endpoint *endpoint;
	struct list_head    *tx_queue, container;
	int i, eid;

	WARN_ON(!p2p_flowctrl);

	INIT_LIST_HEAD(&container);

	for (i = 0; i < NUM_CONN; i++) {
		/* TODO : It's better to check whether this fw_conn already be used or not 
		 *        to bypass it to reduce CPU time.
		 *
		 *        Now, just push this checking to ath6kl_p2p_flowctrl_tx_schedule()
		 *        to avoid previous_can_send not to be updated.
		 */
		spin_lock_bh(&p2p_flowctrl->p2p_flowctrl_lock);
		fw_conn = &p2p_flowctrl->fw_conn_list[i];
		if (!_check_can_send(fw_conn) && fw_conn->previous_can_send) {	
			spin_lock_bh(&ar->htc_target->tx_lock);	
			for (eid = ENDPOINT_2; eid <= ENDPOINT_5; eid++) {
				endpoint = &ar->htc_target->endpoint[eid];
				tx_queue = &endpoint->txq;
				if (list_empty(tx_queue)) 
					continue;

				list_for_each_entry_safe(packet, tmp_pkt, tx_queue, list) {
					if (packet->connid != i) 
						continue;
					
					list_del(&packet->list);
					if (packet->recycle_count > ATH6KL_P2P_FLOWCTRL_RECYCLE_LIMIT) {
						ath6kl_info("recycle packet exceeded limitation\n");
						packet->status = 0;
						list_add_tail(&packet->list, &container);

						fw_conn->sche_re_tx_aging++;
						fw_conn->sche_tx_queued--;
					} else {
						packet->recycle_count++;
						list_add_tail(&packet->list, &fw_conn->re_queue);

						fw_conn->sche_re_tx++;
						fw_conn->sche_tx_queued++;
					}
				}
			}
			spin_unlock_bh(&ar->htc_target->tx_lock);
		}
		fw_conn->previous_can_send = _check_can_send(fw_conn);
		spin_unlock_bh(&p2p_flowctrl->p2p_flowctrl_lock);
	}

	ath6kl_dbg(ATH6KL_DBG_FLOWCTRL,
		   "p2p_flowctrl state_change (ar %p) re_tx %d re_tx_aging %d \n",
		   ar,
		   fw_conn->sche_re_tx,
		   fw_conn->sche_re_tx_aging);

	ath6kl_tx_complete(ar->htc_target, &container);

	return;
}

void ath6kl_p2p_flowctrl_state_update(struct ath6kl *ar,
				      u8 numConn,
				      u8 ac_map[],
				      u8 ac_queue_depth[])
{
	struct ath6kl_p2p_flowctrl *p2p_flowctrl = ar->p2p_flowctrl_ctx;
	struct ath6kl_fw_conn_list *fw_conn;
	int i; 

	WARN_ON(!p2p_flowctrl);
	WARN_ON(numConn > NUM_CONN);

	spin_lock_bh(&p2p_flowctrl->p2p_flowctrl_lock);
	for (i = 0; i < numConn; i++) {
		fw_conn = &p2p_flowctrl->fw_conn_list[i];		
		fw_conn->connect_status = ac_map[i];

		/* TODO : ac_queue_depth? */
		
	}
	spin_unlock_bh(&p2p_flowctrl->p2p_flowctrl_lock);

	ath6kl_dbg(ATH6KL_DBG_FLOWCTRL,
		   "p2p_flowctrl state_update (ar %p) ac_map %02x %02x %02x %02x\n",
		   ar,
		   ac_map[0], ac_map[1], ac_map[2], ac_map[3]);
	
	return;
}

static u8 _find_parent_conn_id(struct ath6kl_p2p_flowctrl *p2p_flowctrl, 
				struct ath6kl_vif *hint_vif)
{
	struct ath6kl_fw_conn_list *fw_conn;
	u8 connId = ATH6KL_P2P_FLOWCTRL_NULL_CONNID;
	int i;

	/* Need protected in p2p_flowctrl_lock by caller. */
	fw_conn = &p2p_flowctrl->fw_conn_list[0];
	for (i = 0; i < NUM_CONN; i++, fw_conn++) {
		if (fw_conn->connId == ATH6KL_P2P_FLOWCTRL_NULL_CONNID)
			continue;

		if ((fw_conn->vif == hint_vif) &&
		    (fw_conn->parent_connId == fw_conn->connId)){
			connId = fw_conn->connId;
			break;
		}
	}

	WARN_ON(connId == ATH6KL_P2P_FLOWCTRL_NULL_CONNID);

	return connId;
}

void ath6kl_p2p_flowctrl_set_conn_id(struct ath6kl_vif *vif, 
				     u8 mac_addr[],
				     u8 connId)
{
	struct ath6kl *ar = vif->ar;
	struct ath6kl_p2p_flowctrl *p2p_flowctrl = ar->p2p_flowctrl_ctx;
	struct ath6kl_fw_conn_list *fw_conn;

	WARN_ON(!p2p_flowctrl);

	/* FIXME : Call this API w/ NULL mac address if DISCONNECT event. */

	spin_lock_bh(&p2p_flowctrl->p2p_flowctrl_lock);
	fw_conn = &p2p_flowctrl->fw_conn_list[connId];
	if (mac_addr) {
		if (fw_conn->sche_tx_queued != 0) {
			ath6kl_err("memory leakage ? [%d/%02x] tx %d re_tx %d re_tx_aging %d tx_queued %d\n",
					fw_conn->connId,
					fw_conn->connect_status,
					fw_conn->sche_tx,
					fw_conn->sche_re_tx,
					fw_conn->sche_re_tx_aging,
					fw_conn->sche_tx_queued);
			WARN_ON(1);
		}

		fw_conn->vif = vif;
		fw_conn->connId = connId;
		memcpy(fw_conn->mac_addr, mac_addr, ETH_ALEN);

		/* 
		 * Parent's connId of P2P-GO/P2P-Client/STA is self.
		 * Parent's connId of P2P-GO's Clients is P2P-GO.
		 */
		if ((vif->nw_type == AP_NETWORK) &&
 		    (memcmp(vif->ndev->dev_addr, mac_addr, ETH_ALEN))) {
	    		/* P2P-GO's Client connection event. */
			fw_conn->parent_connId = _find_parent_conn_id(p2p_flowctrl, vif);
		} else {
			/* P2P-GO/P2P-Client/STA connection event. */
			fw_conn->parent_connId = fw_conn->connId;
		}
	} else {
		/* FIXME : Recycle the conn_queue/re_queue here. */

		fw_conn->connId = ATH6KL_P2P_FLOWCTRL_NULL_CONNID;
		fw_conn->parent_connId = ATH6KL_P2P_FLOWCTRL_NULL_CONNID;
		fw_conn->connect_status = 0;
		fw_conn->previous_can_send = true;
		memset(fw_conn->mac_addr, 0, ETH_ALEN);
	}

	fw_conn->sche_tx = 0;
	fw_conn->sche_re_tx = 0;
	fw_conn->sche_re_tx_aging = 0;
	fw_conn->sche_tx_queued = 0;
	spin_unlock_bh(&p2p_flowctrl->p2p_flowctrl_lock);

	ath6kl_dbg(ATH6KL_DBG_FLOWCTRL,
		   "p2p_flowctrl set conn_id (ar %p) mode %d connId %d parent_connId %d mac_addr %02x:%02x:%02x:%02x:%02x:%02x\n",
		   ar,
		   vif->nw_type,
		   connId,
		   fw_conn->parent_connId,
		   mac_addr[0], mac_addr[1], mac_addr[2],
		   mac_addr[3], mac_addr[4], mac_addr[5]);
	
	return;
}

u8 ath6kl_p2p_flowctrl_get_conn_id(struct ath6kl_vif *vif, 
				   struct sk_buff *skb)
{
	struct ath6kl *ar = vif->ar;
	struct ath6kl_p2p_flowctrl *p2p_flowctrl = ar->p2p_flowctrl_ctx;
	struct ath6kl_fw_conn_list *fw_conn;
	struct ethhdr *ethhdr;
	u8 *hint;
	u8 connId = ATH6KL_P2P_FLOWCTRL_NULL_CONNID;
	int i;

	if (!p2p_flowctrl) 
		return connId;

	ethhdr = (struct ethhdr *)(skb->data + sizeof(struct wmi_data_hdr));


	if (vif->nw_type != AP_NETWORK)
		hint = ethhdr->h_source;
	else {
		if (is_multicast_ether_addr(ethhdr->h_dest))
			hint = ethhdr->h_source;
		else
			hint = ethhdr->h_dest;
	}

	spin_lock_bh(&p2p_flowctrl->p2p_flowctrl_lock);
	fw_conn = &p2p_flowctrl->fw_conn_list[0];
	for (i = 0; i < NUM_CONN; i++, fw_conn++) {
		if (fw_conn->connId == ATH6KL_P2P_FLOWCTRL_NULL_CONNID)
			continue;

		if (memcmp(fw_conn->mac_addr, hint, ETH_ALEN) == 0) {
			connId = fw_conn->connId;
			break;
		}
	}

	/* Change to parent's. */
	if (p2p_flowctrl->sche_type == P2P_FLOWCTRL_SCHE_TYPE_INTERFACE)
		connId = fw_conn->parent_connId;
	spin_unlock_bh(&p2p_flowctrl->p2p_flowctrl_lock);

	ath6kl_dbg(ATH6KL_DBG_FLOWCTRL,
		   "p2p_flowctrl get conn_id (ar %p) connId %d hint %02x:%02x:%02x:%02x:%02x:%02x\n",
		   ar,
		   connId,
		   hint[0], hint[1], hint[2], hint[3], hint[4], hint[5]);
	
	return connId;
}

int ath6kl_p2p_flowctrl_stat(struct ath6kl *ar,
			     u8 *buf, int buf_len)
{
	struct ath6kl_p2p_flowctrl *p2p_flowctrl = ar->p2p_flowctrl_ctx;
	struct ath6kl_fw_conn_list *fw_conn;
	int i, len = 0;

	if ((!p2p_flowctrl) || (!buf))
		return 0;

	len += snprintf(buf + len, buf_len - len, " \n NUM_CONN : %d", NUM_CONN);
	len += snprintf(buf + len, buf_len - len, " \n SCHE_TYPE : %s\n", 
					(p2p_flowctrl->sche_type == P2P_FLOWCTRL_SCHE_TYPE_CONNECTION ?
					 "CONNECTION" : "INTERFACE"));

	spin_lock_bh(&p2p_flowctrl->p2p_flowctrl_lock);
	for (i = 0; i < NUM_CONN; i++) {
		fw_conn = &p2p_flowctrl->fw_conn_list[i];

		if (fw_conn->connId == ATH6KL_P2P_FLOWCTRL_NULL_CONNID)
			continue;

		len += snprintf(buf + len, buf_len - len, "\n[%d]===============================\n", i);
		len += snprintf(buf + len, buf_len - len, " vif          : %p\n", fw_conn->vif);
		len += snprintf(buf + len, buf_len - len, " connId       : %d\n", fw_conn->connId);
		len += snprintf(buf + len, buf_len - len, " parent_connId: %d\n", fw_conn->parent_connId);
		len += snprintf(buf + len, buf_len - len, " macAddr      : %02x:%02x:%02x:%02x:%02x:%02x\n", 
				fw_conn->mac_addr[0], fw_conn->mac_addr[1], fw_conn->mac_addr[2], 
				fw_conn->mac_addr[3], fw_conn->mac_addr[4], fw_conn->mac_addr[5]);
		len += snprintf(buf + len, buf_len - len, " status       : %02x\n", fw_conn->connect_status);
		len += snprintf(buf + len, buf_len - len, " preCanSend   : %d\n", fw_conn->previous_can_send);
		len += snprintf(buf + len, buf_len - len, " tx_queued    : %d\n", fw_conn->sche_tx_queued);
		len += snprintf(buf + len, buf_len - len, " tx           : %d\n", fw_conn->sche_tx);
		len += snprintf(buf + len, buf_len - len, " rx_tx        : %d\n", fw_conn->sche_re_tx);
		len += snprintf(buf + len, buf_len - len, " re_tx_aging  : %d\n", fw_conn->sche_re_tx_aging);
	}
	spin_unlock_bh(&p2p_flowctrl->p2p_flowctrl_lock);

	return len;
}

