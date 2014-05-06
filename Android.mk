#Android makefile to build kernel as a part of Android Build
#Its here and not under gaosp-kernel/ for compatiblity with the froyo branch
#The config for the kernel is arch/arm/configs/gaosp_msm_defconfig
LOCAL_PATH := $(call my-dir)
ifdef BUILD_KERNEL
ifeq ($(TARGET_BOOTLOADER_BOARD_NAME),betelgeuse)

include $(CLEAR_VARS)

KERNEL_OUT := $(TARGET_OUT_INTERMEDIATES)/kernel
KERNEL_CONFIG := $(KERNEL_OUT)/.config
KERNEL_SRC := $(LOCAL_PATH)
MODULES_OUT := $(CURDIR)/$(TARGET_OUT)/lib/modules

TARGET_PREBUILT_KERNEL := $(KERNEL_OUT)/arch/arm/boot/zImage
$(TARGET_PREBUILT_KERNEL):= build_kernel
TARGET_COPY_MODULES:= build_kernel
#TARGET_COPY_MODULES:= $(KERNEL_OUT)/arch/arm/boot/zImage

$(INSTALLED_KERNEL_TARGET): $(TARGET_PREBUILT_KERNEL) | $(ACP) $(TARGET_COPY_MODULES)
	$(transform-prebuilt-to-target)

$(KERNEL_OUT):
	mkdir -p $(KERNEL_OUT)

$(MODULES_OUT):
	mkdir -p $(MODULES_OUT)

$(KERNEL_CONFIG): $(LOCAL_PATH)/arch/arm/configs/tegra_betelgeuse_android_defconfig | $(ACP)
	$(transform-prebuilt-to-target)

$(TARGET_PREBUILT_KERNEL): $(KERNEL_OUT) $(KERNEL_CONFIG)
	$(MAKE) -C $(KERNEL_SRC) O=../../$(KERNEL_OUT) ARCH=arm CROSS_COMPILE=$(shell pwd)/prebuilt/linux-x86/toolchain/arm-eabi-4.4.3/bin/arm-eabi- INSTALL_MOD_STRIP=1

$(TARGET_COPY_MODULES): $(MODULES_OUT) $(TARGET_PREBUILT_KERNEL)
	$(eval _modules_files := $(shell find $(KERNEL_OUT) -name '*.ko'))
	$(foreach _module_file, $(_modules_files), \
		$(eval _dest_file := $(shell basename $(_module_file) )) \
		$(shell cp -f $(_module_file) $(CURDIR)/$(TARGET_OUT)/lib/modules/$(_dest_file)) \
	)

endif
endif
