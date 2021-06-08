CXX = clang++
CXXFLAGS = -std=c++11 -g
BISON = bison
FLEX = flex

TARGET = src/output

TEST_TXT = src/example.txt

CXX_SOURCE = src/analyzer.cc src/flex.cc src/debug.cc src/ast.cc
OBJS = src/analyzer.o src/ast.o src/debug.o src/flex.o

BISON_SOURCE = src/analyzer.y
BISON_FLAG  = -Wcounterexamples
BISON_OUT_CPP = analyzer.cc
BISON_OUTPUT = src/analyzer.cc src/analyzer.hh
FLEX_SOURCE = src/flex.l
FLEX_OUTPUT = src/flex.cc

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
	$(TARGET) < $(TEST_TXT)

.PHONY: archive
archive : clean
	cd .. && tar -cf compile.tar compile

.PHONY: clean
clean :
	rm $(TARGET) $(BISON_OUTPUT) $(FLEX_OUTPUT) $(OBJS)
