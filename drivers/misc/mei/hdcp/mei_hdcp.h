/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright © 2019 Intel Corporation
 *
 * Authors:
 * Ramalingam C <ramalingam.c@intel.com>
 */

#ifndef __MEI_HDCP_H__
#define __MEI_HDCP_H__

#include <linux/version.h>
#include <osv_version.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(5,19,0) && !REDHAT_RELEASE_VERSION_GEQ(8,8,0)
#include <drm/drm_hdcp.h>
#else
#include <drm/display/drm_hdcp.h>
#endif

#include <drm/i915_cp_fw_hdcp_interface.h>

#endif /* __MEI_HDCP_H__ */
