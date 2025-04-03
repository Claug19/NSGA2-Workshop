CXX = g++
CXXFLAGS = -Wall -Werror
BINPATH = bin
SRCPATH = src
MAINFILE = NSGA2

build: createBin $(patsubst %.cpp, %.out, $(SRCPATH)/$(MAINFILE).cpp)
cleanBuild: clean build

%.out: %.cpp
	$(CXX) $(CXXFLAGS) $< -o $(BINPATH)/$(MAINFILE)

clean:
	rm -rf bin

createBin:
	mkdir -p $(BINPATH)
