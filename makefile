CXX = clang++
FPATH = -F/Library/Frameworks
CCFLAGS = -g -c -ffast-math -Wall -std=c++11 $(FPATH)
LDFLAGS = $(FPATH)
SRC = main.cpp timing.cpp
OBJ = $(SRC:.cpp=.o)
EXE = run

all: $(SRC) $(EXE)

$(EXE): $(OBJ)
	$(CXX) $(LDFLAGS) $(OBJ) -o $@

.cpp.o:
	$(CXX) $(CCFLAGS) $< -o $@

tidy:
	rm *.o
	
clean:
	rm *.o src/*.o && rm $(EXE)