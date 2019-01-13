var express = require('express');
var router = express.Router();

router.get('/', function(req, res, next){
  res.render('me');
});

router.get('/files/:doc', function(req, res, next){
  var doc = req.params.doc;
  var dir = 'public/files/'+doc;
  //res.sendfile('public/files/'+doc );
  //console.log(__dirname);
  res.sendFile(doc,{root:__dirname+'/../public/files'});
});

module.exports = router;
