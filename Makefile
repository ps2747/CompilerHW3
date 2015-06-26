CXX = g++

CXXFLAGS = -std=c++0x -g -I./inc
LIBS = 
OUT_EXE = compile

all: $(OUT_EXE)

$(OUT_EXE): main.o symbol_table.o ll_code_gen.o TreeAnalizer.o TreeNode.o
	$(CXX) $^ $(LIBS) -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $<

.PHONY: clean
clean:
	rm *.o $(OUT_EXE)
