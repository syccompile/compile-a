CXX = g++
CXXFLAGS = -std=c++11 -g
BISON = bison
FLEX = flex

TARGET = output

TEST_TXT = example.txt

OBJS = analyzer.o ast.o debug.o flex.o ast_to_ir.o main.o context/types.o context/vartab.o context/context.o

BISON_SOURCE = analyzer.y
BISON_FLAG  = -Wall
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
