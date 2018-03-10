const _my = require('./_mysql')

const SERVER_STATUS_IN_TRANS = 1;
const SERVER_STATUS_AUTOCOMMIT = 2;
const SERVER_MORE_RESULTS_EXISTS = 8;
const SERVER_QUERY_NO_GOOD_INDEX_USED = 16;
const SERVER_QUERY_NO_INDEX_USED = 32;
const SERVER_STATUS_CURSOR_EXISTS = 64;
const SERVER_STATUS_LAST_ROW_SENT = 128;
const SERVER_STATUS_DB_DROPPED = 256;
const SERVER_STATUS_NO_BACKSLASH_ESCAPES = 512;
const SERVER_STATUS_METADATA_CHANGED = 1024;
const SERVER_QUERY_WAS_SLOW = 2048;
const SERVER_PS_OUT_PARAMS = 4096;
const SERVER_STATUS_IN_TRANS_READONLY = 8192;
const SERVER_SESSION_STATE_CHANGED = (1<<14);

const CLIENT_LONG_PASSWORD = 1;
const CLIENT_FOUND_ROWS = 2;
const CLIENT_LONG_FLAG = 4;
const CLIENT_CONNECT_WITH_DB = 8;
const CLIENT_NO_SCHEMA = 16;
const CLIENT_COMPRESS = 32;
const CLIENT_ODBC = 64;
const CLIENT_LOCAL_FILES = 128;
const CLIENT_IGNORE_SPACE = 256;
const CLIENT_PROTOCOL_41 = 512;
const CLIENT_SSL = 1024;
const CLIENT_IGNORE_SIGPIPE = 4096;
const CLIENT_TRANSACTIONS = 8192;
const CLIENT_RESERVED = 16384;
const CLIENT_RESERVED2 = 32768;
const CLIENT_MULTI_STATEMENTS = (1<<16);
const CLIENT_MULTI_RESULTS = (1<<17);
const CLIENT_PS_MULTI_RESULTS = (1<<18);
const CLIENT_PLUGIN_AUTH = (1<<19);
const CLIENT_CONNECT_ATTRS = (1<<20);
const CLIENT_PLUGIN_AUTH_LENENC_CLIENT_DATA = (1<<21);
const CLIENT_CAN_HANDLE_EXPIRED_PASSWORDS = (1<<22);
const CLIENT_SESSION_TRACK = (1<<23);
const CLIENT_DEPRECATE_EOF = (1<<24);
const CLIENT_SSL_VERIFY_SERVER_CERT = (1<<30);
const CLIENT_REMEMBER_OPTIONS = (1<<31);

function connect() {
  var cn = _my.connect();
  return cn;
}

function *rset(cn,query) {
  var res = _my.query(cn.handle,query,true);
  if (res == null ) return;
  while((r = _my.fetch_row(res)) != null)
    yield r;
}

function test() {
  var g = rset(_my.connect(),'select * from user');
  var r = g.next();
  do {
    console.log(r.value);
    r = g.next();	
  }while(!r.done)
}

exports.connect = connect;
exports.query = (cn,query)=>{return _my.query(cn.handle,query,false);};
exports.rset = rset;
exports.test = test;

