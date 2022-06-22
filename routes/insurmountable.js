var express = require('express');
var router = express.Router();

router.get('/', function(req, res, next){
  res.redirect('http://54.174.197.121:8000');
});

module.exports = router;
