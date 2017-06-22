#ifndef __VJSON_H__
#define __VJSON_H__ 
#include <string>
#include <vector>
#include <memory>
#include <map>

namespace vjson{

class JsonValue;

class Json final{
public:
    //定义json的所有类型
    enum JsonType{
        NUL, BOOL, NUMBER, STRING, ARRAY, OBJECT
    };
    
    typedef std::map<std::string,Json> Object;
    typedef std::vector<Json>   Array;

    //构造函数
    Json() noexcept;                   //null
    Json(std::nullptr_t) noexcept;     //null
    Json(bool value);                  //bool
    Json(int value);                   //number int
    Json(double value);                //number double
    Json(const std::string &value);    //string
    Json(const char *value);           //string
    Json(const Array &value);          //array
    Json(const Object &value);         //object
    Json(const Json &) = default;      //copy constructor

    //判断Json存储的类型
    JsonType myType() const;
    
    bool is_null()  const {return myType() == NUL;}
    bool is_bool()  const {return myType() == BOOL;}
    bool is_number() const {return myType() == NUMBER;}
    bool is_string() const {return myType() == STRING;}
    bool is_array()  const {return myType() == ARRAY;}
    bool is_object() const {return myType() == OBJECT;}

    //返回布尔型
    bool get_bool() const;
    //返回double
    double get_number() const;
    //返回string
    const std::string & get_string() const;
    //返回array
    const Array & get_array() const;
    //返回Object
    const Object & get_object() const;

    //格式化json
    void format(std::string &out) const;
    std::string format() const {
        std::string out;
        format(out);
        return out;
    }
    
    //解析,如果解析失败，返回Json(),并设置err
    static Json Parse(const std::string &in,std::string &err);
    static Json Parse(const char *in,std::string &err){
        if(in){
            return Parse(std::string(in),err);
        }
        err = "null input";
        return nullptr;
    }

private: 
    std::shared_ptr<JsonValue> v_ptr; 

};//end class Json

//不向用户提供API,用于内部
class JsonValue{
    friend class Json;
    //friend class JsonNum;
protected:
    //virtual double get_num() const;
    virtual Json::JsonType myType() const = 0;
    virtual void format(std::string &out) const = 0;
    virtual bool get_bool() const;
    virtual double get_number() const;
    virtual const std::string & get_string() const;
    virtual const Json::Array & get_array() const;
    virtual const Json::Object & get_object() const;
    virtual ~JsonValue(){}
};


}//end namespace vjson

#endif
