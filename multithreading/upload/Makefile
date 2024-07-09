EXE=vector matrix

all: clean $(EXE)

%: %.cpp
	g++ -O3 -std=c++11 -o $@ $^ -lpthread

clean:
	rm -rf $(EXE) 2>/dev/null
