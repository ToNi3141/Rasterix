PROJ = Rasterizer

VERILATOR = verilator

X_RESOLUTION = 640
Y_RESOLUTION = 480
Y_LINE_RESOLUTION = 48

all: mem 

clean:
	rm -rf obj_dir

simulation:
	$(VERILATOR) -DUNITTEST -DX_RESOLUTION=$(X_RESOLUTION) -DY_RESOLUTION=$(Y_RESOLUTION) -DY_LINE_RESOLUTION=$(Y_LINE_RESOLUTION) -CFLAGS '-arch x86_64' --cc top.v --top-module top -I../../Rasterix -I../../Float/rtl/float/
	-make -C obj_dir -f Vtop.mk

mem:
	$(VERILATOR) -Wno-lint -DUNITTEST -DX_RESOLUTION=$(X_RESOLUTION) -DY_RESOLUTION=$(Y_RESOLUTION) -DY_LINE_RESOLUTION=$(Y_LINE_RESOLUTION) -CFLAGS '-arch x86_64 -O3' --cc topMemory.v --top-module top -I../../Rasterix -I../../3rdParty -I../../Float/rtl/float/
	-make -C obj_dir -f Vtop.mk

.SECONDARY:
.PHONY: all clean
