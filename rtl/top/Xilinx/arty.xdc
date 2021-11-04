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

## Pmod Header JA
set_property -dict { PACKAGE_PIN A18   IOSTANDARD LVCMOS33 } [get_ports { resetn_in }]; #IO_L10N_T1_AD11N_15 Sch=ja[9]

## ChipKit Outer Digital Header
#set_property -dict { PACKAGE_PIN V15   IOSTANDARD LVCMOS33 } [get_ports { ck_io0  }]; #IO_L16P_T2_CSI_B_14          Sch=ck_io[0]
#set_property -dict { PACKAGE_PIN U16   IOSTANDARD LVCMOS33 } [get_ports { ck_io1  }]; #IO_L18P_T2_A12_D28_14        Sch=ck_io[1]
set_property -dict { PACKAGE_PIN P14   IOSTANDARD LVCMOS33 } [get_ports { ck_io[2]  }]; #IO_L8N_T1_D12_14             Sch=ck_io[2]
set_property -dict { PACKAGE_PIN T11   IOSTANDARD LVCMOS33 } [get_ports { ck_io[3]  }]; #IO_L19P_T3_A10_D26_14        Sch=ck_io[3]
set_property -dict { PACKAGE_PIN R12   IOSTANDARD LVCMOS33 } [get_ports { ck_io[4]  }]; #IO_L5P_T0_D06_14             Sch=ck_io[4]
set_property -dict { PACKAGE_PIN T14   IOSTANDARD LVCMOS33 } [get_ports { ck_io[5]  }]; #IO_L14P_T2_SRCC_14           Sch=ck_io[5]
set_property -dict { PACKAGE_PIN T15   IOSTANDARD LVCMOS33 } [get_ports { ck_io[6]  }]; #IO_L14N_T2_SRCC_14           Sch=ck_io[6]
set_property -dict { PACKAGE_PIN T16   IOSTANDARD LVCMOS33 } [get_ports { ck_io[7]  }]; #IO_L15N_T2_DQS_DOUT_CSO_B_14 Sch=ck_io[7]
set_property -dict { PACKAGE_PIN N15   IOSTANDARD LVCMOS33 } [get_ports { ck_io[0]  }]; #IO_L11P_T1_SRCC_14           Sch=ck_io[8]
set_property -dict { PACKAGE_PIN M16   IOSTANDARD LVCMOS33 } [get_ports { ck_io[1]  }]; #IO_L10P_T1_D14_14            Sch=ck_io[9]
#set_property -dict { PACKAGE_PIN V17   IOSTANDARD LVCMOS33 } [get_ports { ck_io10 }]; #IO_L18N_T2_A11_D27_14        Sch=ck_io[10]
#set_property -dict { PACKAGE_PIN U18   IOSTANDARD LVCMOS33 } [get_ports { ck_io11 }]; #IO_L17N_T2_A13_D29_14        Sch=ck_io[11]
#set_property -dict { PACKAGE_PIN R17   IOSTANDARD LVCMOS33 } [get_ports { ck_io12 }]; #IO_L12N_T1_MRCC_14           Sch=ck_io[12]
#set_property -dict { PACKAGE_PIN P17   IOSTANDARD LVCMOS33 } [get_ports { ck_io13 }]; #IO_L12P_T1_MRCC_14           Sch=ck_io[13]

## ChipKit Outer Analog Header - as Single-Ended Analog Inputs
## NOTE: These ports can be used as single-ended analog inputs with voltages from 0-3.3V (ChipKit analog pins A0-A5) or as digital I/O.
## WARNING: Do not use both sets of constraints at the same time!
## NOTE: The following constraints should be used with the XADC IP core when using these ports as analog inputs.
#set_property -dict { PACKAGE_PIN C5    IOSTANDARD LVCMOS33 } [get_ports { vaux4_n  }]; #IO_L1N_T0_AD4N_35     Sch=ck_an_n[0] ChipKit pin=A0
#set_property -dict { PACKAGE_PIN C6    IOSTANDARD LVCMOS33 } [get_ports { vaux4_p  }]; #IO_L1P_T0_AD4P_35     Sch=ck_an_p[0] ChipKit pin=A0
#set_property -dict { PACKAGE_PIN A5    IOSTANDARD LVCMOS33 } [get_ports { vaux5_n  }]; #IO_L3N_T0_DQS_AD5N_35 Sch=ck_an_n[1] ChipKit pin=A1
#set_property -dict { PACKAGE_PIN A6    IOSTANDARD LVCMOS33 } [get_ports { vaux5_p  }]; #IO_L3P_T0_DQS_AD5P_35 Sch=ck_an_p[1] ChipKit pin=A1
#set_property -dict { PACKAGE_PIN B4    IOSTANDARD LVCMOS33 } [get_ports { vaux6_n  }]; #IO_L7N_T1_AD6N_35     Sch=ck_an_n[2] ChipKit pin=A2
#set_property -dict { PACKAGE_PIN C4    IOSTANDARD LVCMOS33 } [get_ports { vaux6_p  }]; #IO_L7P_T1_AD6P_35     Sch=ck_an_p[2] ChipKit pin=A2
#set_property -dict { PACKAGE_PIN A1    IOSTANDARD LVCMOS33 } [get_ports { vaux7_n  }]; #IO_L9N_T1_DQS_AD7N_35 Sch=ck_an_n[3] ChipKit pin=A3
#set_property -dict { PACKAGE_PIN B1    IOSTANDARD LVCMOS33 } [get_ports { vaux7_p  }]; #IO_L9P_T1_DQS_AD7P_35 Sch=ck_an_p[3] ChipKit pin=A3
#set_property -dict { PACKAGE_PIN B2    IOSTANDARD LVCMOS33 } [get_ports { vaux15_n }]; #IO_L10N_T1_AD15N_35   Sch=ck_an_n[4] ChipKit pin=A4
#set_property -dict { PACKAGE_PIN B3    IOSTANDARD LVCMOS33 } [get_ports { vaux15_p }]; #IO_L10P_T1_AD15P_35   Sch=ck_an_p[4] ChipKit pin=A4
#set_property -dict { PACKAGE_PIN C14   IOSTANDARD LVCMOS33 } [get_ports { vaux0_n  }]; #IO_L1N_T0_AD0N_15     Sch=ck_an_n[5] ChipKit pin=A5
#set_property -dict { PACKAGE_PIN D14   IOSTANDARD LVCMOS33 } [get_ports { vaux0_p  }]; #IO_L1P_T0_AD0P_15     Sch=ck_an_p[5] ChipKit pin=A5
## ChipKit Outer Analog Header - as Digital I/O
## NOTE: The following constraints should be used when using these ports as digital I/O.
set_property -dict { PACKAGE_PIN F5    IOSTANDARD LVCMOS33 } [get_ports { ck_a0 }]; #IO_0_35            Sch=ck_a[0]
set_property -dict { PACKAGE_PIN D8    IOSTANDARD LVCMOS33 } [get_ports { ck_a1 }]; #IO_L4P_T0_35       Sch=ck_a[1]
set_property -dict { PACKAGE_PIN C7    IOSTANDARD LVCMOS33 } [get_ports { ck_a2 }]; #IO_L4N_T0_35       Sch=ck_a[2]
set_property -dict { PACKAGE_PIN E7    IOSTANDARD LVCMOS33 } [get_ports { ck_a3 }]; #IO_L6P_T0_35       Sch=ck_a[3]
set_property -dict { PACKAGE_PIN D7    IOSTANDARD LVCMOS33 } [get_ports { ck_a4 }]; #IO_L6N_T0_VREF_35  Sch=ck_a[4]
set_property -dict { PACKAGE_PIN D5    IOSTANDARD LVCMOS33 } [get_ports { ck_a5 }]; #IO_L11P_T1_SRCC_35 Sch=ck_a[5]

##Pmod Header JC
set_property -dict { PACKAGE_PIN U12    IOSTANDARD LVCMOS33 } [get_ports { ftRxf }]; 
set_property -dict { PACKAGE_PIN V12    IOSTANDARD LVCMOS33 } [get_ports { ftRd }];
set_property -dict { PACKAGE_PIN V10    IOSTANDARD LVCMOS33 } [get_ports { ftClk }]; 

##Pmod Header JD
set_property -dict { PACKAGE_PIN D4   IOSTANDARD LVCMOS33 } [get_ports { ftData[0] }];
set_property -dict { PACKAGE_PIN D3   IOSTANDARD LVCMOS33 } [get_ports { ftData[1] }];
set_property -dict { PACKAGE_PIN F4   IOSTANDARD LVCMOS33 } [get_ports { ftData[2] }];
set_property -dict { PACKAGE_PIN F3   IOSTANDARD LVCMOS33 } [get_ports { ftData[3] }];
set_property -dict { PACKAGE_PIN E2   IOSTANDARD LVCMOS33 } [get_ports { ftData[4] }];
set_property -dict { PACKAGE_PIN D2   IOSTANDARD LVCMOS33 } [get_ports { ftData[5] }];
set_property -dict { PACKAGE_PIN H2   IOSTANDARD LVCMOS33 } [get_ports { ftData[6] }];
set_property -dict { PACKAGE_PIN G2   IOSTANDARD LVCMOS33 } [get_ports { ftData[7] }];
