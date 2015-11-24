CXX = clang
CCFLAGS = -g -c -Wall #-ffast-math
SRC = main.c src/encoding.c lib/fasthash/fasthash.c
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