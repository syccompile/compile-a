CXX = clang++
CXXFLAGS = -std=c++14
BISON = bison
FLEX = flex

TARGET = test

AST_SOURCE = ast.cc

BISON_SOURCE = analyzer.y
BISON_OUT_CPP = analyzer.cc
BISON_OUTPUT = analyzer.cc analyzer.hh
FLEX_SOURCE = flex.l
FLEX_OUTPUT = flex.cc

TARGET : BISON_OUTPUT FLEX_OUTPUT 
	$(CXX) $(CXXFLAGS) $(BISON_OUT_CPP) $(FLEX_OUTPUT) $(AST_SOURCE) debug.cc -o $(TARGET) -g

BISON_OUTPUT : 
	$(BISON) -d $(BISON_SOURCE) -o $(BISON_OUT_CPP)

FLEX_OUTPUT : 
	$(FLEX)  -o $(FLEX_OUTPUT) $(FLEX_SOURCE)

test : TARGET
	./test < test.txt

.PHONY: clean
clean :
	rm $(TARGET) $(BISON_OUTPUT) $(FLEX_OUTPUT)


