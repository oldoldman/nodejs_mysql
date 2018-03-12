#include <node.h>
#include <mysql.h>
/*
clang++ -std=c++11 -Wno-unused-result -I/usr/local/include/node/ -I/usr/local/include/mysql -shared -o _mysql.node mysql.cc /usr/local/lib/mysql/libmysqlclient.a
*/

using namespace v8;

#define MYSQL_SERVER_STATUS 1
#define MYSQL_CLIENT_FLAG   2
#define MYSQL_STATUS        3
#define MYSQL_WARNING_COUNT 4
#define MYSQL_AFFECTED_ROWS 5
#define STR(string) String::NewFromUtf8(isolate,string)

PropertyAttribute dont_delete_enum = static_cast<PropertyAttribute>(v8::DontDelete|v8::DontEnum);
PropertyAttribute dont_delete = static_cast<PropertyAttribute>(v8::DontDelete);

void get_MYSQL_field(Local<Name> property,
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
  Local<Context> context = isolate->GetCurrentContext();
  MYSQL *con = mysql_init(NULL);
  MY_CHARSET_INFO csi;
  
  con = mysql_real_connect(con,"localhost","root","shang1974","mysql",0,"/tmp/mysql.sock",0);
  mysql_get_character_set_info(con,&csi);
  
	Local<ObjectTemplate> objt = ObjectTemplate::New(isolate);
	Local<Object> obj = objt->NewInstance();  

  obj->DefineOwnProperty(context,STR("handle"),External::New(isolate,con),dont_delete_enum);
  obj->DefineOwnProperty(context,STR("host"),STR(con->host),dont_delete);
  obj->DefineOwnProperty(context,STR("user"),STR(con->user),dont_delete);
  obj->DefineOwnProperty(context,STR("passwd"),STR(con->passwd),dont_delete_enum);
  obj->DefineOwnProperty(context,STR("unix_socket"),STR(con->unix_socket),dont_delete);
  obj->DefineOwnProperty(context,STR("server_version"),STR(con->server_version),dont_delete);
  obj->DefineOwnProperty(context,STR("host_info"),STR(con->host_info),dont_delete);
  if(con->info == nullptr) obj->DefineOwnProperty(context,STR("info"),Null(isolate),dont_delete);
  else obj->DefineOwnProperty(context,STR("info"),STR(con->info),dont_delete);
  if(con->db == nullptr) obj->DefineOwnProperty(context,STR("db"),Null(isolate),dont_delete);
  else obj->DefineOwnProperty(context,STR("db"),STR(con->db),dont_delete);
  obj->SetAccessor(context,STR("server_status"),get_MYSQL_field,nullptr,Uint32::New(isolate,MYSQL_SERVER_STATUS),DEFAULT,dont_delete);
  obj->SetAccessor(context,STR("client_flag"),get_MYSQL_field,nullptr,Uint32::New(isolate,MYSQL_CLIENT_FLAG),DEFAULT,dont_delete);
  obj->SetAccessor(context,STR("status"),get_MYSQL_field,nullptr,Uint32::New(isolate,MYSQL_STATUS),DEFAULT,dont_delete);
  obj->SetAccessor(context,STR("warning_count"),get_MYSQL_field,nullptr,Uint32::New(isolate,MYSQL_WARNING_COUNT),DEFAULT,dont_delete);
  obj->SetAccessor(context,STR("affected_rows"),get_MYSQL_field,nullptr,Uint32::New(isolate,MYSQL_AFFECTED_ROWS),DEFAULT,dont_delete);

  Local<Object> csobj = objt->NewInstance();
  csobj->DefineOwnProperty(context,STR("number"),Uint32::New(isolate,csi.number),dont_delete);
  csobj->DefineOwnProperty(context,STR("state"),Uint32::New(isolate,csi.state),dont_delete);
  if (csi.csname == nullptr) csobj->DefineOwnProperty(context,STR("csname"),Null(isolate),dont_delete);
  else csobj->DefineOwnProperty(context,STR("csname"),STR(csi.csname),dont_delete);
  if (csi.name == nullptr) csobj->DefineOwnProperty(context,STR("name"),Null(isolate),dont_delete);
  else csobj->DefineOwnProperty(context,STR("name"),STR(csi.name),dont_delete);
  if (csi.comment == nullptr) csobj->DefineOwnProperty(context,STR("comment"),Null(isolate),dont_delete);
  else csobj->DefineOwnProperty(context,STR("comment"),STR(csi.comment),dont_delete);
  if (csi.dir == nullptr) csobj->DefineOwnProperty(context,STR("dir"),Null(isolate),dont_delete);
  else csobj->DefineOwnProperty(context,STR("dir"),STR(csi.dir),dont_delete);
  csobj->DefineOwnProperty(context,STR("mbminlen"),Uint32::New(isolate,csi.mbminlen),dont_delete);
  csobj->DefineOwnProperty(context,STR("mbmaxlen"),Uint32::New(isolate,csi.mbmaxlen),dont_delete);
  
  obj->DefineOwnProperty(context,STR("cs"),csobj,dont_delete);  
  args.GetReturnValue().Set(obj);
}

void my_query(const FunctionCallbackInfo<Value>& args) {
  Isolate *isolate = args.GetIsolate();
  Local<Context> context = isolate->GetCurrentContext();
  External *con = External::Cast(*args[0]);
  String *query = String::Cast(*args[1]);
  Boolean *gen = Boolean::Cast(*args[2]);

  char buf[500];
  query->WriteUtf8(buf);

  MYSQL *cn = (MYSQL*)con->Value();
  Local<Array> results = Array::New(isolate);
  
  
  if(!mysql_query(cn,buf))
  {
    Local<ObjectTemplate> objt = ObjectTemplate::New(isolate);
    MYSQL_RES *res = mysql_use_result(cn);

    if(gen->Value())
    {
      args.GetReturnValue().Set(External::New(isolate,res));
      return;
    }

    MYSQL_ROW row;
    int idx = 0;

    while((row = mysql_fetch_row(res)))
    {
      Local<Object> obj = objt->NewInstance();
      for(int i = 0 ; i < res->field_count; i++)
      {
        Local<Value> val;
        if (row[i] == nullptr) val = Null(isolate);
        else val = STR(row[i]);
        obj->DefineOwnProperty(context,STR(res->fields[i].name),val,dont_delete);
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
  Local<Context> context = isolate->GetCurrentContext();
  External *rs = External::Cast(*args[0]);
  MYSQL_RES *res = (MYSQL_RES*)rs->Value();	
  PropertyAttribute dont_delete = static_cast<PropertyAttribute>(v8::DontDelete);

  Local<ObjectTemplate> objt = ObjectTemplate::New(isolate);
  Local<Object> obj = objt->NewInstance();
  MYSQL_ROW row = mysql_fetch_row(res);
  if(row)
  {
    for(int i = 0 ; i < res->field_count; i++)
    {
      Local<Value> val;
      if (row[i] == nullptr) val = Null(isolate);
      else val = String::NewFromUtf8(isolate, row[i]);
      obj->DefineOwnProperty(context,STR(res->fields[i].name),val,dont_delete);
    }
    args.GetReturnValue().Set(obj);
    return;
  }
  args.GetReturnValue().Set(Null(isolate));
}

void my_client_info(const FunctionCallbackInfo<Value>& args) {
  Isolate *isolate = args.GetIsolate();
  args.GetReturnValue().Set(STR(mysql_get_client_info()));
}

void v8_version(const FunctionCallbackInfo<Value>& args) {
  Isolate *isolate = args.GetIsolate();
  args.GetReturnValue().Set(STR(v8::V8::GetVersion()));
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

