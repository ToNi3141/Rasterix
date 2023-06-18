SUMMARY = "Recipe for  build an external dma-proxy Linux kernel module"
SECTION = "PETALINUX/modules"
LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://COPYING;md5=12f884d2ae1ff87c09e5b7ccc2c4ca7e"

inherit module

INHIBIT_PACKAGE_STRIP = "1"

SRC_URI = "file://Makefile \
           file://dma-proxy.c \
	   file://COPYING \
       file://include/ \
          "

S = "${WORKDIR}"

do_compile () {
    oe_runmake INCLUDE=${S}/include
}

# The inherit of module.bbclass will automatically name module packages with
# "kernel-module-" prefix as required by the oe-core build environment.
