CXX = clang++
CXXFLAGS = -std=c++11 -g
DEFINE = -D CONFIG_ARM
BISON = bison
FLEX = flex

TARGET = output

TEST_TXT = example.txt

CXX_SOURCE = analyzer.cc flex.cc debug.cc ast.cc symtab.cc ir_translator.cc frame.cc main.cc 
OBJS = analyzer.o ast.o debug.o flex.o symtab.o ir_translator.o frame.o main.o

BISON_SOURCE = analyzer.y
BISON_FLAG  = -Wcounterexamples
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
	$(CXX) $(CXXFLAGS) -c $^ -o $@ $(DEFINE)

.PHONY: test
test : $(TARGET)
	./$(TARGET) < $(TEST_TXT)

.PHONY: clean
clean :
	rm $(TARGET) $(BISON_OUTPUT) $(FLEX_OUTPUT) $(OBJS)
