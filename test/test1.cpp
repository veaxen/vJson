#include "../include/vjson.h"
#include <string>
#include <iostream>

int main()
{
    vjson::Json json_test = vjson::Json::Object {
        {"key1","value"},
        {"key2",123},
        {"key3",false},
        {"key4",vjson::Json::Array {4,false,"abc"}}
    };
    std::cout<<json_test.format()<<std::endl;
    return 0;
}

