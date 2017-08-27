CXX = g++

json:vjson/vjson.h test/test1.cpp vjson/vjson.cpp
	$(CXX) -o json test/test1.cpp vjson/vjson.cpp -std=c++11

.PHONY:clean
clean:
	rm -f json
