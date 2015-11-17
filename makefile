CXX = clang
CCFLAGS = -g -c -ffast-math -Wall
SRC = main.cpp lib/timing.cpp
OBJ = $(SRC:.cpp=.o)
EXE = run

all: $(SRC) $(EXE)

$(EXE): $(OBJ)
	$(CXX) $(OBJ) -o $@

.cpp.o:
	$(CXX) $(CCFLAGS) $< -o $@

tidy:
	rm *.o
	
clean:
	rm *.o src/*.o && rm $(EXE)