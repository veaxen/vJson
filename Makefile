CXX = g++

json:include/vjson.h test/test1.cpp src/vjson.cpp
	$(CXX) -o json test/test1.cpp src/vjson.cpp -std=c++11

.PHONY:clean
clean:
	rm -f json
