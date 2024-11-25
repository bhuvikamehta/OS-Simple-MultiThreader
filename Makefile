EXE=vector matrix

all: clean $(EXE)

%: %.cpp simple-multithreader.h
	g++ -O3 -std=c++11 -o $@ $^ -lpthread

clean:
	rm -rf $(EXE) 2>/dev/null
