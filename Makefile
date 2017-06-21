CXX = g++

json:include/vjson.h test/test1.cpp
	$(CXX) -o json include/vjson.h test/test1.cpp

.PHONY:clean
clean:
	rm -f json
