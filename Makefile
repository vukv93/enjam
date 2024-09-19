# High level development interface, see CMakeLists.txt for details.
all: cmake
cmake:
	mkdir -p build
	cd build && cmake ../ && make -j24
clean:
	rm -rf build
rebuild:
	make clean && make
test: all
	./build/test/test
html: 
	doxygen
	cp -r doc build/doc/html
doc: html
	cp -r doc build/doc/latex
	cd build/doc/latex && pdflatex refman.tex
vhtml:
	firefox build/doc/html/index.html
vpdf:
	zathura build/doc/latex/refman.pdf
memtest: cmake
	valgrind --leak-check=full --show-leak-kinds=all ./build/test/test
.PHONY: clean rebuild doc test
