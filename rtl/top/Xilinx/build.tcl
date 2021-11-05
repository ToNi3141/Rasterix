set SYNTH_OUT ./synth
file mkdir $SYNTH_OUT

# STEP#0: generate ip cores
set board_part digilentinc.com:arty-a7-35:part0:1.0
create_project -in_memory -part xc7a35ticsg324-1L 
set_property board_part $board_part [current_project]
read_ip ./clk_wiz_0.xci
generate_target all [get_files *clk_wiz_0.xci]
synth_ip [get_files *clk_wiz_0.xci]
get_files -all -of_objects [get_files *clk_wiz_0.xci]

#
# STEP#1: setup design sources and constraints
#
read_verilog ./top.v
read_verilog ./../../RasteriCEr/DualPortRam.v
read_verilog ./../../RasteriCEr/FrameBuffer.v
read_verilog ./../../RasteriCEr/Rasterizer.v
read_verilog ./../../RasteriCEr/RasteriCEr.v
read_verilog ./../../RasteriCEr/FragmentPipeline.v
read_verilog ./../../RasteriCEr/FragmentPipelineIce40Wrapper.v
read_verilog ./../../RasteriCEr/CommandParser.v
read_verilog ./../../RasteriCEr/DmaStreamEngine.v
read_verilog ./../../RasteriCEr/Serial2AXIS.v
read_verilog ./../../RasteriCEr/Recip.v
read_verilog ./../../RasteriCEr/TextureBuffer.v
read_verilog ./../../Display/DisplayControllerSpi.v
read_verilog ./../../3rdParty/SPI_Slave.v
read_verilog ./../../3rdParty/sfifo.v
read_verilog ./../../3rdParty/axi_ram.v
read_xdc ./artySpi.xdc

#
# STEP#2: run synthesis, report utilization and timing estimates, write checkpoint design
#
synth_design -top top -part xc7a35ticsg324-1L -flatten rebuilt
write_checkpoint -force $SYNTH_OUT/post_synth
report_timing_summary -file $SYNTH_OUT/post_synth_timing_summary.rpt
report_power -file $SYNTH_OUT/post_synth_power.rpt
#
# STEP#3: run placement and logic optimization, report utilization and timing estimates, write checkpoint design
#
opt_design
power_opt_design
place_design
phys_opt_design
write_checkpoint -force $SYNTH_OUT/post_place
report_timing_summary -file $SYNTH_OUT/post_place_timing_summary.rpt
#
# STEP#4: run router, report actual utilization and timing, write checkpoint design, run drc, write verilog and xdc out
#
route_design
write_checkpoint -force $SYNTH_OUT/post_route
report_timing_summary -file $SYNTH_OUT/post_route_timing_summary.rpt
report_timing -sort_by group -max_paths 100 -path_type summary -file $SYNTH_OUT/post_route_timing.rpt
report_clock_utilization -file $SYNTH_OUT/clock_util.rpt
report_utilization -file $SYNTH_OUT/post_route_util.rpt
report_power -file $SYNTH_OUT/post_route_power.rpt
report_drc -file $SYNTH_OUT/post_imp_drc.rpt
write_verilog -force $SYNTH_OUT/rastericer_impl_netlist.v
write_xdc -no_fixed_only -force $SYNTH_OUT/rastericer_impl.xdc
#
# STEP#5: generate a bitstream
#
write_bitstream -force $SYNTH_OUT/rastericer.bit
