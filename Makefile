CXX := g++
CXXFLAGS := -Wall -Wextra -pedantic -std=c++2a

.PHONY: all clean

all: juego

clean: 
	rm -f *.o
	rm -f juego

juego: main.o gameMaster.o equipo.o config.o
	$(CXX) $(CXXFLAGS) $^ -o $@ -lpthread 

config.o: config.cpp config.h definiciones.h
	$(CXX) $(CXXFLAGS) -c $< -o $@ 

equipo.o: equipo.cpp equipo.h definiciones.h gameMaster.h
	$(CXX) $(CXXFLAGS) -c $< -o $@ 

gameMaster.o: gameMaster.cpp gameMaster.h config.h definiciones.h
	$(CXX) $(CXXFLAGS) -c $< -o $@ 

main.o: main.cpp gameMaster.h equipo.h definiciones.h config.h
	$(CXX) $(CXXFLAGS) -c $< -o $@ 