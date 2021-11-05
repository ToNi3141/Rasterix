## Arty Board Constraints

## Clock
set_property -dict {PACKAGE_PIN E3  IOSTANDARD LVCMOS33} [get_ports {clk100}];
create_clock -add -name sys_clk_pin -period 10.00 \
    -waveform {0 5} [get_ports {clk100}];

## LEDs
#set_property -dict {PACKAGE_PIN H5  IOSTANDARD LVCMOS33} [get_ports {led[0]}]; 
#set_property -dict {PACKAGE_PIN J5  IOSTANDARD LVCMOS33} [get_ports {led[1]}];
#set_property -dict {PACKAGE_PIN T9  IOSTANDARD LVCMOS33} [get_ports {led[2]}];
#set_property -dict {PACKAGE_PIN T10 IOSTANDARD LVCMOS33} [get_ports {led[3]}];

# TODO: Ugly but works for now...
set_property CLOCK_DEDICATED_ROUTE FALSE [get_nets serial_sck_IBUF];

## Pmod Header JA
set_property -dict { PACKAGE_PIN G13   IOSTANDARD LVCMOS33 } [get_ports { serial_sck }]; #IO_0_15 Sch=ja[1]
set_property -dict { PACKAGE_PIN B11   IOSTANDARD LVCMOS33 } [get_ports { serial_mosi }]; #IO_L4P_T0_15 Sch=ja[2]
set_property -dict { PACKAGE_PIN A11   IOSTANDARD LVCMOS33 } [get_ports { tft_dc }]; #IO_L4N_T0_15 Sch=ja[3]
set_property -dict { PACKAGE_PIN D12   IOSTANDARD LVCMOS33 } [get_ports { tft_cs }]; #IO_L6P_T0_15 Sch=ja[4]
set_property -dict { PACKAGE_PIN D13   IOSTANDARD LVCMOS33 } [get_ports { serial_tft_mux }]; #IO_L6N_T0_VREF_15 Sch=ja[7]
set_property -dict { PACKAGE_PIN B18   IOSTANDARD LVCMOS33 } [get_ports { serial_cs }]; #IO_L10P_T1_AD11P_15 Sch=ja[8]
set_property -dict { PACKAGE_PIN A18   IOSTANDARD LVCMOS33 } [get_ports { resetn_in }]; #IO_L10N_T1_AD11N_15 Sch=ja[9]
set_property -dict { PACKAGE_PIN K16   IOSTANDARD LVCMOS33 } [get_ports { serial_cts }]; #IO_25_15 Sch=ja[10]

## Pmod Header JB
set_property -dict { PACKAGE_PIN E15   IOSTANDARD LVCMOS33 } [get_ports { display_sck }]; #IO_L11P_T1_SRCC_15 Sch=jb_p[1]
set_property -dict { PACKAGE_PIN E16   IOSTANDARD LVCMOS33 } [get_ports { display_mosi }]; #IO_L11N_T1_SRCC_15 Sch=jb_n[1]
# set_property -dict { PACKAGE_PIN D15   IOSTANDARD LVCMOS33 } [get_ports { jb[2] }]; #IO_L12P_T1_MRCC_15 Sch=jb_p[2]
# set_property -dict { PACKAGE_PIN C15   IOSTANDARD LVCMOS33 } [get_ports { jb[3] }]; #IO_L12N_T1_MRCC_15 Sch=jb_n[2]
set_property -dict { PACKAGE_PIN J17   IOSTANDARD LVCMOS33 } [get_ports { display_cs }]; #IO_L23P_T3_FOE_B_15 Sch=jb_p[3]
set_property -dict { PACKAGE_PIN J18   IOSTANDARD LVCMOS33 } [get_ports { display_reset }]; #IO_L23N_T3_FWE_B_15 Sch=jb_n[3]
set_property -dict { PACKAGE_PIN K15   IOSTANDARD LVCMOS33 } [get_ports { display_dc }]; #IO_L24P_T3_RS1_15 Sch=jb_p[4]
# set_property -dict { PACKAGE_PIN J15   IOSTANDARD LVCMOS33 } [get_ports { jb[7] }]; #IO_L24N_T3_RS0_15 Sch=jb_n[4]