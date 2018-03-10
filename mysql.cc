#include <node.h>
#include <mysql.h>
/*
clang++ -std=c++11 -I /usr/local/include/node/ -I/usr/local/include/mysql -shared -o _mysql.node mysql.cc /usr/local/lib/mysql/libmysqlclient.a
*/
using v8::Array;
using v8::Boolean;
using v8::External;
using v8::FunctionCallbackInfo;
using v8::FunctionTemplate;
using v8::Isolate;
using v8::Local;
using v8::Object;
using v8::ObjectTemplate;
using v8::PropertyAttribute;
using v8::String;
using v8::Null;
using v8::Value;
using v8::Uint32;
using v8::PropertyCallbackInfo;

#define MYSQL_SERVER_STATUS 1
#define MYSQL_CLIENT_FLAG 2
#define MYSQL_STATUS 3
#define MYSQL_WARNING_COUNT 4
#define MYSQL_AFFECTED_ROWS 5

void get_MYSQL_field(Local<String> property,
  const PropertyCallbackInfo<Value>& info) {
  Isolate *isolate = info.GetIsolate();
  Local<Value> handle = info.This()->Get(String::NewFromUtf8(isolate,"handle"));
  Local<External> external = Local<External>::Cast(handle);
  Uint32 *idx = Uint32::Cast(*(info.Data()));
  MYSQL *cn = (MYSQL*)external->Value();
  
  switch (idx->Value())
  {
    case MYSQL_SERVER_STATUS:
      info.GetReturnValue().Set((int32_t)cn->server_status);
      break;
    case MYSQL_CLIENT_FLAG:
      info.GetReturnValue().Set((int32_t)cn->client_flag);
      break;
    case MYSQL_STATUS:
      info.GetReturnValue().Set((int32_t)cn->status);
      break;
    case MYSQL_WARNING_COUNT:
      info.GetReturnValue().Set((int32_t)cn->warning_count);
      break;
    // case MYSQL_AFFECTED_ROWS:
    default:
      info.GetReturnValue().Set((int32_t)cn->affected_rows);
  }
}

void my_connect(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  MYSQL *con = mysql_init(NULL);
  Local<ObjectTemplate> objt = ObjectTemplate::New(isolate);
  con = mysql_real_connect(con,"localhost","root","shang1974","mysql",0,"/tmp/mysql.sock",0);

  Local<String> handle = String::NewFromUtf8(isolate,"handle");
  Local<String> host = String::NewFromUtf8(isolate,"host");
  Local<String> user = String::NewFromUtf8(isolate,"user");
  Local<String> passwd = String::NewFromUtf8(isolate,"passwd");
  Local<String> unix_socket = String::NewFromUtf8(isolate,"unix_socket");
  Local<String> server_version = String::NewFromUtf8(isolate,"server_version");
  Local<String> host_info = String::NewFromUtf8(isolate,"host_info");
  Local<String> info = String::NewFromUtf8(isolate,"info");
  Local<String> db = String::NewFromUtf8(isolate,"db");
  Local<String> client_flag = String::NewFromUtf8(isolate,"client_flag");
  Local<String> server_status = String::NewFromUtf8(isolate,"server_status");
  Local<String> status = String::NewFromUtf8(isolate,"status");
  Local<String> warning_count = String::NewFromUtf8(isolate,"warning_count");
  Local<String> affected_rows = String::NewFromUtf8(isolate,"affected_rows");

  PropertyAttribute dont_delete_enum = static_cast<PropertyAttribute>(v8::DontDelete|v8::DontEnum);
  PropertyAttribute dont_delete = static_cast<PropertyAttribute>(v8::DontDelete);
	
  objt->Set(handle,ObjectTemplate::New(isolate),dont_delete_enum);
  objt->Set(host,ObjectTemplate::New(isolate),dont_delete);  
  objt->Set(user,ObjectTemplate::New(isolate),dont_delete);
  objt->Set(passwd,ObjectTemplate::New(isolate),dont_delete_enum);
  objt->Set(unix_socket,ObjectTemplate::New(isolate),dont_delete);
  objt->Set(server_version,ObjectTemplate::New(isolate),dont_delete);
  objt->Set(host_info,ObjectTemplate::New(isolate),dont_delete);
  objt->Set(info,ObjectTemplate::New(isolate),dont_delete);
  objt->Set(db,ObjectTemplate::New(isolate),dont_delete);
  objt->SetAccessor(client_flag,get_MYSQL_field,0,Uint32::New(isolate,MYSQL_CLIENT_FLAG));
  objt->SetAccessor(server_status,get_MYSQL_field,0,Uint32::New(isolate,MYSQL_SERVER_STATUS));
  objt->SetAccessor(status,get_MYSQL_field,0,Uint32::New(isolate,MYSQL_STATUS));
  objt->SetAccessor(warning_count,get_MYSQL_field,0,Uint32::New(isolate,MYSQL_WARNING_COUNT));
  objt->SetAccessor(affected_rows,get_MYSQL_field,0,Uint32::New(isolate,MYSQL_AFFECTED_ROWS));  

  Local<Object> obj = objt->NewInstance();
  obj->Set(handle,External::New(isolate,con));
  obj->Set(host,String::NewFromUtf8(isolate,con->host));
  obj->Set(user,String::NewFromUtf8(isolate,con->user));
  obj->Set(passwd,String::NewFromUtf8(isolate,con->passwd));
  obj->Set(unix_socket,String::NewFromUtf8(isolate,con->unix_socket));
  obj->Set(server_version,String::NewFromUtf8(isolate,con->server_version));
  obj->Set(host_info,String::NewFromUtf8(isolate,con->host_info));
  obj->Set(info,String::NewFromUtf8(isolate,con->info == nullptr?"":con->info));
  obj->Set(db,String::NewFromUtf8(isolate,con->db == nullptr?"":con->db));
  
  args.GetReturnValue().Set(obj);
}

void my_query(const FunctionCallbackInfo<Value>& args) {
  Isolate *isolate = args.GetIsolate();
  External *con = External::Cast(*args[0]);
  String *query = String::Cast(*args[1]);
  Boolean *gen = Boolean::Cast(*args[2]);

  char buf[500];
  query->WriteUtf8(buf);

  MYSQL *cn = (MYSQL*)con->Value();
  Local<Array> results = Array::New(isolate);
  PropertyAttribute dont_delete = static_cast<PropertyAttribute>(v8::DontDelete);
  
  if(!mysql_query(cn,buf))
  {
    Local<ObjectTemplate> objt = ObjectTemplate::New(isolate);
    MYSQL_RES *res = mysql_use_result(cn);

    if(gen->Value())
    {
      args.GetReturnValue().Set(External::New(isolate,res));
      return;
    }

    for(int i = 0 ; i < res->field_count; i++)
    {
      Local<String> field = String::NewFromUtf8(isolate,res->fields[i].name);
      objt->Set(field,ObjectTemplate::New(isolate),dont_delete);
    }  

    MYSQL_ROW row;
    int idx = 0;

    while((row = mysql_fetch_row(res)))
    {
      Local<Object> obj = objt->NewInstance();
      for(int i = 0 ; i < res->field_count; i++)
      {
        Local<String> field = String::NewFromUtf8(isolate,res->fields[i].name);
        Local<String> val = String::NewFromUtf8(isolate,row[i] == nullptr ? "":row[i]);
        obj->Set(field,val);
        results->Set(idx,obj);
      }
      idx++;
    }
    args.GetReturnValue().Set(results);
    return;
  }
  args.GetReturnValue().Set(Null(isolate));
}

void my_fetch_row(const FunctionCallbackInfo<Value>& args) {
  Isolate *isolate = args.GetIsolate();
  External *rs = External::Cast(*args[0]);
  MYSQL_RES *res = (MYSQL_RES*)rs->Value();	
  PropertyAttribute dont_delete = static_cast<PropertyAttribute>(v8::DontDelete);
  Local<ObjectTemplate> objt = ObjectTemplate::New(isolate);

  for(int i = 0 ; i < res->field_count; i++)
  {
    Local<String> field = String::NewFromUtf8(isolate,res->fields[i].name);
    objt->Set(field,ObjectTemplate::New(isolate),dont_delete);
  } 

  Local<Object> obj = objt->NewInstance();
  MYSQL_ROW row = mysql_fetch_row(res);
  if(row)
  {
    for(int i = 0 ; i < res->field_count; i++)
    {
      Local<String> field = String::NewFromUtf8(isolate,res->fields[i].name);
      Local<String> val = String::NewFromUtf8(isolate,row[i] == nullptr ? "":row[i]);
      obj->Set(field,val);
    }
    args.GetReturnValue().Set(obj);
    return;
  }
  args.GetReturnValue().Set(Null(isolate));
}

void my_client_info(const FunctionCallbackInfo<Value>& args) {
  Isolate *isolate = args.GetIsolate();
  args.GetReturnValue().Set(String::NewFromUtf8(isolate,mysql_get_client_info()));
}

void v8_version(const FunctionCallbackInfo<Value>& args) {
  Isolate *isolate = args.GetIsolate();
  args.GetReturnValue().Set(String::NewFromUtf8(isolate,v8::V8::GetVersion()));
  Local<String> t;
}

void init(Local<Object> exports) {
  NODE_SET_METHOD(exports, "connect", my_connect);
  NODE_SET_METHOD(exports, "fetch_row", my_fetch_row);
  NODE_SET_METHOD(exports, "client_info", my_client_info);  
  NODE_SET_METHOD(exports, "query", my_query);
  NODE_SET_METHOD(exports, "v8_version", v8_version);
}

NODE_MODULE(NODE_GYP_MODULE_NAME, init)

