CXX = g++
CXXFLAGS = -Wall -Werror
BINPATH = bin
SRCPATH = src
MAINFILE = NSGA2

build: createBin
	$(CXX) $(CXXFLAGS) $(SRCPATH)/$(MAINFILE).cpp -o $(BINPATH)/$(MAINFILE)

run:
	./$(BINPATH)/$(MAINFILE)

buildAndRun: build run

createBin:
	mkdir -p $(BINPATH)

clean:
	rm -rf bin
