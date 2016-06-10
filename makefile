obj_dir = ./obj
lib_dir = ./lib/bin
source_dirs = ./ debug
include_dirs = $(source_dirs)
include_dirs += lib/include
search_cpp_wildcards = $(addsuffix /*.cpp, $(source_dirs)) 
cpp_obj = $(addprefix $(obj_dir)/, $(notdir $(patsubst %.cpp, $(obj_dir)/%.o, $(wildcard $(search_cpp_wildcards)))))
cflags = `pkg-config --cflags glib-2.0 gstreamer-1.0`
cflags += -std=c++0x -Wall -O3
lflags = `pkg-config --libs glib-2.0 gstreamer-1.0 `
lflags +=   -lgstapp-1.0  -laubio -lboost_filesystem

create_obj_dir := $(shell mkdir -p obj)

all: bpm

bpm: $(c_obj) $(cpp_obj)
	g++ $^ -o $@ -L$(lib_dir) $(lflags)

VPATH := $(source_dirs)	  

$(obj_dir)/%.o: %.cpp
	gcc -c -MD $(addprefix -I, $(include_dirs)) $(cflags) -o $@ $<

include $(wildcard $(obj_dir)/*.d) 

clean: 
	rm -f $(obj_dir)/*.o $(obj_dir)/*.d ./bpm
