ifneq ($(KERNELRELEASE),)
NOSTDINC_FLAGS += \
        -I$(M)/backport-include/ \
	-I$(M)/backport-include/uapi
obj-m := mei.o
mei-objs := drivers/misc/mei/init.o
mei-objs += drivers/misc/mei/hbm.o
mei-objs += drivers/misc/mei/interrupt.o
mei-objs += drivers/misc/mei/client.o
mei-objs += drivers/misc/mei/main.o
mei-objs += drivers/misc/mei/dma-ring.o
mei-objs += drivers/misc/mei/bus.o
mei-objs += drivers/misc/mei/bus-fixup.o
mei-$(CONFIG_DEBUG_FS) += drivers/misc/mei/debugfs.o
mei-$(CONFIG_EVENT_TRACING) += drivers/misc/mei/mei-trace.o
ccflags-y += -I$(M)/drivers/misc/mei/

obj-m += mei-me.o
mei-me-objs := drivers/misc/mei/pci-me.o
mei-me-objs += drivers/misc/mei/hw-me.o

obj-m += mei-gsc.o
mei-gsc-objs := drivers/misc/mei/gsc-me.o

obj-m += mei_hdcp.o
mei_hdcp-objs := drivers/misc/mei/hdcp/mei_hdcp.o

obj-m += mei_pxp.o
mei_pxp-objs := drivers/misc/mei/pxp/mei_pxp.o

obj-m += mei_wdt.o
mei_wdt-objs := drivers/watchdog/mei_wdt.o

obj-m += mei_iaf.o
mei_iaf-objs := drivers/misc/mei/iaf/mei_iaf.o

else
KDIR ?= /lib/modules/`uname -r`/build

modules:
	$(MAKE) -C $(KDIR) M=$$PWD
	mkdir -p bin
	cp *.ko bin/

clean:
	$(MAKE) -C $(KDIR) M=$$PWD clean
	rm -rf bin/

modules_install:
	$(MAKE) -C $(KDIR) M=$$PWD modules_install

help:
	$(MAKE) -C $(KDIR) M=$$PWD help

endif
