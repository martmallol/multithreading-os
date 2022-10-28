# Flags
CXX := g++
CXXFLAGS := -Wall -Wextra -pedantic -std=c++2a
GTESTDIR := tests/gtest-1.8.1/
OBJS := gameMaster.o equipo.o config.o barrera.o
TSTOBJS := tests/testGame.o tests/testBelcebu.o tests/testEquipo.o tests/testBarrera.o

# Phonies
.PHONY: all clean tester

all: juego tester

clean: 
	rm -f *.o
	rm -f tests/*.o
	rm -f juego
	rm -f tester

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

# Testing
tester: tests/testmain.cpp $(TSTOBJS) $(OBJS) # Executable
	$(CXX) $(CXXFLAGS) $^ -o $@ $(GTESTDIR)gtest-all.cc $(GTESTDIR)gtest.h -lpthread 

tests/testGame.o: tests/testGame.cpp equipo.h
	$(CXX) $(CXXFLAGS) -c $< -o $@ 

tests/testBelcebu.o: tests/testBelcebu.cpp gameMaster.h 
	$(CXX) $(CXXFLAGS) -c $< -o $@ 

tests/testEquipo.o: tests/testEquipo.cpp equipo.h
	$(CXX) $(CXXFLAGS) -c $< -o $@ 

tests/testBarrera.o: tests/testBarrera.cpp equipo.h
	$(CXX) $(CXXFLAGS) -c $< -o $@ 