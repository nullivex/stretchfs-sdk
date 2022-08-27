'use strict';
const P = require('bluebird')
const bodyParser = require('body-parser')
const busboy = require('busboy')
const express = require('express')
const fs = require('graceful-fs')
const https = require('https')
const mime = require('mime')
const request = require('request')
const sha1stream = require('sha1-stream')
const temp = require('temp')

const app = express()
const content = require('./helpers/content')
const contentExists = require('./helpers/contentExists')
const job = require('./helpers/job')
const NetworkError = require('../helpers/NetworkError')
const pkg = require('../package.json')
const promisePipe = require('../helpers/promisePipe')
const purchase = require('./helpers/purchase')
const sslOptions = {
  keyFile: __dirname + '/../ssl/stretchfs_test.key',
  certFile: __dirname + '/../ssl/stretchfs_test.crt',
  pemFile: __dirname + '/../ssl/stretchfs_test.pem',
  key: fs.readFileSync(__dirname + '/../ssl/stretchfs_test.key'),
  cert: fs.readFileSync(__dirname + '/../ssl/stretchfs_test.crt'),
  pem: fs.readFileSync(__dirname + '/../ssl/stretchfs_test.pem')
}
const server = https.createServer(
  {
    cert: sslOptions.pem,
    key: sslOptions.pem
  },
  app
)
const user = require('./helpers/user')
const UserError = require('../helpers/UserError')

//make some promises
P.promisifyAll(fs)
P.promisifyAll(server)

//setup
app.use(bodyParser.json())


//--------------------
//public routes
//--------------------

//home page
app.post('/',function(req,res){
  res.json({message: 'Welcome to StretchFS Mock version ' + pkg.version})
})

//health test
app.post('/ping',function(req,res){
  res.json({pong: 'pong'})
})

//--------------------
//protected routes
//--------------------
const validateSession = function(req,res,next){
  const token = req.get('X-StretchFS-Token')
  if(!token || user.session.token !== token){
    res.status(401)
    res.json({error: 'Invalid session'})
  } else {
    req.session = user.session
    next()
  }
}

//user functions
app.post('/user/login',function(req,res){
  P.try(function(){
    if(!req.body.username || 'test' !== req.body.username)
      throw new UserError('No user found')
    if(!req.body.password || user.password !== req.body.password)
      throw new UserError('Invalid password')
    res.json({
      success: 'User logged in',
      session: user.session
    })
  })
    .catch(UserError,function(err){
      res.json({error: err.message})
    })
})

app.post('/user/logout',validateSession,function(req,res){
  res.json({success: 'User logged out'})
})
app.post('/user/session/validate',validateSession,function(req,res){
  res.json({success: 'Session Valid'})
})

//content functions
app.post('/content/detail',validateSession,function(req,res){
  const detail = contentExists
  detail.hash = req.body.hash || req.body.sha1
  detail.sha1 = req.body.hash || req.body.sha1
  res.json(detail)
})
app.post('/content/upload',validateSession,function(req,res){
  const data = {}
  const files = {}
  const filePromises = []
  const bb = busboy({
    headers: req.headers,
    highWaterMark: 65536, //64K
    limits: {
      fileSize: 2147483648000 //2TB
    }
  })
  bb.on('field',function(key,value){
    data[key] = value
  })
  bb.on('file',function(key,file,name,encoding,mimetype){
    const tmpfile = temp.path({prefix: 'stretchfs-mock-'})
    const sniff = sha1stream.createStream()
    const writeStream = fs.createWriteStream(tmpfile)
    files[key] = {
      key: key,
      tmpfile: tmpfile,
      name: name,
      encoding: encoding,
      mimetype: mimetype,
      ext: mime.getExtension(mimetype),
      hash: null
    }
    filePromises.push(
      promisePipe(file,sniff,writeStream)
        .then(function(){
          files[key].hash = sniff.hash
        })
    )
  })
  bb.on('finish',function(){
    P.all(filePromises)
      //destroy all the temp files from uploading
      .then(function(){
        const keys = Object.keys(files)
        const promises = []
        let file
        for(let i = 0; i < keys.length; i++){
          file = files[keys[i]]
          promises.push(fs.unlinkAsync(file.tmpfile))
        }
        return P.all(promises)
      })
      .then(function(){
        res.json({success: 'File(s) uploaded',data: data,files: files})
      })
      .catch(UserError,function(err){
        res.json({error: err.message})
      })
  })
  req.pipe(bb)
})
app.post('/content/retrieve',validateSession,function(req,res){
  const retrieveRequest = req.body.request
  const extension = req.body.extension || 'bin'
  const sniff = sha1stream.createStream()
  let hash
  P.try(function(){
    return promisePipe(request(retrieveRequest),sniff)
      .then(
      function(val){return val},
      function(err){throw new UserError(err.message)}
    )
  })
    .then(function(){
      hash = sniff.hash
      res.json({
        hash: hash,
        extension: extension
      })
    })
    .catch(NetworkError,function(err){
      res.status(500)
      res.json({
        error: 'Failed to check content existence: ' + err.message
      })
    })
})
app.post('/content/purchase',validateSession,function(req,res){
  const hash = req.body.hash || req.body.sha1
  const ext = req.body.ext
  const referrer = req.body.referrer
  const life = req.body.life
  if(!hash){
    res.json({error: 'No SHA1 passed for purchase'})
  }
  const detail = purchase
  detail.life = life || detail.life
  detail.referrer = referrer || detail.referrer
  detail.hash = hash
  detail.sha1 = hash
  detail.ext = ext
  res.json(detail)
})
app.post('/content/purchase/detail',validateSession,function(req,res){
  const token = req.body.token
  if(!token){
    res.json({error: 'No token passed for purchase detail'})
  }
  res.json(purchase)
})
app.post('/content/purchase/remove',validateSession,function(req,res){
  const token = req.body.token
  res.json({token: token, count: 1, success: 'Purchase removed'})
})

//job functions
app.post('/job/create',validateSession,function(req,res){
  const data = req.body
  res.json({
    handle: job.handle,
    description: data.description,
    priority: data.priority,
    category: data.category || 'resource',
    UserId: job.UserId
  })
})
app.post('/job/detail',validateSession,function(req,res){
  res.json({
    handle: job.handle,
    description: job.description,
    priority: job.priority,
    category: job.category,
    status: job.status,
    statusDescription: job.statusDescription,
    stepTotal: job.stepTotal,
    stepComplete: job.stepComplete,
    frameTotal: job.frameTotal,
    frameComplete: job.frameComplete,
    frameDescription: job.frameDescription,
    UserId: job.UserId
  })
})
app.post('/job/update',validateSession,function(req,res){
  const data = req.body
  res.json({
    handle: data.handle || job.handle,
    description: data.description || job.description,
    priority: data.priority || job.priority,
    category: data.category || job.category,
    status: data.status || job.status,
    statusDescription: data.statusDescription || job.statusDescription,
    stepTotal: data.stepTotal || job.stepTotal,
    stepComplete: data.stepComplete || job.stepComplete,
    frameTotal: data.frameTotal || job.frameTotal,
    frameComplete: data.frameComplete || job.frameComplete,
    frameDescription: data.frameDescription || job.frameDescription,
    UserId: data.UserId || job.UserId
  })
})
app.post('/job/remove',validateSession,function(req,res){
  res.json({
    success: 'Job removed',
    count: 1
  })
})
app.post('/job/start',validateSession,function(req,res){
  const jobStart = job
  jobStart.status = 'queued'
  res.json(jobStart)
})
app.post('/job/retry',validateSession,function(req,res){
  const jobRetry = job
  jobRetry.status = 'queued_retry'
  res.json(jobRetry)
})
app.post('/job/abort',validateSession,function(req,res){
  const jobAbort = job
  jobAbort.status = 'queued_abort'
  res.json(jobAbort)
})
app.post('/job/content/exists',validateSession,function(req,res){
  res.json({
    exists: false
  })
})
app.get('/job/content/download/:handle/:file',function(req,res){
  res.type('text/plain')
  res.send('foo\n')
})

//main content retrieval route
app.get('/:token/:filename',function(req,res){
  res.redirect(302,
    'http://mock.stretchfs.com/' + purchase.token + '/' + req.params.filename)
})


/**
 * Mock content record
 * @type {object}
 */
exports.content = content


/**
 * Mock content exists
 * @type {object}
 */
exports.contentExists = contentExists


/**
 * Mock job
 * @type {object}
 */
exports.job = job


/**
 * Mock SSL certificate
 * @type {object}
 */
exports.sslOptions = sslOptions


/**
 * Mock purchase
 * @type {object}
 */
exports.purchase = purchase


/**
 * Mock user and session
 * @type {object}
 */
exports.user = user


/**
 * Start stretchfs mock
 * @param {number} port
 * @param {string} host
 * @return {P}
 */
exports.start = function(port,host){
  return server.listenAsync(+port,host)
}


/**
 * Stop stretchfs prism
 * @return {P}
 */
exports.stop = function(){
  return server.closeAsync()
}
