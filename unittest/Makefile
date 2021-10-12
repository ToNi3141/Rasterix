PROJ = Rasterizer

all: simulation simulationDisplay

clean:
	rm -rf obj_dir

simulation:
	verilator -DUNITTEST -CFLAGS -std=c++11 --cc -exe ../rtl/RasteriCEr/PreCommandParser.v --top-module PreCommandParser cpp/sim_PreCommandParser.cpp
	-make -C obj_dir -f VPreCommandParser.mk
	./obj_dir/VPreCommandParser

simulationDisplay:
	verilator -DUNITTEST -CFLAGS -std=c++11 --cc -exe ../rtl/Display/DisplayController8BitILI9486.v --top-module DisplayController8BitILI9486 cpp/sim_DisplayController8BitILI9486.cpp
	-make -C obj_dir -f VDisplayController8BitILI9486.mk
	./obj_dir/VDisplayController8BitILI9486

.SECONDARY:
.PHONY: all clean
