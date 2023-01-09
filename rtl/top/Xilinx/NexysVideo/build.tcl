set SYNTH_OUT ./synth
file mkdir $SYNTH_OUT

# STEP#0: generate ip cores
# set board_part digilentinc.com:nexys_video:part0:1.2
# create_project -in_memory -part xc7a200tsbg484-1 
# set_property board_part $board_part [current_project]
# set_property source_mgmt_mode All [current_project]
# set_property target_language Verilog [current_project]
# set_property DEFAULT_LIB work [current_project]

#create_project -in_memory
set_part xc7a200tsbg484-1
set_property TARGET_LANGUAGE Verilog [current_project]
set_property BOARD_PART digilentinc.com:nexys_video:part0:1.2 [current_project]
set_property DEFAULT_LIB work [current_project]
# create_project -in_memory
#create_project -in_memory -part xc7a200tsbg484-1 

set_property source_mgmt_mode All [current_project]

#
# STEP#1: setup design sources and constraints
#
read_verilog ./../../../Rasterix/AttributeInterpolator.v
read_verilog ./../../../Rasterix/AttributeInterpolatorDefines.vh
read_verilog ./../../../Rasterix/ColorBlender.v
read_verilog ./../../../Rasterix/ColorInterpolator.v
read_verilog ./../../../Rasterix/ColorMixer.v
read_verilog ./../../../Rasterix/ColorMixerSigned.v
read_verilog ./../../../Rasterix/CommandParser.v
read_verilog ./../../../Rasterix/DmaStreamEngine.v
read_verilog ./../../../Rasterix/DualPortRam.v
read_verilog ./../../../Rasterix/Fog.v
read_verilog ./../../../Rasterix/FrameBuffer.v
read_verilog ./../../../Rasterix/FunctionInterpolator.v
read_verilog ./../../../Rasterix/PerFragmentPipeline.v
read_verilog ./../../../Rasterix/PixelPipeline.v
read_verilog ./../../../Rasterix/PixelUtil.vh
read_verilog ./../../../Rasterix/Rasterix.v
read_verilog ./../../../Rasterix/Rasterizer.v
read_verilog ./../../../Rasterix/RasterizerDefines.vh
read_verilog ./../../../Rasterix/RegisterAndDescriptorDefines.vh
read_verilog ./../../../Rasterix/RegisterBank.v
read_verilog ./../../../Rasterix/TestFunc.v
read_verilog ./../../../Rasterix/TexEnv.v
read_verilog ./../../../Rasterix/TextureBuffer.v
read_verilog ./../../../Rasterix/TextureFilter.v
read_verilog ./../../../Rasterix/TextureMappingUnit.v
read_verilog ./../../../Rasterix/TextureSampler.v
read_verilog ./../../../Rasterix/TrueDualPortRam.v
read_verilog ./../../../Float/rtl/float/FindExponent.v
read_verilog ./../../../Float/rtl/float/FloatAdd.v
read_verilog ./../../../Float/rtl/float/FloatFastRecip.v
read_verilog ./../../../Float/rtl/float/FloatFastRecip2.v
read_verilog ./../../../Float/rtl/float/FloatMul.v
read_verilog ./../../../Float/rtl/float/FloatSub.v
read_verilog ./../../../Float/rtl/float/FloatToInt.v
read_verilog ./../../../Float/rtl/float/IntToFloat.v
read_verilog ./../../../Float/rtl/float/ValueDelay.v
read_verilog ./../../../Float/rtl/float/ValueTrack.v
read_verilog ./../../../Util/FT245S2AXIS.v
read_verilog ./../../../3rdParty/FPGA-ftdi245fifo/RTL/ftdi_245fifo.sv
read_verilog ./../../../3rdParty/FPGA-ftdi245fifo/RTL/stream_async_fifo.sv
read_verilog ./../../../3rdParty/FPGA-ftdi245fifo/RTL/stream_wtrans.sv
read_verilog ./../../../Display/Dvi.v
read_verilog ./../../../3rdParty/core_dvi_framebuffer/src_v/dvi_framebuffer.v
read_verilog ./../../../3rdParty/core_dvi_framebuffer/src_v/dvi_tmds_encoder.v
read_verilog ./../../../3rdParty/core_dvi_framebuffer/src_v/dvi_resync.v
read_verilog ./../../../3rdParty/core_dvi_framebuffer/src_v/dvi_serialiser.v
read_verilog ./../../../3rdParty/core_dvi_framebuffer/src_v/dvi.v
read_verilog ./../../../3rdParty/core_dvi_framebuffer/src_v/dvi_framebuffer_defs.v
read_verilog ./../../../3rdParty/core_dvi_framebuffer/src_v/dvi_framebuffer_fifo.v
read_xdc ./Nexys-Video-Master.xdc

#set_property IP_REPO_PATHS {./} [current_fileset]

generate_target all [get_files ./design_1.bd]
read_bd ./design_1.bd
# create_bd_design ./design_1.bd
#open_bd_design ./design_1.bd


#set_property synth_checkpoint_mode None [get_files ./design_1.bd]
#generate_target all [get_files ./design_1.bd]
# read_bd ./design_1.bd
# open_bd_design ./design_1.bd

#generate_target all [get_files ./design_1.bd]
make_wrapper -import -files [get_files ./design_1.bd] -top
#add_files -norecurse ./src/hdl/design_1.v


set_property synth_checkpoint_mode None [get_files ./design_1.bd]
generate_target all [get_files ./design_1.bd]

#Run synthesis and implementation
synth_design -top design_1
opt_design
place_design
route_design

report_timing_summary -file $SYNTH_OUT/post_route_timing_summary.rpt
report_timing -sort_by group -max_paths 100 -path_type summary -file $SYNTH_OUT/post_route_timing.rpt
report_clock_utilization -file $SYNTH_OUT/clock_util.rpt
report_utilization -file $SYNTH_OUT/post_route_util.rpt
report_power -file $SYNTH_OUT/post_route_power.rpt
report_drc -file $SYNTH_OUT/post_imp_drc.rpt

write_bitstream rasterix.bit

# generate_target all [get_files *design_1.bd]
# synth_ip [get_files *design_1.bd]
# get_files -all -of_objects [get_files *design_1.bd]
