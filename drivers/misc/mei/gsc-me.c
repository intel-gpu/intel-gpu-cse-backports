// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright(c) 2019-2020, Intel Corporation. All rights reserved.
 *
 * Intel Management Engine Interface (Intel MEI) Linux driver
 */

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/device.h>
#include <linux/irqreturn.h>
#include <linux/jiffies.h>
#include <linux/ktime.h>
#include <linux/delay.h>
#include <linux/pm_runtime.h>
#include <linux/kthread.h>

#include "mei_dev.h"
#include "hw-me.h"
#include "hw-me-regs.h"

#include "mei-trace.h"

#define MEI_GSC_RPM_TIMEOUT 500

static inline bool mei_gsc_hw_is_unavailable(const struct device *dev)
{
	return dev->offline;
}

static int mei_gsc_read_hfs(const struct mei_device *dev, int where, u32 *val)
{
	struct mei_me_hw *hw = to_me_hw(dev);

	*val = ioread32(hw->mem_addr + where + 0xC00);

	return 0;
}

static void mei_gsc_set_ext_op_mem(const struct mei_me_hw *hw, struct resource *mem)
{
	u32 low = lower_32_bits(mem->start);
	u32 hi  = upper_32_bits(mem->start);
	u32 limit = (resource_size(mem) / SZ_4K) | GSC_EXT_OP_MEM_VALID;

	iowrite32(low, hw->mem_addr + H_GSC_EXT_OP_MEM_BASE_ADDR_LO_REG);
	iowrite32(hi, hw->mem_addr + H_GSC_EXT_OP_MEM_BASE_ADDR_HI_REG);
	iowrite32(limit, hw->mem_addr + H_GSC_EXT_OP_MEM_LIMIT_REG);
}

/**
 * mei_gsc_mecbrw_read_null - read 32bit data from ME circular buffer (empty implementation)
 *  read window register
 *
 * @dev: the device structure
 *
 * Return: always 0
 */
static inline u32 mei_gsc_mecbrw_read_null(const struct mei_device *dev)
{
	return 0;
}

/**
 * mei_gsc_trc_status_null - read trc status register (empty implementation)
 *
 * @dev: mei device
 * @trc: trc status register value
 *
 * Return: always 0
 */
static int mei_gsc_trc_status_null(struct mei_device *dev, u32 *trc)
{
	*trc = 0;
	return 0;
}

/**
 * mei_gsc_fw_status_null - read fw status register from pci config space (empty implementation)
 *
 * @dev: mei device
 * @fw_status: fw status register values
 *
 * Return: always 0
 */
static int mei_gsc_fw_status_null(struct mei_device *dev,
				  struct mei_fw_status *fw_status)
{
	return 0;
}

/**
 * mei_gsc_hw_config_null - configure hw dependent settings (empty implementation)
 *
 * @dev: mei device
 *
 * Return: always 0
 *
 */
static int mei_gsc_hw_config_null(struct mei_device *dev)
{
	return 0;
}

/**
 * mei_gsc_pg_state_null  - translate internal pg state (empty implementation)
 *   to the mei power gating state
 *
 * @dev:  mei device
 *
 * Return: always MEI_PG_OFF
 */
static inline enum mei_pg_state mei_gsc_pg_state_null(struct mei_device *dev)
{
	return MEI_PG_OFF;
}

/**
 * mei_gsc_intr_clear_null - clear and stop interrupts (empty implementation)
 *
 * @dev: the device structure
 */
static void mei_gsc_intr_clear_null(struct mei_device *dev)
{
}

/**
 * mei_gsc_intr_enable_null - enables mei device interrupts (empty implementation)
 *
 * @dev: the device structure
 */
static void mei_gsc_intr_enable_null(struct mei_device *dev)
{
}

/**
 * mei_gsc_intr_disable_null - disables mei device interrupts (empty implementation)
 *
 * @dev: the device structure
 */
static void mei_gsc_intr_disable_null(struct mei_device *dev)
{
}

/**
 * mei_gsc_synchronize_irq_null - wait for pending IRQ handlers (empty implementation)
 *
 * @dev: the device structure
 */
static void mei_gsc_synchronize_irq_null(struct mei_device *dev)
{
}

/**
 * mei_gsc_host_is_ready_null - check whether the host has turned ready (empty implementation)
 *
 * @dev: mei device
 * Return: always true
 */
static bool mei_gsc_host_is_ready_null(struct mei_device *dev)
{
	return true;
}

/**
 * mei_gsc_hw_start_null - hw start routine (empty implementation)
 *
 * @dev: mei device
 * Return: always 0
 */
static int mei_gsc_hw_start_null(struct mei_device *dev)
{
	return 0;
}

/**
 * mei_gsc_hbuf_is_empty_null - checks if host buffer is empty (empty implementation)
 *
 * @dev: the device structure
 *
 * Return: always true
 */
static bool mei_gsc_hbuf_is_empty_null(struct mei_device *dev)
{
	return true;
}

/**
 * mei_gsc_hbuf_empty_slots_null - counts write empty slots (empty implementation)
 *
 * @dev: the device structure
 *
 * Return: always -EOVERFLOW
 */
static int mei_gsc_hbuf_empty_slots_null(struct mei_device *dev)
{
	return -EOVERFLOW;
}

/**
 * mei_gsc_hbuf_depth_null - returns depth of the hw buffer (empty implementation)
 *
 * @dev: the device structure
 *
 * Return: always 1
 */
static u32 mei_gsc_hbuf_depth_null(const struct mei_device *dev)
{
	return 0;
}

/**
 * mei_gsc_hbuf_write_null - writes a message to host hw buffer (empty implementation)
 *
 * @dev: the device structure
 * @hdr: header of message
 * @hdr_len: header length in bytes: must be multiplication of a slot (4bytes)
 * @data: payload
 * @data_len: payload length in bytes
 *
 * Return: always 0
 */
static int mei_gsc_hbuf_write_null(struct mei_device *dev,
				   const void *hdr, size_t hdr_len,
				   const void *data, size_t data_len)
{
	return 0;
}

/**
 * mei_gsc_count_full_read_slots_null - counts read full slots (empty implementation)
 *
 * @dev: the device structure
 *
 * Return: always -EOVERFLOW
 */
static int mei_gsc_count_full_read_slots_null(struct mei_device *dev)
{
	return -EOVERFLOW;
}

/**
 * mei_gsc_read_slots_null - reads a message from mei device (empty implementation)
 *
 * @dev: the device structure
 * @buffer: message buffer will be written
 * @buffer_length: message size will be read
 *
 * Return: always 0
 */
static int mei_gsc_read_slots_null(struct mei_device *dev, unsigned char *buffer,
				   unsigned long buffer_length)
{
	return 0;
}

/**
 * mei_gsc_pg_in_transition_null - is device now in pg transition (empty implementation)
 *
 * @dev: the device structure
 *
 * Return: always false
 */
static bool mei_gsc_pg_in_transition_null(struct mei_device *dev)
{
	return false;
}

/**
 * mei_gsc_pg_is_enabled_null - detect if PG is supported by HW (empty implementation)
 *
 * @dev: the device structure
 *
 * Return: always false
 */
static bool mei_gsc_pg_is_enabled_null(struct mei_device *dev)
{
	return false;
}

/**
 * mei_gsc_hw_is_ready_null - check whether the me(hw) has turned ready (empty implementation)
 *
 * @dev: mei device
 * Return: always true
 */
static bool mei_gsc_hw_is_ready_null(struct mei_device *dev)
{
	return true;
}

/**
 * mei_gsc_hw_reset_null - resets fw via mei csr register (empty implementation)
 *
 * @dev: the device structure
 * @intr_enable: if interrupt should be enabled after reset.
 *
 * Return: always 0
 */
static int mei_gsc_hw_reset_null(struct mei_device *dev, bool intr_enable)
{
	return 0;
}

static const struct mei_hw_ops mei_gsc_hw_ops_null = {
	.trc_status = mei_gsc_trc_status_null,
	.fw_status = mei_gsc_fw_status_null,
	.pg_state  = mei_gsc_pg_state_null,

	.host_is_ready = mei_gsc_host_is_ready_null,

	.hw_is_ready = mei_gsc_hw_is_ready_null,
	.hw_reset = mei_gsc_hw_reset_null,
	.hw_config = mei_gsc_hw_config_null,
	.hw_start = mei_gsc_hw_start_null,

	.pg_in_transition = mei_gsc_pg_in_transition_null,
	.pg_is_enabled = mei_gsc_pg_is_enabled_null,

	.intr_clear = mei_gsc_intr_clear_null,
	.intr_enable = mei_gsc_intr_enable_null,
	.intr_disable = mei_gsc_intr_disable_null,
	.synchronize_irq = mei_gsc_synchronize_irq_null,

	.hbuf_free_slots = mei_gsc_hbuf_empty_slots_null,
	.hbuf_is_ready = mei_gsc_hbuf_is_empty_null,
	.hbuf_depth = mei_gsc_hbuf_depth_null,

	.write = mei_gsc_hbuf_write_null,

	.rdbuf_full_slots = mei_gsc_count_full_read_slots_null,
	.read_hdr = mei_gsc_mecbrw_read_null,
	.read = mei_gsc_read_slots_null
};

static int mei_gsc_probe(struct platform_device *platdev)
{
	struct mei_device *dev;
	struct mei_me_hw *hw;
	struct resource *bar;
	struct resource *ext_op_mem = NULL;
	struct device *device;
	const struct platform_device_id *ent;
	const struct mei_cfg *cfg;
	int ret;
	bool is_gsc_pxp = false;
	int irq;

	ent = platform_get_device_id(platdev);
	cfg = mei_me_get_cfg(ent->driver_data);
	if (!cfg)
		return -ENODEV;

	device = &platdev->dev;

	irq = platform_get_irq_optional(platdev, 0);

	dev = mei_me_dev_init(device, cfg, (irq < 0));
	if (!dev) {
		ret = -ENOMEM;
		goto err;
	}

	bar = platform_get_resource(platdev, IORESOURCE_MEM, 0);

	hw = to_me_hw(dev);

	hw->mem_addr = devm_ioremap_resource(device, bar);
	if (IS_ERR(hw->mem_addr)) {
		dev_err(device, "mmio not mapped\n");
		ret = PTR_ERR(hw->mem_addr);
		goto err;
	}

	hw->irq = irq;

	hw->read_fws = mei_gsc_read_hfs;

	ext_op_mem = (struct resource *)platdev->dev.platform_data;
	is_gsc_pxp = strcmp(cfg->kind, "gsc") == 0 && ext_op_mem;
	if (is_gsc_pxp) {
		mei_gsc_set_ext_op_mem(hw, ext_op_mem);
		dev->pxp_mode = MEI_DEV_PXP_INIT;
	}

	platform_set_drvdata(platdev, dev);

	/* use polling */
	if (mei_me_hw_use_polling(hw)) {
		mei_disable_interrupts(dev);
		mei_clear_interrupts(dev);
		init_waitqueue_head(&hw->wait_active);
		hw->is_active = true; /* start in active mode for initialization */
		hw->polling_thread = kthread_run(mei_me_polling_thread, dev,
						 "kmegscirqd/%s", dev_name(device));
		if (IS_ERR(hw->polling_thread)) {
			ret = PTR_ERR(hw->polling_thread);
			dev_err(device, "unable to create kernel thread: %d\n", ret);
			goto err;
		}
	} else {
		ret = devm_request_threaded_irq(device, hw->irq,
						mei_me_irq_quick_handler,
						mei_me_irq_thread_handler,
						IRQF_ONESHOT, KBUILD_MODNAME, dev);
		if (ret) {
			dev_err(device, "irq register failed %d\n", ret);
			goto err;
		}
	}

	pm_runtime_get_noresume(device);
	pm_runtime_set_active(device);
	pm_runtime_enable(device);

	/* Continue to char device setup in spite of firmware handshake failure.
	 * In order to provide access to the firmware status registers to the user
	 * space via sysfs.
	 */
	if (mei_start(dev))
		dev_warn(device, "init hw failure.\n");

	pm_runtime_set_autosuspend_delay(device, MEI_GSC_RPM_TIMEOUT);
	pm_runtime_use_autosuspend(device);

	ret = mei_register(dev, device);
	if (ret)
		goto register_err;

	pm_runtime_put_noidle(device);

	return 0;

register_err:
	mei_stop(dev);
	if (!mei_me_hw_use_polling(hw))
		devm_free_irq(device, hw->irq, dev);

err:
	dev_err(device, "probe failed: %d\n", ret);
	platform_set_drvdata(platdev, NULL);
	return ret;
}

static int mei_gsc_remove(struct platform_device *platdev)
{
	struct mei_device *dev;
	struct mei_me_hw *hw;

	dev = platform_get_drvdata(platdev);
	if (!dev)
		return -ENODEV;

	if (mei_gsc_hw_is_unavailable(&platdev->dev))
		dev->ops = &mei_gsc_hw_ops_null;

	mei_stop(dev);

	hw = to_me_hw(dev);
	if (mei_me_hw_use_polling(hw))
		kthread_stop(hw->polling_thread);

	mei_deregister(dev);

	pm_runtime_disable(&platdev->dev);

	mei_disable_interrupts(dev);
	if (!mei_me_hw_use_polling(hw))
		devm_free_irq(&platdev->dev, hw->irq, dev);

	return 0;
}

static int __maybe_unused mei_gsc_pm_suspend(struct device *device)
{
	struct mei_device *dev = dev_get_drvdata(device);

	if (!dev)
		return -ENODEV;

	mei_stop(dev);

	mei_disable_interrupts(dev);

	return 0;
}

static int __maybe_unused mei_gsc_pm_resume(struct device *device)
{
	struct mei_device *dev = dev_get_drvdata(device);
	int err;
	struct resource *ext_op_mem;
	struct platform_device *platdev;
	const struct platform_device_id *ent;
	const struct mei_cfg *cfg;
	bool is_gsc_pxp = false;
	struct mei_me_hw *hw;

	if (!dev)
		return -ENODEV;

	hw = to_me_hw(dev);
	platdev = to_platform_device(device);
	if (!platdev)
		return -ENODEV;
	ent = platform_get_device_id(platdev);
	cfg = mei_me_get_cfg(ent->driver_data);
	if (!cfg)
		return -ENODEV;
	ext_op_mem = (struct resource *)platdev->dev.platform_data;
	is_gsc_pxp = strcmp(cfg->kind, "gsc") == 0 && ext_op_mem;
	if (is_gsc_pxp) {
		mei_gsc_set_ext_op_mem(hw, ext_op_mem);
		dev->pxp_mode = MEI_DEV_PXP_INIT;
	}

	err = mei_restart(dev);
	if (err)
		return err;

	/* Start timer if stopped in suspend */
	schedule_delayed_work(&dev->timer_work, HZ);

	return 0;
}

static int __maybe_unused mei_gsc_pm_runtime_idle(struct device *device)
{
	struct mei_device *dev = dev_get_drvdata(device);

	if (!dev)
		return -ENODEV;
	if (mei_write_is_idle(dev))
		pm_runtime_autosuspend(device);

	return -EBUSY;
}

static int  __maybe_unused mei_gsc_pm_runtime_suspend(struct device *device)
{
	struct mei_device *dev = dev_get_drvdata(device);
	struct mei_me_hw *hw;
	int ret;

	if (!dev)
		return -ENODEV;

	mutex_lock(&dev->device_lock);

	if (mei_write_is_idle(dev)) {
		hw = to_me_hw(dev);
		hw->pg_state = MEI_PG_ON;

		if (mei_me_hw_use_polling(hw))
			hw->is_active = false;
		ret = 0;
	} else {
		ret = -EAGAIN;
	}

	mutex_unlock(&dev->device_lock);

	return ret;
}

static int __maybe_unused mei_gsc_pm_runtime_resume(struct device *device)
{
	struct mei_device *dev = dev_get_drvdata(device);
	struct mei_me_hw *hw;
	irqreturn_t irq_ret;

	if (!dev)
		return -ENODEV;

	mutex_lock(&dev->device_lock);

	hw = to_me_hw(dev);
	hw->pg_state = MEI_PG_OFF;

	if (mei_me_hw_use_polling(hw)) {
		hw->is_active = true;
		wake_up(&hw->wait_active);
	}

	mutex_unlock(&dev->device_lock);

	irq_ret = mei_me_irq_thread_handler(1, dev);
	if (irq_ret != IRQ_HANDLED)
		dev_err(dev->dev, "thread handler fail %d\n", irq_ret);

	return 0;
}

static const struct dev_pm_ops mei_gsc_pm_ops = {
	SET_SYSTEM_SLEEP_PM_OPS(mei_gsc_pm_suspend,
				mei_gsc_pm_resume)
	SET_RUNTIME_PM_OPS(mei_gsc_pm_runtime_suspend,
			   mei_gsc_pm_runtime_resume,
			   mei_gsc_pm_runtime_idle)
};

static const struct platform_device_id gsc_devtypes[] = {
	{
		.name = "mei-gsc",
		.driver_data = MEI_ME_GSC_CFG,
	},
	{
		.name = "mei-gscfi",
		.driver_data = MEI_ME_GSCFI_CFG,
	},
	{
		/* sentinel */
	}
};

static struct platform_driver mei_gsc_driver = {
	.probe	= mei_gsc_probe,
	.remove = mei_gsc_remove,
	.driver = {
		.name = "mei-gsc",
		.pm = &mei_gsc_pm_ops,
	},
	.id_table = gsc_devtypes,
};

static int __init mei_gsc_init(void)
{
	int ret;

	ret = platform_driver_register(&mei_gsc_driver);

	return ret;
}
module_init(mei_gsc_init);

static void __exit mei_gsc_exit(void)
{
	platform_driver_unregister(&mei_gsc_driver);
}
module_exit(mei_gsc_exit);

MODULE_AUTHOR("Intel Corporation");
MODULE_ALIAS("platform:mei-gsc");
MODULE_ALIAS("platform:mei-gscfi");
MODULE_LICENSE("GPL v2");