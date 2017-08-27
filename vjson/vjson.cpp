#include "vjson.h"
#include <limits>
#include <cmath>

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

/******************************解析**************************************/
static inline string esc(char c){
    char buf[10];
    if(static_cast<unsigned char>(c) >= 0x20 && static_cast<unsigned char>(c) <= 0x7f){
        snprintf(buf,sizeof(buf),"'%c' (%d)",c,c);
    }else{
        snprintf(buf,sizeof(buf),"(%d)",c);
    }
    return string(buf);
}

static inline bool in_range(long x,long lower,long upper){
    return (x>=lower && x<=upper);
}

//定义一个解析类，用于解析
struct JsonParser final{
    const string &str;
    string &err;
    bool failed;
    size_t i;//where

    //过滤
    size_t next_validChar(){
        while(str[i]==' ' || str[i]=='\r' || str[i]=='\n' || str[i]=='\t') 
            i++;
        return i;
    }

    Json fail(string &&msg){
        failed = true;
        err = msg;
        return Json();
    }

    Json parse_object(){
        Json::Object res;
        string key;
        i++;
        bool flag = false;
        while(true){
            if(i >= str.size())
                return fail("unexpected end of input in string");
            
            char ch = str[next_validChar()];
            
            if(ch == '}'){
                i++;
                if(res.empty())
                    return fail("the object can not be empty");
                return Json(res);
            }
            
            if(!flag){
                i--;
                flag = true;
            }else if(ch != ',')
                return fail("test expected ',' character,got " + esc(ch));
            
            i++;
            
            key = parse_string().get_string();
            if(failed)
                return Json();
            
            ch = str[next_validChar()];
            if(ch != ':')
                return fail("expected ':' character,got " + esc(ch));
            i++;
            
            res.insert(std::pair<string,Json>(key,parse_json()));
            if(failed)
                return Json();
        }
    }


    Json parse_array(){
        Json::Array res;
        i++;
        bool flag = false;
        while(true){
            if(i >= str.size())
                return fail("unexpected end of input in string");
            
            char ch = str[next_validChar()];
            
            if(ch == ']'){
                i++;
                if(res.empty())
                    return fail("the Array can not be empty");
                return Json(res);
            }
            
            if(!flag){
                i--;
                flag = true;
            }else if(ch != ',')
                return fail("expected ','  character,got " + esc(ch));
            
            i++;
            
            res.push_back(parse_json());
            if(failed){
                return Json();
            }
        }
        
    }
    
    Json parse_string(){
        string res;
        if(str[next_validChar()] != '"')
            return fail("expected value,got " + esc(str[next_validChar()]));
        i++;
        
        while(true){
            
            if(i >= str.size())
                return fail("unexpected end of input in string");
            
            char ch = str[i++];
            
            if(ch == '"'){
                return Json(res);
            }
            
            if(ch != '\\'){//大多数情况是没有转义的
                res += ch;
                continue;
            }
            
            ch = str[i++];
            if(ch == 't'){
                res += '\t';
            }else if(ch == 'r'){
                res += '\r';
            }else if(ch == 'n'){
                res += '\n';
            }else if(ch == 'b'){
                res += '\b';
            }else if(ch == '\\' || ch == '"' || ch == '?' || ch == '/'){
                res += ch;
            }else{
                return fail("invalid escape character "+esc(ch));
            }
        }
    }
    
    Json parse_number(){
        size_t pos = i;
        
        if(str[i] == '-')   
            i++;
        
        //整数
        if(str[i] == '0'){
            i++;
            if(in_range(str[i],'0','9')){
                return fail("leading 0 not permitted in numbers");
            }
        }else if(in_range(str[i],'1','9')){
            i++;
            while(in_range(str[i],'0','9')) 
                i++;
        }else{
            return fail("invalid " + esc(str[i])+" in number");
        }
        
        if(str[i] != '.' && str[i] != 'e' && str[i] != 'E'){
            
            return Json(std::atoi(str.c_str() + pos));
        }
        
        //小数
        if(str[i] == '.'){
            i++;
            
            if(!in_range(str[i],0,9))//小数点之后至少要有一个数字
                return fail("at least one digit required in fractional part");
                
            while(in_range(str[i],'0','9')) 
                i++;
        }
        
        if(str[i] == 'e' || str[i] == 'E'){
            i++;
            
            if(str[i] == '-' || str[i] == '+')
                i++;
            
            if(!in_range(str[i],'0','9'))//科学计数法至少要有一个数字
                return fail("at least one digit required in exponent");
            
            while(in_range(str[i],'0','9')) 
                i++;
        }
        
        return Json(std::strtod(str.c_str() + pos,nullptr));
    }
    
    Json parse_bool(){
        if(str.substr(i,4) == "true"){
            i += 4;
            return Json(true);
        }else if(str.substr(i,5) == "false"){
            i += 5;
            return Json(false);
        }
        return fail("expected value,did you mean boolean type value? "+esc(str[i])); 
    }

    Json parse_null(){
        if(str.substr(i,4) == "null"){ 
            i += 4;
            return Json(nullptr);
        }
        return fail("expected value ,did you mean [null]? "+esc(str[i]));
    }

    Json parse_json(){
        
        char ch = str[next_validChar()];
        
        if(ch == '{'){
            return parse_object();
        }else if(ch == '['){
            return parse_array();
        }else if(ch == '"'){
            return parse_string();
        }else if(ch == '-' || (ch >= '0' && ch <= '9')){
            return parse_number();
        }else if(ch == 't' || ch == 'f'){
            return parse_bool();
        }else if(ch == 'n'){
            return parse_null();
        } 
        return fail("expected value,got "+esc(ch));
    }
};

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

static const Json & Static_null()
{
    static const Json json_null;
    return json_null;
}

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

/*
 * 解json，输入in，当解析错误时，返回Json()，并在err中设置相关的出错信息
 */
Json Json::Parse(const string &in,string &err)
{   
    if(in.empty()){
        err = "empty input";
        return Json();
    }
    JsonParser parser{in,err,false,0};
    return parser.parse_json();
}


//当为Array或者Object时，返回想用的Json,其他返回Json()
const Json & Json::operator[](size_t i)
{
    if(i>=get_array().size() || !is_array())
        return Static_null();
    return get_array()[i];
}
const Json & Json::operator[](const std::string &key)
{
    if(!is_object()) return Static_null();
    auto iter = get_object().find(key);
    return (iter != get_object().end())?iter->second:Static_null();
}

/********************************************class JsonValue********************************************/
bool                    JsonValue::get_bool()               const {return false;}
double                  JsonValue::get_number()             const {return 0.0;}
const string &          JsonValue::get_string()             const {return sg_init.empty_str;}
const Json::Array &     JsonValue::get_array()              const {return sg_init.empty_array;}
const Json::Object &    JsonValue::get_object()             const {return sg_init.empty_obj;}

};//end namespace vjson

