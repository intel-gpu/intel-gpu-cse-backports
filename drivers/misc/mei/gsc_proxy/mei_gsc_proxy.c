// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2022 Intel Corporation
 */

/**
 * DOC: MEI_GSC_PROXY Client Driver
 *
 * The mei_gsc_proxy driver acts as a translation layer between
 * proxy user (I915) and ME FW by proxying messages to ME FW
 */

#include <linux/module.h>
#include <linux/slab.h>
#include <linux/uuid.h>
#include <linux/mei_cl_bus.h>
#include <linux/component.h>
#include <drm/drm_connector.h>
#include <drm/i915_component.h>
#include <drm/i915_gsc_proxy_mei_interface.h>

#include "version/module_version.h"

/**
 * mei_gsc_proxy_send() - Sends a proxy message to ME FW.
 * @dev: device corresponding to the mei_cl_device
 * @buf: a message buffer to send
 * @size: size of the message
 * Return: 0 on Success, <0 on Failure
 */
static int mei_gsc_proxy_send(struct device *dev, const void *buf, size_t size)
{
	struct mei_cl_device *cldev;
	ssize_t ret;

	if (!dev || !buf)
		return -EINVAL;

	cldev = to_mei_cl_device(dev);

	ret = mei_cldev_send(cldev, buf, size);
	if (ret < 0)
		dev_dbg(dev, "mei_cldev_send failed. %zd\n", ret);

	return ret;
}

/**
 * mei_gsc_proxy_recv() - Receives a proxy message from ME FW.
 * @dev: device corresponding to the mei_cl_device
 * @buf: a message buffer to contain the received message
 * @size: size of the buffer
 * Return: bytes sent on Success, <0 on Failure
 */
static int mei_gsc_proxy_recv(struct device *dev, void *buf, size_t size)
{
	struct mei_cl_device *cldev;
	ssize_t ret;

	if (!dev || !buf)
		return -EINVAL;

	cldev = to_mei_cl_device(dev);

	ret = mei_cldev_recv(cldev, buf, size);
	if (ret < 0)
		dev_dbg(dev, "mei_cldev_recv failed. %zd\n", ret);

	return ret;
}

static const struct i915_gsc_proxy_component_ops mei_gsc_proxy_ops = {
	.owner = THIS_MODULE,
	.send = mei_gsc_proxy_send,
	.recv = mei_gsc_proxy_recv,
};

static int mei_component_master_bind(struct device *dev)
{
	struct mei_cl_device *cldev = to_mei_cl_device(dev);
	struct i915_gsc_proxy_component *comp_master = mei_cldev_get_drvdata(cldev);
	int ret;

	comp_master->ops = &mei_gsc_proxy_ops;
	comp_master->mei_dev = dev;
	ret = component_bind_all(dev, comp_master);
	if (ret < 0)
		return ret;

	return 0;
}

static void mei_component_master_unbind(struct device *dev)
{
	struct mei_cl_device *cldev = to_mei_cl_device(dev);
	struct i915_gsc_proxy_component *comp_master = mei_cldev_get_drvdata(cldev);

	component_unbind_all(dev, comp_master);
}

static const struct component_master_ops mei_component_master_ops = {
	.bind = mei_component_master_bind,
	.unbind = mei_component_master_unbind,
};

/**
 * mei_gsc_proxy_component_match - compare function for matching mei.
 *
 *    The function checks if the driver is i915, the subcomponent is SW Proxy
 *    and the grand parent of proxy and the parent of i915 are the same
 *    PCH device.
 *
 * @dev: master device
 * @subcomponent: subcomponent to match (I915_COMPONENT_SWPROXY)
 * @data: compare data (mei gsc proxy device)
 *
 * Return:
 * * 1 - if components match
 * * 0 - otherwise
 */
static int mei_gsc_proxy_component_match(struct device *dev, int subcomponent,
				       void *data)
{
	struct device *base = data;

	if (!dev)
		return 0;

	if (strcmp(dev->driver->name, "i915") ||
	    subcomponent != I915_COMPONENT_GSC_PROXY)
		return 0;

	base = base->parent;
	if (!base) /* mei device */
		return 0;

	base = base->parent; /* pci device */

	dev = dev->parent;
	return (base && dev && dev == base);
}

static int mei_gsc_proxy_probe(struct mei_cl_device *cldev,
			     const struct mei_cl_device_id *id)
{
	struct i915_gsc_proxy_component *comp_master;
	struct component_match *master_match;
	int ret;

	ret = mei_cldev_enable(cldev);
	if (ret < 0) {
		dev_err(&cldev->dev, "mei_cldev_enable Failed. %d\n", ret);
		goto enable_err_exit;
	}

	comp_master = kzalloc(sizeof(*comp_master), GFP_KERNEL);
	if (!comp_master) {
		ret = -ENOMEM;
		goto err_exit;
	}

	master_match = NULL;
	component_match_add_typed(&cldev->dev, &master_match,
				  mei_gsc_proxy_component_match, &cldev->dev);
	if (IS_ERR_OR_NULL(master_match)) {
		ret = -ENOMEM;
		goto err_exit;
	}

	mei_cldev_set_drvdata(cldev, comp_master);
	ret = component_master_add_with_match(&cldev->dev,
					      &mei_component_master_ops,
					      master_match);
	if (ret < 0) {
		dev_err(&cldev->dev, "Master comp add failed %d\n", ret);
		goto err_exit;
	}

	return 0;

err_exit:
	mei_cldev_set_drvdata(cldev, NULL);
	kfree(comp_master);
	mei_cldev_disable(cldev);
enable_err_exit:
	return ret;
}

static void mei_gsc_proxy_remove(struct mei_cl_device *cldev)
{
	struct i915_gsc_proxy_component *comp_master = mei_cldev_get_drvdata(cldev);
	int ret;

	component_master_del(&cldev->dev, &mei_component_master_ops);
	kfree(comp_master);
	mei_cldev_set_drvdata(cldev, NULL);

	ret = mei_cldev_disable(cldev);
	if (ret)
		dev_warn(&cldev->dev, "mei_cldev_disable() failed %d\n", ret);
}

#define MEI_GUID_GSC_PROXY GUID_INIT(0xf73db04, 0x97ab, 0x4125, \
				   0xb8, 0x93, 0xe9, 0x4, 0xad, 0xd, 0x54, 0x64)

static struct mei_cl_device_id mei_gsc_proxy_tbl[] = {
	{ .uuid = MEI_GUID_GSC_PROXY, .version = MEI_CL_VERSION_ANY },
	{ }
};
MODULE_DEVICE_TABLE(mei, mei_gsc_proxy_tbl);

static struct mei_cl_driver mei_gsc_proxy_driver = {
	.id_table = mei_gsc_proxy_tbl,
	.name = KBUILD_MODNAME,
	.probe = mei_gsc_proxy_probe,
	.remove	= mei_gsc_proxy_remove,
};

module_mei_cl_driver(mei_gsc_proxy_driver);

MODULE_AUTHOR("Intel Corporation");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("MEI GSC PROXY");
MODULE_VERSION(BACKPORT_MODULE_VER);