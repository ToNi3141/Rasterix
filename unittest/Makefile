PROJ = Rasterizer

all: \
	dmaStreamEngine  \
	simulationILI9486 \
	simulationILI9341 \
	attributeInterpolator \
	functionInterpolator \
	textureSampler \
	colorInterpolator \
	texEnv \
	stencilOp \
	memoryReadRequestGenerator \
	framebufferSerializer \
	streamConcatFifo \
	framebufferWriter \
	framebufferWriterStrobeGen \
	framebufferWriterClear \
	framebufferReader \
	lodCalculator \
	attributeInterpolationX \
	attributePerspectiveCorrectionX \
	triangleStreamF2XConverter \
	pagedMemoryReader
 
clean:
	rm -rf obj_dir

dmaStreamEngine:
	verilator -DUNITTEST -CFLAGS -std=c++20 --cc -exe ../rtl/Rasterix/DmaStreamEngine.v --top-module DmaStreamEngine cpp/sim_DmaStreamEngine.cpp
	-make -C obj_dir -f VDmaStreamEngine.mk
	./obj_dir/VDmaStreamEngine

simulationILI9486:
	verilator -DUNITTEST -CFLAGS -std=c++20 --cc -exe ../rtl/Display/DisplayController8BitILI9486.v --top-module DisplayController8BitILI9486 cpp/sim_DisplayController8BitILI9486.cpp
	-make -C obj_dir -f VDisplayController8BitILI9486.mk
	./obj_dir/VDisplayController8BitILI9486

simulationILI9341:
	verilator -DUNITTEST -CFLAGS -std=c++20 --cc -exe ../rtl/Display/DisplayController8BitILI9341.v --top-module DisplayController8BitILI9341 cpp/sim_DisplayController8BitILI9341.cpp
	-make -C obj_dir -f VDisplayController8BitILI9341.mk
	./obj_dir/VDisplayController8BitILI9341

attributeInterpolator:
	verilator -DUNITTEST -CFLAGS -std=c++20 --cc -exe ../rtl/Rasterix/AttributeInterpolator.v --top-module AttributeInterpolator cpp/sim_AttributeInterpolator.cpp -I../rtl/Float/rtl/float/ -I../rtl/Rasterix/
	-make -C obj_dir -f VAttributeInterpolator.mk
	./obj_dir/VAttributeInterpolator

functionInterpolator:
	verilator -DUNITTEST -CFLAGS -std=c++20 --cc -exe ../rtl/Rasterix/FunctionInterpolator.v --top-module FunctionInterpolator cpp/sim_FunctionInterpolator.cpp -I../rtl/Rasterix/
	-make -C obj_dir -f VFunctionInterpolator.mk
	./obj_dir/VFunctionInterpolator

textureSampler:
	verilator -DUNITTEST -CFLAGS -std=c++20 --cc -exe v/TextureSamplerTestModule.v --top-module TextureSamplerTestModule cpp/sim_TextureSampler.cpp -I../rtl/Rasterix/ -I../rtl/Float/rtl/float/
	-make -C obj_dir -f VTextureSamplerTestModule.mk
	./obj_dir/VTextureSamplerTestModule

colorInterpolator:
	verilator -DUNITTEST -CFLAGS -std=c++20 --cc -exe ../rtl/Rasterix/ColorInterpolator.v --top-module ColorInterpolator cpp/sim_ColorInterpolator.cpp -I../rtl/Rasterix/
	-make -C obj_dir -f VColorInterpolator.mk
	./obj_dir/VColorInterpolator

texEnv:
	verilator -DUNITTEST -CFLAGS -std=c++20 --cc -exe ../rtl/Rasterix/TexEnv.v --top-module TexEnv cpp/sim_TexEnv.cpp -I../rtl/Rasterix/ -I../rtl/Float/rtl/float/
	-make -C obj_dir -f VTexEnv.mk
	./obj_dir/VTexEnv

stencilOp:
	verilator -DUNITTEST -CFLAGS -std=c++20 --cc -exe ../rtl/Rasterix/StencilOp.v --top-module StencilOp cpp/sim_StencilOp.cpp -I../rtl/Rasterix/ -I../rtl/Float/rtl/float/
	-make -C obj_dir -f VStencilOp.mk
	./obj_dir/VStencilOp

memoryReadRequestGenerator:
	verilator -DUNITTEST -CFLAGS -std=c++20 --cc -exe ../rtl/Rasterix/MemoryReadRequestGenerator.v --top-module MemoryReadRequestGenerator cpp/sim_MemoryReadRequestGenerator.cpp -I../rtl/Rasterix/
	-make -C obj_dir -f VMemoryReadRequestGenerator.mk
	./obj_dir/VMemoryReadRequestGenerator

framebufferSerializer:
	verilator -DUNITTEST -CFLAGS -std=c++20 --cc -exe ../rtl/Rasterix/FramebufferSerializer.v --top-module FramebufferSerializer cpp/sim_FramebufferSerializer.cpp -I../rtl/Rasterix/
	-make -C obj_dir -f VFramebufferSerializer.mk
	./obj_dir/VFramebufferSerializer

streamConcatFifo:
	verilator -DUNITTEST -CFLAGS -std=c++20 --cc -exe ../rtl/Rasterix/StreamConcatFifo.v --top-module StreamConcatFifo cpp/sim_StreamConcatFifo.cpp -I../rtl/Rasterix/ -I../rtl/3rdParty/
	-make -C obj_dir -f VStreamConcatFifo.mk
	./obj_dir/VStreamConcatFifo

framebufferWriter:
	verilator -DUNITTEST -CFLAGS -std=c++20 --cc -exe ../rtl/Rasterix/FramebufferWriter.v --top-module FramebufferWriter cpp/sim_FramebufferWriter.cpp -I../rtl/Rasterix/ -I../rtl/3rdParty/
	-make -C obj_dir -f VFramebufferWriter.mk
	./obj_dir/VFramebufferWriter

framebufferWriterStrobeGen:
	verilator -DUNITTEST -CFLAGS -std=c++20 --cc -exe ../rtl/Rasterix/FramebufferWriterStrobeGen.v --top-module FramebufferWriterStrobeGen cpp/sim_FramebufferWriterStrobeGen.cpp -I../rtl/Rasterix/ -I../rtl/3rdParty/
	-make -C obj_dir -f VFramebufferWriterStrobeGen.mk
	./obj_dir/VFramebufferWriterStrobeGen

framebufferWriterClear:
	verilator -DUNITTEST -CFLAGS -std=c++20 --cc -exe ../rtl/Rasterix/FramebufferWriterClear.v --top-module FramebufferWriterClear cpp/sim_FramebufferWriterClear.cpp -I../rtl/Rasterix/ -I../rtl/3rdParty/
	-make -C obj_dir -f VFramebufferWriterClear.mk
	./obj_dir/VFramebufferWriterClear

framebufferReader:
	verilator -DUNITTEST -CFLAGS -std=c++20 --cc -exe ../rtl/Rasterix/FramebufferReader.v --top-module FramebufferReader cpp/sim_FramebufferReader.cpp -I../rtl/Rasterix/ -I../rtl/3rdParty/ -I../rtl/3rdParty/verilog-axis
	-make -C obj_dir -f VFramebufferReader.mk
	./obj_dir/VFramebufferReader

lodCalculator:
	verilator -DUNITTEST -CFLAGS -std=c++20 --cc -exe ../rtl/Rasterix/LodCalculator.v --top-module LodCalculator cpp/sim_LodCalculator.cpp -I../rtl/Rasterix/ -I../rtl/3rdParty/ 
	-make -C obj_dir -f VLodCalculator.mk
	./obj_dir/VLodCalculator

attributeInterpolationX:
	verilator -DUNITTEST -CFLAGS -std=c++20 --cc -exe ../rtl/Rasterix/AttributeInterpolationX.v --top-module AttributeInterpolationX cpp/sim_AttributeInterpolationX.cpp -I../rtl/Rasterix/ -I../rtl/3rdParty/ -I../rtl/Float/rtl/float/
	-make -C obj_dir -f VAttributeInterpolationX.mk
	./obj_dir/VAttributeInterpolationX

attributePerspectiveCorrectionX:
	verilator -DUNITTEST -CFLAGS -std=c++20 --cc -exe ../rtl/Rasterix/AttributePerspectiveCorrectionX.v --top-module AttributePerspectiveCorrectionX cpp/sim_AttributePerspectiveCorrectionX.cpp -I../rtl/Rasterix/ -I../rtl/3rdParty/ -I../rtl/Float/rtl/float/
	-make -C obj_dir -f VAttributePerspectiveCorrectionX.mk
	./obj_dir/VAttributePerspectiveCorrectionX

triangleStreamF2XConverter:
	verilator -DUNITTEST -CFLAGS -std=c++20 --cc -exe ../rtl/Rasterix/TriangleStreamF2XConverter.v --top-module TriangleStreamF2XConverter cpp/sim_TriangleStreamF2XConverter.cpp -I../rtl/Rasterix/ -I../rtl/3rdParty/ -I../rtl/Float/rtl/float/
	-make -C obj_dir -f VTriangleStreamF2XConverter.mk
	./obj_dir/VTriangleStreamF2XConverter

pagedMemoryReader:
	verilator -DUNITTEST -CFLAGS -std=c++20 --cc -exe ../rtl/Rasterix/PagedMemoryReader.v --top-module PagedMemoryReader cpp/sim_PagedMemoryReader.cpp -I../rtl/Rasterix/ -I../rtl/3rdParty/ -I../rtl/Float/rtl/float/
	-make -C obj_dir -f VPagedMemoryReader.mk
	./obj_dir/VPagedMemoryReader

.SECONDARY:
.PHONY: all clean
