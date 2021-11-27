PROJ = Rasterizer

all: dmaStreamEngine simulationDisplay attributeInterpolator

clean:
	rm -rf obj_dir

dmaStreamEngine:
	verilator -DUNITTEST -CFLAGS -std=c++11 --cc -exe ../rtl/RasteriCEr/DmaStreamEngine.v --top-module DmaStreamEngine cpp/sim_DmaStreamEngine.cpp
	-make -C obj_dir -f VDmaStreamEngine.mk
	./obj_dir/VDmaStreamEngine

simulationDisplay:
	verilator -DUNITTEST -CFLAGS -std=c++11 --cc -exe ../rtl/Display/DisplayController8BitILI9486.v --top-module DisplayController8BitILI9486 cpp/sim_DisplayController8BitILI9486.cpp
	-make -C obj_dir -f VDisplayController8BitILI9486.mk
	./obj_dir/VDisplayController8BitILI9486

attributeInterpolator:
	verilator -DUNITTEST -CFLAGS -std=c++11 --cc -exe ../rtl/RasteriCEr/AttributeInterpolator.v --top-module AttributeInterpolator cpp/sim_AttributeInterpolator.cpp -I../rtl/float/rtl/float/ -I../rtl/RasteriCEr/
	-make -C obj_dir -f VAttributeInterpolator.mk
	./obj_dir/VAttributeInterpolator

.SECONDARY:
.PHONY: all clean
