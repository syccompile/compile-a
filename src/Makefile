CC = g++
BISON = bison
FLEX = flex

TARGET = test

AST_SOURCE = ast.cc

BISON_SOURCE = analyzer.y
BISON_OUT_CPP = analyzer.tab.cc
BISON_OUTPUT = analyzer.tab.cc analyzer.tab.hh

FLEX_SOURCE = flex.l
FLEX_OUTPUT = flex.cc

TARGET : BISON_OUTPUT FLEX_OUTPUT 
	$(CC) $(BISON_OUT_CPP) $(FLEX_OUTPUT) $(AST_SOURCE) -o $(TARGET)

BISON_OUTPUT : 
	$(BISON) -d $(BISON_SOURCE) -o $(BISON_OUT_CPP)

FLEX_OUTPUT : 
	$(FLEX)  -o $(FLEX_OUTPUT) $(FLEX_SOURCE)

.PHONY: clean
clean :
	rm $(TARGET) $(BISON_OUTPUT) $(FLEX_OUTPUT)


