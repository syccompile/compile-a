CXX = clang++
CXXFLAGS = -std=c++11 -g
DEFINE = -D CONFIG_ARM
BISON = bison
FLEX = flex
CODE_CHECK = run-clang-format.sh
CODE_FORMAT = clang-format

TARGET = output

TEST_TXT = example.txt

OBJS = analyzer.o ast.o debug.o flex.o symtab.o ir_translator.o frame.o main.o

BISON_SOURCE = analyzer.y
BISON_FLAG  = -Wcounterexamples
BISON_OUTPUT = analyzer.cpp analyzer.hpp

FLEX_SOURCE = flex.l
FLEX_OUTPUT = flex.cpp

$(TARGET) : check $(OBJS)
	$(CXX) $(OBJS) -o $@
analyzer.cpp : $(BISON_SOURCE)
	$(BISON) -d $^ -o $@ $(BISON_FLAG)
flex.cpp : $(FLEX_SOURCE)
	$(FLEX) -o $@ $^
%.o : %.cc
	$(CXX) $(CXXFLAGS) -c $^ -o $@ $(DEFINE)
analyzer.o : analyzer.cpp
	$(CXX) $(CXXFLAGS) -c $^ -o $@ $(DEFINE)
flex.o : flex.cpp
	$(CXX) $(CXXFLAGS) -c $^ -o $@ $(DEFINE)

.PHONY: test
test : $(TARGET)
	./$(TARGET) < $(TEST_TXT)

.PHONY: check
check: 
	./$(CODE_CHECK)

.PHONY: format
format:
	$(CODE_FORMAT) -i *.cc *.h

.PHONY: clean
clean :
	rm $(TARGET) $(BISON_OUTPUT) $(FLEX_OUTPUT) $(OBJS)
