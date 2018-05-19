kchain: src/kchain.cc
	c++ -o kchain src/kchain.cc --std=c++17 -O2

clean:
	rm -f kchain
