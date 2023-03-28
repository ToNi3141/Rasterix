PROJ = Rasterizer

all: dmaStreamEngine simulationILI9486 simulationILI9341 attributeInterpolator functionInterpolator textureSampler colorInterpolator texEnv stencilOp
 
clean:
	rm -rf obj_dir

dmaStreamEngine:
	verilator -DUNITTEST -CFLAGS -std=c++11 --cc -exe ../rtl/Rasterix/DmaStreamEngine.v --top-module DmaStreamEngine cpp/sim_DmaStreamEngine.cpp
	-make -C obj_dir -f VDmaStreamEngine.mk
	./obj_dir/VDmaStreamEngine

simulationILI9486:
	verilator -DUNITTEST -CFLAGS -std=c++11 --cc -exe ../rtl/Display/DisplayController8BitILI9486.v --top-module DisplayController8BitILI9486 cpp/sim_DisplayController8BitILI9486.cpp
	-make -C obj_dir -f VDisplayController8BitILI9486.mk
	./obj_dir/VDisplayController8BitILI9486

simulationILI9341:
	verilator -DUNITTEST -CFLAGS -std=c++11 --cc -exe ../rtl/Display/DisplayController8BitILI9341.v --top-module DisplayController8BitILI9341 cpp/sim_DisplayController8BitILI9341.cpp
	-make -C obj_dir -f VDisplayController8BitILI9341.mk
	./obj_dir/VDisplayController8BitILI9341

attributeInterpolator:
	verilator -DUNITTEST -CFLAGS -std=c++11 --cc -exe ../rtl/Rasterix/AttributeInterpolator.v --top-module AttributeInterpolator cpp/sim_AttributeInterpolator.cpp -I../rtl/float/rtl/float/ -I../rtl/Rasterix/
	-make -C obj_dir -f VAttributeInterpolator.mk
	./obj_dir/VAttributeInterpolator

functionInterpolator:
	verilator -DUNITTEST -CFLAGS -std=c++11 --cc -exe ../rtl/Rasterix/FunctionInterpolator.v --top-module FunctionInterpolator cpp/sim_FunctionInterpolator.cpp -I../rtl/Rasterix/
	-make -C obj_dir -f VFunctionInterpolator.mk
	./obj_dir/VFunctionInterpolator

textureSampler:
	verilator -DUNITTEST -CFLAGS -std=c++11 --cc -exe v/TextureSamplerTestModule.v --top-module TextureSamplerTestModule cpp/sim_TextureSampler.cpp -I../rtl/Rasterix/ -I../rtl/float/rtl/float/
	-make -C obj_dir -f VTextureSamplerTestModule.mk
	./obj_dir/VTextureSamplerTestModule

colorInterpolator:
	verilator -DUNITTEST -CFLAGS -std=c++11 --cc -exe ../rtl/Rasterix/ColorInterpolator.v --top-module ColorInterpolator cpp/sim_ColorInterpolator.cpp -I../rtl/Rasterix/
	-make -C obj_dir -f VColorInterpolator.mk
	./obj_dir/VColorInterpolator

texEnv:
	verilator -DUNITTEST -CFLAGS -std=c++11 --cc -exe ../rtl/Rasterix/TexEnv.v --top-module TexEnv cpp/sim_TexEnv.cpp -I../rtl/Rasterix/ -I../rtl/float/rtl/float/
	-make -C obj_dir -f VTexEnv.mk
	./obj_dir/VTexEnv

stencilOp:
	verilator -DUNITTEST -CFLAGS -std=c++11 --cc -exe ../rtl/Rasterix/StencilOp.v --top-module StencilOp cpp/sim_StencilOp.cpp -I../rtl/Rasterix/ -I../rtl/float/rtl/float/
	-make -C obj_dir -f VStencilOp.mk
	./obj_dir/VStencilOp

.SECONDARY:
.PHONY: all clean
