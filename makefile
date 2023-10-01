all:myfind

myfind:myfind.cpp
	g++ -std=c++17 -Wall -o myfind myfind.cpp

clean:
	rm -f myfind