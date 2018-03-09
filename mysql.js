const _my = require('./_mysql')

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

exports.connect = ()=>{return _my.connect();};
exports.query = (cn,query)=>{return _my.query(cn.handle,query,false);};
exports.rset = rset;
exports.test = test;

