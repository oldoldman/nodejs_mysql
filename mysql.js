const _my = require('./_mysql')

function *rset(cn,query) {
	var res = _my.query(cn.handle,query,true)
	while((r = _my.fetch_row(res)) != null)
		yield r;
}

function query(cn,query) {
	return _my.query(cn.handle,query,false);
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
exports.rset = rset;
exports.test = test;
exports.query = query;
