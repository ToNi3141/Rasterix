set SYNTH_OUT ./synth
set REPORT_PATH ./reports
set BOARD_FILE .srcs/sources_1/bd/design_1/design_1.bd
file delete -force $SYNTH_OUT
file mkdir $SYNTH_OUT

# Change to synth directory as build directory
cd $SYNTH_OUT

file mkdir $REPORT_PATH

# Setup project properties
set_part xc7a35tcpg236-1
set_property TARGET_LANGUAGE Verilog [current_project]
set_property BOARD_PART digilentinc.com:cmod_a7-35t:part0:1.2 [current_project]
set_property DEFAULT_LIB work [current_project]
set_property source_mgmt_mode All [current_project]

# Load verilog source
read_verilog ./../../../../Rasterix/AttributeInterpolator.v
read_verilog ./../../../../Rasterix/AttributeF2XConverter.v
read_verilog ./../../../../Rasterix/AttributeInterpolatorX.v
read_verilog ./../../../../Rasterix/AttributeInterpolationX.v
read_verilog ./../../../../Rasterix/AttributePerspectiveCorrectionX.v
read_verilog ./../../../../Rasterix/ColorBlender.v
read_verilog ./../../../../Rasterix/ColorInterpolator.v
read_verilog ./../../../../Rasterix/ColorMixerSigned.v
read_verilog ./../../../../Rasterix/ColorMixer.v
read_verilog ./../../../../Rasterix/CommandParser.v
read_verilog ./../../../../Rasterix/DmaStreamEngine.v
read_verilog ./../../../../Rasterix/DualPortRam.v
read_verilog ./../../../../Rasterix/Fog.v
read_verilog ./../../../../Rasterix/FrameBuffer.v
read_verilog ./../../../../Rasterix/FramebufferReader.v
read_verilog ./../../../../Rasterix/FramebufferSerializer.v
read_verilog ./../../../../Rasterix/FramebufferWriter.v
read_verilog ./../../../../Rasterix/FramebufferWriterClear.v
read_verilog ./../../../../Rasterix/FramebufferWriterStrobeGen.v
read_verilog ./../../../../Rasterix/FunctionInterpolator.v
read_verilog ./../../../../Rasterix/LodCalculator.v
read_verilog ./../../../../Rasterix/MipmapOptimizedRam.v
read_verilog ./../../../../Rasterix/MemoryReadRequestGenerator.v
read_verilog ./../../../../Rasterix/PerFragmentPipeline.v
read_verilog ./../../../../Rasterix/PixelPipeline.v
read_verilog ./../../../../Rasterix/PixelUtil.vh
read_verilog ./../../../../Rasterix/RasterixEF.v
read_verilog ./../../../../Rasterix/RasterixRenderCore.v
read_verilog ./../../../../Rasterix/Rasterizer.v
read_verilog ./../../../../Rasterix/RasterizerCommands.vh
read_verilog ./../../../../Rasterix/RegisterAndDescriptorDefines.vh
read_verilog ./../../../../Rasterix/RegisterBank.v
read_verilog ./../../../../Rasterix/StencilOp.v
read_verilog ./../../../../Rasterix/StreamBarrier.v
read_verilog ./../../../../Rasterix/StreamConcatFifo.v
read_verilog ./../../../../Rasterix/StreamFramebuffer.v
read_verilog ./../../../../Rasterix/StreamSemaphore.v
read_verilog ./../../../../Rasterix/TestFunc.v
read_verilog ./../../../../Rasterix/TexEnv.v
read_verilog ./../../../../Rasterix/TextureBuffer.v
read_verilog ./../../../../Rasterix/TextureFilter.v
read_verilog ./../../../../Rasterix/TextureMappingUnit.v
read_verilog ./../../../../Rasterix/TextureSampler.v
read_verilog ./../../../../Rasterix/TrueDualPortRam.v
read_verilog ./../../../../Rasterix/StencilOp.v
read_verilog ./../../../../Float/rtl/float/XRecip.v
read_verilog ./../../../../Float/rtl/float/ComputeRecip.v
read_verilog ./../../../../Float/rtl/float/FindExponent.v
read_verilog ./../../../../Float/rtl/float/FloatAdd.v
read_verilog ./../../../../Float/rtl/float/FloatRecip.v
read_verilog ./../../../../Float/rtl/float/FloatFastRecip.v
read_verilog ./../../../../Float/rtl/float/FloatMul.v
read_verilog ./../../../../Float/rtl/float/FloatSub.v
read_verilog ./../../../../Float/rtl/float/FloatToInt.v
read_verilog ./../../../../Float/rtl/float/IntToFloat.v
read_verilog ./../../../../Float/rtl/float/ValueDelay.v
read_verilog ./../../../../Float/rtl/float/ValueTrack.v
read_verilog ./../../../../Util/Serial2AXIS.v
read_verilog ./../../../../Util/SpiSlave.v
read_verilog ./../../../../3rdParty/verilog-axis/axis_broadcast.v
read_verilog ./../../../../3rdParty/skidbuffer.v
read_verilog ./../../../../3rdParty/sfifo.v
read_verilog ./../../../../Display/DisplayController8BitILI9341.v
read_verilog ./../AsyncSramController.v
read_verilog ./../AsyncSramPhy.v
read_xdc ./../Cmod-A7-Master.xdc

source ../design_1.tcl

# Open board file
read_bd $BOARD_FILE
# Create wrapper for the board file and set the board file as top
make_wrapper -import -files [get_files $BOARD_FILE] -top

set_property synth_checkpoint_mode None [get_files $BOARD_FILE]
generate_target all [get_files $BOARD_FILE]


# Run synthesis and implementation
synth_design -top design_1_wrapper
opt_design
place_design
phys_opt_design
route_design

# Create reports
report_timing_summary -file $REPORT_PATH/post_route_timing_summary.rpt
report_timing -sort_by group -max_paths 100 -path_type summary -file $REPORT_PATH/post_route_timing.rpt
report_clock_utilization -file $REPORT_PATH/clock_util.rpt
report_utilization -file $REPORT_PATH/post_route_util.rpt
report_power -file $REPORT_PATH/post_route_power.rpt
report_drc -file $REPORT_PATH/post_imp_drc.rpt

# Write bit file 
write_bitstream -bin_file rasterix.bit
