all: blossom

blossom: main.cpp Graph.h Graph.cpp Makefile
	clang++ -O3 -Wall -Wextra -std=c++17 main.cpp Graph.cpp -o blossom

testdot: all
	./blossom < in 2> out.gv
	dot -Tpdf out.gv | csplit --quiet --elide-empty-files --prefix=tmpfile --suffix=%02d.pdf - "/%%EOF/+1" "{*}"
	pdfjoin tmpfile*.pdf -o out.pdf
	rm tmpfile*.pdf

compout:
	dot -Tpdf out.gv | csplit --quiet --elide-empty-files --prefix=tmpfile --suffix=%02d.pdf - "/%%EOF/+1" "{*}"
	pdfjoin tmpfile*.pdf -o out.pdf
	rm tmpfile*.pdf
