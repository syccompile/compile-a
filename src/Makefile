CXX = clang++
CXXFLAGS = -std=c++11 -g
BISON = bison
FLEX = flex

TARGET = output

CXX_SOURCE = analyzer.cc flex.cc debug.cc ast.cc
OBJS = analyzer.o ast.o debug.o flex.o

BISON_SOURCE = analyzer.y
BISON_FLAG  = -Wcounterexamples
BISON_OUT_CPP = analyzer.cc
BISON_OUTPUT = analyzer.cc analyzer.hh
FLEX_SOURCE = flex.l
FLEX_OUTPUT = flex.cc

$(TARGET) : $(OBJS)
	$(CXX) $^ -o $@
analyzer.cc : $(BISON_SOURCE)
	$(BISON) -d $^ -o $@
flex.cc : $(FLEX_SOURCE)
	$(FLEX) -o $@ $^
%.o : %.cc
	$(CXX) -c $^ -o $@

.PHONY: test
test : $(TARGET)
	./$(TARGET) < test.txt

.PHONY: clean
clean :
	rm $(TARGET) $(BISON_OUTPUT) $(FLEX_OUTPUT) $(OBJS)
