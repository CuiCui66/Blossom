all: blossom

blossom: main.cpp Graph.h Graph.cpp Makefile
	g++ -g -Wall -Wextra -std=c++17 main.cpp Graph.cpp -o blossom

testdot: all
	./blossom < in > out.gv
	dot -Tpdf out.gv | csplit --quiet --elide-empty-files --prefix=tmpfile --suffix=%02d.pdf - "/%%EOF/+1" "{*}"
	pdfjoin tmpfile*.pdf -o out.pdf
	rm tmpfile*.pdf

