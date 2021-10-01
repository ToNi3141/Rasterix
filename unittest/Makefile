PROJ = Rasterizer

all: simulation 

clean:
	rm -rf obj_dir

simulation:
	verilator -DUNITTEST -CFLAGS -std=c++11 --cc -exe ../rtl/RasteriCEr/PreCommandParser.v --top-module PreCommandParser cpp/sim_PreCommandParser.cpp
	-make -C obj_dir -f VPreCommandParser.mk
	./obj_dir/VPreCommandParser

.SECONDARY:
.PHONY: all clean
