'use strict';
var program = require('commander')

var Prism = require('./Prism')
var UserError = require('./UserError')

var pkg = require('../package.json')

program
  .version(pkg.version)
  .option('-u, --username <s>','Username for login')
  .option('-p, --password <s>','Password for login')
  .option('-H, --host <s>','Prism host, defaults to cdn.oose.io')
  .option('-P, --port <n>','Prism port, defaults to 8161')
  .parse(process.argv)

const opts = program.opts()

//setup our prism handle
var prism = new Prism({
  prism: {
    host: opts.host || null,
    port: opts.port || 8161
  }
})

//connect
prism.connect()
  .then(function(){
    return prism.login(opts.username,opts.password)
  })
  .then(function(result){
    console.log('Login successful please use the token below')
    console.log(result)
  })
  .catch(UserError,function(err){
    console.log('ERROR: ' + err.message)
  })
