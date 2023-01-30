PROJ = Rasterizer

VERILATOR = verilator

FRAMEBUFFER_SIZE_BYTES = 16

all: mem 

clean:
	rm -rf obj_dir

simulation:
	$(VERILATOR) -DUNITTEST -DFRAMEBUFFER_SIZE_BYTES=$(FRAMEBUFFER_SIZE_BYTES) -CFLAGS '-arch x86_64' --cc top.v --top-module top -I../../Rasterix -I../../Float/rtl/float/
	-make -C obj_dir -f Vtop.mk

mem:
	$(VERILATOR) -Wno-lint -DUNITTEST -DFRAMEBUFFER_SIZE_BYTES=$(FRAMEBUFFER_SIZE_BYTES) -CFLAGS '-arch x86_64 -O3' --cc topMemory.v --top-module top -I../../Rasterix -I../../3rdParty -I../../Float/rtl/float/
	-make -C obj_dir -f Vtop.mk

.SECONDARY:
.PHONY: all clean
