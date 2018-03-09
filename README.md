# nodejs_mysql
nodejs mysql access module
# compile
```
clang++ -std=c++11 -I /usr/local/include/node/ -I/usr/local/include/mysql -shared -o _mysql.node mysql.cc /usr/local/lib/mysql/libmysqlclient.a
```
# usage 
```javascript
const my = require('./mysql');
var cn = my.connect();
// query the whole resultset
var r = my.query(cn,'select * from user');
console.log(r);
// query use iterator
var g = my.rset(cn,'select * from user');
g.next();
g.next();
```

