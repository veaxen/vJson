# vJson
## 简介
vJson是一个采用C++编写的json解析和生成库，编写这个库的目的是练习C++的语法，想要通过实际的项目使自己的C++代码编写能力得到提升，所以本项目比较适合想要一起学习C++语法和了解json的初学者。

由于现在C++11的许多新特性很好用，所以本项目也尽可能用一些C++11的新特性进行编写，本人技术有限，所以还望指教。

vJson的开方给用户的类定义在[include/vjson.h](https://github.com/veaxen/vJson/blob/master/include/vjson.h)中，用户只要包含此头文件就可以使用本项目对用户开方的类接口；
关于json的主要解析和生成代码源代码放在[src/vjson.cpp](https://github.com/veaxen/vJson/blob/master/src/vjson.cpp)中；
测试代码放在[test/test1.cpp](https://github.com/veaxen/vJson/blob/master/test/test1.cpp)中，在项目的根目录中执行make，便可以得到json可执行文件，运行的代码是项目中的测试代码，具体可以查看Makefile文件。
## 用法
下面代码演示了如何通过vJson生成一个json变量，这里可以直观的看出所要生成的json内容，方便阅读代码和理解。
```
//定义Json变量并初始化
vjson::Json js_val1 = vjson::Json::Object {
  {"key1","value"},
  {"key2",123},
  {"key3",false},
  {"key4",vjson::Json::Array {4,false,"abc"}},
  {"key5",vjson::Json::Object {{"key1",true}}}
};
```
下面代码演示了如何通过json变量产生文本格式的json：
```
//根据Json变量产生json格式的字符串
std::string js_str1 = js_val1.format();
std::cout<<js_str1<<std::endl;
```
下面代码演示了如何vJson解析json文本：
```
//错误信息输出
std::string err;
//定义一个json格式的字符串
std::string js_str2 = "{\"key1\":\"喂，\",\"key2\":\"妖妖灵吗？\"}";
//将字符串解析成Json变量，并将错误信息输出到err中。
vjson::Json js_val3 = vjson::Json::Parse(js_str2,err);
```
