# Flags
CXX := g++
CXXFLAGS := -Wall -Wextra -pedantic -std=c++2a
OBJS := gameMaster.o equipo.o config.o barrera.o

# Phonies
.PHONY: all clean

all: juego 

clean: 
	rm -f *.o
	rm -f tests/*.o
	rm -f juego

# Game
juego: main.cpp $(OBJS) # Executable
	$(CXX) $(CXXFLAGS) $^ -o $@ -lpthread 

config.o: config.cpp config.h
	$(CXX) $(CXXFLAGS) -c $< -o $@ 

equipo.o: equipo.cpp equipo.h 
	$(CXX) $(CXXFLAGS) -c $< -o $@ 

gameMaster.o: gameMaster.cpp gameMaster.h 
	$(CXX) $(CXXFLAGS) -c $< -o $@ 

barrera.o: barrera.cpp barrera.h
	$(CXX) $(CXXFLAGS) -c $< -o $@
