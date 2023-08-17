CXX   = g++-8
EXE = ipl_parser
CXXDEBUG = -g -Wall
CXXSTD = -std=c++11


.PHONY: all
all: parser lexer 	symtab.cpp driver.cpp ast.cpp
	$(CXX) $(CXXDEBUG) $(CXXSTD) -o iplC symtab.cpp driver.cpp utils.cpp ast.cpp parser.o scanner.o 

parser: parser.yy scanner.hh
	bison -d -v $<
	$(CXX) $(CXXDEBUG) $(CXXSTD) -c parser.tab.cc -o parser.o 

lexer: scanner.l scanner.hh parser.tab.hh parser.tab.cc	
	flex++ --outfile=scanner.yy.cc  $<
	$(CXX)  $(CXXDEBUG) $(CXXSTD) -c scanner.yy.cc -o scanner.o

clean:
	rm -f iplC
	rm -f parser.o
	rm -f parser.output
	rm -f parser.tab.cc
	rm -f parser.tab.hh
	rm -f scanner.o
	rm -f scanner.yy.cc
	rm -f stack.hh

