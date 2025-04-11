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
read_verilog ./../../../../RRX/AttributeF2XConverter.v
read_verilog ./../../../../RRX/AttributeInterpolationX.v
read_verilog ./../../../../RRX/AttributeInterpolator.v
read_verilog ./../../../../RRX/AttributeInterpolatorX.v
read_verilog ./../../../../RRX/AttributePerspectiveCorrectionX.v
read_verilog ./../../../../RRX/ColorBlender.v
read_verilog ./../../../../RRX/ColorInterpolator.v
read_verilog ./../../../../RRX/ColorMixerSigned.v
read_verilog ./../../../../RRX/ColorMixer.v
read_verilog ./../../../../RRX/CommandParser.v
read_verilog ./../../../../RRX/AxisFramebufferReader.v
read_verilog ./../../../../RRX/AxisFramebufferWriter.v
read_verilog ./../../../../RRX/DmaStreamEngine.v
read_verilog ./../../../../RRX/DualPortRam.v
read_verilog ./../../../../RRX/Fog.v
read_verilog ./../../../../RRX/FramebufferReader.v
read_verilog ./../../../../RRX/FramebufferSerializer.v
read_verilog ./../../../../RRX/FrameBuffer.v
read_verilog ./../../../../RRX/FramebufferWriterClear.v
read_verilog ./../../../../RRX/FramebufferWriterStrobeGen.v
read_verilog ./../../../../RRX/FramebufferWriter.v
read_verilog ./../../../../RRX/FunctionInterpolator.v
read_verilog ./../../../../RRX/LodCalculator.v
read_verilog ./../../../../RRX/MemoryReadRequestGenerator.v
read_verilog ./../../../../RRX/MipmapOptimizedRam.v
read_verilog ./../../../../RRX/PagedMemoryReader.v
read_verilog ./../../../../RRX/PerFragmentPipeline.v
read_verilog ./../../../../RRX/PixelPipeline.v
read_verilog ./../../../../RRX/PixelUtil.vh
read_verilog ./../../../../RRX/RRXEFCore.v
read_verilog ./../../../../RRX/RRXIFCore.v
read_verilog ./../../../../RRX/RRXRenderCore.v
read_verilog ./../../../../RRX/RasterizerCommands.vh
read_verilog ./../../../../RRX/Rasterizer.v
read_verilog ./../../../../RRX/RegisterAndDescriptorDefines.vh
read_verilog ./../../../../RRX/RegisterBank.v
read_verilog ./../../../../RRX/RRXEF.v
read_verilog ./../../../../RRX/RRXIF.v
read_verilog ./../../../../RRX/RRX.v
read_verilog ./../../../../RRX/StencilOp.v
read_verilog ./../../../../RRX/StreamConcatFifo.v
read_verilog ./../../../../RRX/StreamFramebuffer.v
read_verilog ./../../../../RRX/TestFunc.v
read_verilog ./../../../../RRX/TexEnv.v
read_verilog ./../../../../RRX/TextureBuffer.v
read_verilog ./../../../../RRX/TextureFilter.v
read_verilog ./../../../../RRX/TextureMappingUnit.v
read_verilog ./../../../../RRX/TextureSampler.v
read_verilog ./../../../../RRX/TriangleStreamF2XConverter.v
read_verilog ./../../../../RRX/TrueDualPortRam.v
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
read_verilog ./../../../../3rdParty/verilog-axis/axis_adapter.v
read_verilog ./../../../../3rdParty/verilog-axis/axis_broadcast.v
read_verilog ./../../../../3rdParty/verilog-axi/arbiter.v
read_verilog ./../../../../3rdParty/verilog-axi/axi_crossbar_addr.v
read_verilog ./../../../../3rdParty/verilog-axi/axi_crossbar_rd.v
read_verilog ./../../../../3rdParty/verilog-axi/axi_crossbar.v
read_verilog ./../../../../3rdParty/verilog-axi/axi_crossbar_wr.v
read_verilog ./../../../../3rdParty/verilog-axi/axi_adapter_rd.v
read_verilog ./../../../../3rdParty/verilog-axi/axi_adapter.v
read_verilog ./../../../../3rdParty/verilog-axi/axi_adapter_wr.v
read_verilog ./../../../../3rdParty/verilog-axi/axi_ram.v
read_verilog ./../../../../3rdParty/verilog-axi/axi_register_rd.v
read_verilog ./../../../../3rdParty/verilog-axi/axi_register_wr.v
read_verilog ./../../../../3rdParty/verilog-axi/priority_encoder.v
read_verilog ./../../../../3rdParty/sfifo.v
read_verilog ./../../../../3rdParty/skidbuffer.v
read_verilog ./../../../../Display/DisplayController8BitILI9341.v
read_verilog ./../../../../Util/Serial2AXIS.v
read_verilog ./../../../../Util/SpiSlave.v
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
phys_opt_design

# Create reports
report_timing_summary -file $REPORT_PATH/post_route_timing_summary.rpt
report_timing -sort_by group -max_paths 100 -path_type summary -file $REPORT_PATH/post_route_timing.rpt
report_clock_utilization -file $REPORT_PATH/clock_util.rpt
report_utilization -file $REPORT_PATH/post_route_util.rpt
report_power -file $REPORT_PATH/post_route_power.rpt
report_drc -file $REPORT_PATH/post_imp_drc.rpt

# Write bit file 
write_bitstream -bin_file rrx.bit
