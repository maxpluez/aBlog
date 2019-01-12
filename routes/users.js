var express = require('express');
var router = express.Router();

var cookieSession = require('cookie-session');

var path = require('path');

var MongoClient = require('mongodb').MongoClient;
var url = "mongodb://localhost:27017/";
var ObjectId = require('mongodb').ObjectID;

var usersdb;

MongoClient.connect(url, function(err, db) {
  if (err) throw err;
  usersdb = db.db("blogs");
});

router.use(cookieSession({keys:["fuck"], maxAge:10*60*1000}));

/* GET users listing. */
router.get('/', function(req, res, next) {
  res.redirect('./users/login');
});

router.get('/signup', function(req, res, next) {
  res.sendFile(path.resolve(__dirname+'/../public/html/signup.html'));
});

router.post('/signup', function(req, res, next) {
  var ob = req.body;
  usersdb.collection("users").insert(ob);
  res.redirect('./login');
});

router.get('/login', function(req, res, next) {
  res.sendFile(path.resolve(__dirname+'/../public/html/login.html'));
});

router.post('/login', function(req, res, next) {
  var ob = req.body;
  var oba=[];
  usersdb.collection("users").find({$and:[{username:req.body.username},{password:req.body.password}]}, function(err,doc){
    doc.forEach(function(doc,err){
      oba.push(doc);
    }, function(){
      if(oba.length==0){
        res.redirect('./login');
      }
      else{
        req.session=oba[0];
        res.redirect('../posts');
      }
    });
  });
});

/*
router.get('/show', function(req, res, next) {
  var cursor = usersdb.collection("users").find();
  var oba = [];
  cursor.forEach(function(doc,err){
    oba.push(doc);
  },function(){
    res.render('allusers',{myobjs:oba});
  })

});
*/

router.get('/logout', function(req, res, next){
  req.session=null;
  res.redirect('../posts');
});

module.exports = router;
