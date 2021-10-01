PROJ = Rasterizer
PIN_DEF = rasterizer_up5k.pcf
DEVICE = up5k

X_RESOLUTION = 240
Y_RESOLUTION = 320
Y_LINE_RESOLUTION = 64

NEXTPNR = nextpnr-ice40
NEXTPNR_ARGS = --opt-timing  
ICEPACK = icepack
ICETIME = icetime
ICEPROG = iceprog

SRC = ./topUP5k.v
SRC += ../../RasteriCEr/SinglePortRam32k.v
SRC += ../../RasteriCEr/FrameBuffer.v
SRC += ../../RasteriCEr/Rasterizer.v
SRC += ../../RasteriCEr/RasteriCEr.v
SRC += ../../RasteriCEr/FragmentPipeline.v
SRC += ../../RasteriCEr/FragmentPipelineIce40Wrapper.v
SRC += ../../RasteriCEr/CommandParser.v
SRC += ../../RasteriCEr/Serial2AXIS.v
SRC += ../../RasteriCEr/Recip.v
SRC += ../../RasteriCEr/TextureBuffer.v
SRC += ../../Display/DisplayControllerSpi.v
SRC += ../../3rdParty/SPI_Slave.v
SRC += ../../3rdParty/sfifo.v


SYNTH_OUT = ./synth
YOSYS = yosys
NEXTPNR = nextpnr-ice40
ICEPACK = icepack
ICEPROG = iceprog
#YOSYSFLAGS = -p "synth_ice40 -top top -blif $(FPGA_BLIF_OUT) -json $(FPGA_JSON_OUT) -relut;"
YOSYSFLAGS = -p "synth_ice40 -top top -blif $(FPGA_BLIF_OUT) -json $(FPGA_JSON_OUT) -relut -abc2;" # '-abc2' can help to fit the timing an size requirements
FPGA_BLIF_OUT = $(SYNTH_OUT)/_fpga.blif
FPGA_JSON_OUT = $(SYNTH_OUT)/_fpga.json
FPGA_ASC_OUT = $(SYNTH_OUT)/_fpga.asc
FPGA_BIN_OUT = $(SYNTH_OUT)/_fpga.bin

all: rasterizer.bin

rasterizer.json:
	-mkdir $(SYNTH_OUT)
	$(YOSYS) -q $(YOSYSFLAGS) -DUP5K -DX_RESOLUTION=$(X_RESOLUTION) -DY_RESOLUTION=$(Y_RESOLUTION) -DY_LINE_RESOLUTION=$(Y_LINE_RESOLUTION) $(SRC) 

%.json: %.v 
	yosys -p 'synth_ice40 -json $@' $<

rasterizer.asc: rasterizer.json
	$(NEXTPNR) $(NEXTPNR_ARGS) --$(DEVICE) --pcf $(PIN_DEF) --asc $(FPGA_ASC_OUT) --json $(FPGA_JSON_OUT)

rasterizer.bin: rasterizer.asc
	$(ICEPACK) $(FPGA_ASC_OUT) $(FPGA_BIN_OUT)

prog: rasterizer.bin
	$(ICEPROG) -S $(FPGA_BIN_OUT)

flash: 
	$(ICEPROG) $(FPGA_BIN_OUT)

sram:
	$(ICEPROG) -S $(FPGA_BIN_OUT)

sudo-prog: $(PROJ).bin
	@echo 'Executing prog as root!!!'
	sudo $(ICEPROG) -S $<

clean:
	rm -f $(PROJ).blif $(PROJ).asc $(PROJ).rpt $(PROJ).bin

.SECONDARY:
.PHONY: all prog clean
