#include "../include/vjson.h"

using std::make_shared;
using std::vector;
using std::string;
using std::shared_ptr;
using std::map;

namespace vjson{

/*****************************格式化*************************************/
static void format(std::nullptr_t,string &out){
    out += "null";
}
static void format(bool value,string &out){
    if(value)   out += "true";
    else        out += "false";
}

static void format(double value,string &out){
    char buf[32];
    //double的精度在小数后16-17位，这里我用16位
    snprintf(buf,sizeof(buf),"%.16g",value);
    out += buf;
}

static void format(const string &value,string &out){
    out += '"';
    for(size_t i=0;i<value.length();i++){
        const char ch = value[i];
        if(ch == '\\'){
            out += "\\\\";
        } else if(ch == '"'){
            out += "\\\"";
        } else if(ch == '\b'){
            out += "\\b";
        } else if(ch == '\f'){
            out += "\\f";
        } else if(ch == '\n'){
            out += "\\n";
        } else if(ch == '\r'){
            out += "\\r";
        } else if(ch == '\t'){
            out += "\\t";
        } else if(static_cast<unsigned char>(ch) <= 0x1f){
            char buf[8];
            snprintf(buf,sizeof(buf),"\\u%04x",ch);
            out += buf;
        } else{
            out += ch;
        }
    }
    out += '"';
}

static void format(const Json::Array &value,string &out){
    out += '[';
    bool flag = false;
    for(const auto &val:value){
        if(flag) out += ',';
        val.format(out);
        flag = true;
    }
    out += ']';
}

static void format(const Json::Object &value,string &out){
    out += '{';
    bool flag = false;
    for(const auto &obj:value){
        if(flag) out += ',';
        format(obj.first,out);
        out += ':';
        obj.second.format(out);
        flag += true;
    }
    out += '}';
}
/************************************************************************/

//用于对JsonValue进行扩展，以适应json的数据类型
template<Json::JsonType tag,class T>
class Value:public JsonValue{
protected:
    const T m_value;
    explicit Value(const T &value) : m_value(value){}
    
    //对纯虚函数进行定义
    Json::JsonType myType() const override {return tag;}
    void format(string &out) const override {vjson::format(m_value,out);}
};


class JsonNull final:public Value<Json::NUL,std::nullptr_t>{
public:
    explicit JsonNull() : Value(nullptr){}
};

class JsonNumber final:public Value<Json::NUMBER,double>{
    double get_number() const override {return m_value;}
public:
    explicit JsonNumber(double value) : Value(value){}
};

class JsonBoolean final:public Value<Json::BOOL, bool>{
    bool get_bool() const override {return m_value;}
public:
    JsonBoolean(bool value):Value(value){}
};

class JsonString final:public Value<Json::STRING, string>{
    const string & get_string() const override {return m_value;}
public:
    JsonString(const string &value):Value(value){}
};

class JsonArray final:public Value<Json::ARRAY, Json::Array>{
    const Json::Array & get_array() const override {return m_value;}
public:
    JsonArray(const Json::Array &value):Value(value){}
};

class JsonObject final:public Value<Json::OBJECT, Json::Object>{
    const Json::Object & get_object() const override {return m_value;}
public:
    JsonObject(const Json::Object &value):Value(value){}
};


//用于初始化
struct Statics{
    Statics() = default;
    Statics(const Statics &) = delete;
    ~Statics() = default;
    const shared_ptr<JsonValue> null = make_shared<JsonNull>();//子类转基类我们认为是安全可行的
    const shared_ptr<JsonValue> t = make_shared<JsonBoolean>(true);
    const shared_ptr<JsonValue> f = make_shared<JsonBoolean>(false); 
    const string empty_str;
    const Json::Array empty_array;
    const Json::Object empty_obj;
};
static const Statics sg_init;

/***********************************************class Json**********************************************/

/*Constructors*/
Json::Json() noexcept                   :v_ptr(sg_init.null){}
Json::Json(std::nullptr_t) noexcept     :v_ptr(sg_init.null){}
Json::Json(bool value)                  :v_ptr(value?sg_init.t:sg_init.f){}
Json::Json(int value)                   :Json((double)value){}
Json::Json(double value)                :v_ptr(make_shared<JsonNumber>(value)){}
Json::Json(const string &value)         :v_ptr(make_shared<JsonString>(value)){}
Json::Json(const char *value)           :Json(string(value)){}
Json::Json(const Array &value)          :v_ptr(make_shared<JsonArray>(value)){}
Json::Json(const Object &value)         :v_ptr(make_shared<JsonObject>(value)){}


Json::JsonType          Json::myType()              const {return v_ptr->myType();}
bool                    Json::get_bool()            const {return v_ptr->get_bool();}
double                  Json::get_number()          const {return v_ptr->get_number();}
const string &          Json::get_string()          const {return v_ptr->get_string();}
const Json::Array &     Json::get_array()           const {return v_ptr->get_array();}
const Json::Object &    Json::get_object()          const {return v_ptr->get_object();}

void Json::format(string &out) const   {v_ptr->format(out);}

Json Json::Parse(const string &in,string &err)
{   
    return nullptr;
}
/********************************************class JsonValue********************************************/
bool                    JsonValue::get_bool()               const {return false;}
double                  JsonValue::get_number()             const {return 0.0;}
const string &          JsonValue::get_string()             const {return sg_init.empty_str;}
const Json::Array &     JsonValue::get_array()              const {return sg_init.empty_array;}
const Json::Object &    JsonValue::get_object()             const {return sg_init.empty_obj;}

};//end namespace vjson

