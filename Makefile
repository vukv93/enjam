all: cmake

cmake:
	mkdir -p build
	cd build && cmake ../ && make -j24

clean:
	rm -r build

rebuild:
	make clean
	make

doc:
	doxygen
	cp -r doc build/doc/html

.PHONY: clean rebuild doc
