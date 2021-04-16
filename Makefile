server : proj1.o
	g++ proj1.o -o server
proj1.o : proj1.cpp
	g++ -c proj1.cpp
