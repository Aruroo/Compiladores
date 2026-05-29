all: carta

carta: lex.yy.c parser.tab.c ast.cpp main.cpp analyzer.cpp codegen.c
	g++ lex.yy.c parser.tab.c ast.cpp main.cpp analyzer.cpp codegen.c -o carta

lex.yy.c: scanner.l
	flex scanner.l

parser.tab.c parser.tab.h: parser.y
	bison -d parser.y

clean:
	rm -f lex.yy.c parser.tab.c parser.tab.h carta salida.fis25