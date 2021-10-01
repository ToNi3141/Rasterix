PROJ = Rasterizer

X_RESOLUTION = 640
Y_RESOLUTION = 480
Y_LINE_RESOLUTION = 120

all: simulation 

clean:
	rm -rf obj_dir

simulation:
	verilator -DUNITTEST -DX_RESOLUTION=$(X_RESOLUTION) -DY_RESOLUTION=$(Y_RESOLUTION) -DY_LINE_RESOLUTION=$(Y_LINE_RESOLUTION) -CFLAGS -std=c++11 --cc top.v --top-module top -I../../RasteriCEr
	-make -C obj_dir -f Vtop.mk

mem:
	verilator -DUNITTEST -DX_RESOLUTION=$(X_RESOLUTION) -DY_RESOLUTION=$(Y_RESOLUTION) -DY_LINE_RESOLUTION=$(Y_LINE_RESOLUTION) -CFLAGS -std=c++11 --cc topMemory.v --top-module top -I../../RasteriCEr
	-make -C obj_dir -f Vtop.mk

.SECONDARY:
.PHONY: all clean
