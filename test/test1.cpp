#include "../include/vjson.h"
#include <string>
#include <iostream>

int main()
{
    //定义Json变量并初始化
    vjson::Json js_val1 = vjson::Json::Object {
        {"key1","value"},
        {"key2",123},
        {"key3",false},
        {"key4",vjson::Json::Array {4,false,"abc"}},
        {"key5",vjson::Json::Object {{"key1",nullptr}}}
    };
    
    //根据Json变量产生json格式的字符串
    std::string js_str1 = js_val1.format();
    std::cout<<js_str1<<std::endl;
    
    std::cout<<std::endl;

    //解析json字符串
    std::string err;//出错信息存放
    vjson::Json js_val2 = vjson::Json::Parse(js_str1,err);
    std::cout<<"错误信息err:"<<std::endl
        <<err<<std::endl;
    std::cout<<"js_val2转换为json字符串:"<<std::endl
        <<js_val2.format()<<std::endl;

    std::cout<<std::endl;

    //手写json字符串
    std::string js_str2 = "{\"key1\":\"喂，\",\"key2\":\"妖妖灵吗？\"}";
    //解析成Json变量
    vjson::Json js_val3 = vjson::Json::Parse(js_str2,err);
    //从Json变量中读取出值
    std::cout<<js_val3["key1"].get_string();
    std::cout<<js_val3["key2"].get_string();
    std::cout<<std::endl;

    return 0;
}

