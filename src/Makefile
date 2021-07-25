CXX = g++
CXXFLAGS = -std=c++17 -g
BISON = bison
FLEX = flex

TARGET = output

TEST_TXT = example.txt

OBJS = analyzer.o ast.o debug.o flex.o ast_to_ir.o main.o\
context/types.o context/vartab.o context/context.o context/functab.o \
ir_opt/remove_redunctant_label.o ir_to_asm/translate_to_asm.o \
reg_allocate/color_graph.o reg_allocate/liveness_analyze.o  \
reg_allocate/reg_allocate.o

BISON_SOURCE = analyzer.y
BISON_FLAG  = 
BISON_OUT_CPP = analyzer.cc
BISON_OUTPUT = analyzer.cc analyzer.hh
FLEX_SOURCE = flex.l
FLEX_OUTPUT = flex.cc

$(TARGET) : $(OBJS)
	$(CXX) $^ -o $@

analyzer.cc : $(BISON_SOURCE)
	$(BISON) -d $^ -o $@ $(BISON_FLAG)
flex.cc : $(FLEX_SOURCE)
	$(FLEX) -o $@ $^

%.o : %.cc
	$(CXX) $(CXXFLAGS) -c $^ -o $@

.PHONY: test
test : $(TARGET)
	./$(TARGET) < $(TEST_TXT)

.PHONY: clean
clean :
	rm $(TARGET) $(BISON_OUTPUT) $(FLEX_OUTPUT) $(OBJS)
