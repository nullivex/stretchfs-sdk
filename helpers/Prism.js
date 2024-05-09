'use strict';
var P = require('bluebird')
var dns = require('dns')
var fs = require('graceful-fs')
var ObjectManage = require('object-manage')
var path = require('path')

var api = require('./api')
var UserError = require('./UserError')

//make some promises
P.promisifyAll(dns)



/**
 * Prism Public Interaction Helper
 * @param {object} opts
 * @constructor
 */
var Prism = function(opts){
  //setup options
  this.opts = new ObjectManage({
    username: '',
    password: '',
    domain: 'cdn.stretchfs.com',
    prism: {
      host: null,
      port: 8161,
      ssl: true
    }
  })
  this.opts.$load(opts)
  //set properties
  this.api = {}
  this.authenticated = false
  this.connected = false
  this.session = {}
}


/**
 * Check if we are connected
 * @return {boolean}
 */
Prism.prototype.isConnected = function(){
  return this.connected
}


/**
 * Check if we are authenticated
 * @return {boolean}
 */
Prism.prototype.isAuthenticated = function(){
  return this.authenticated
}


/**
 * Set the session statically
 * @param {string} sessionToken
 * @return {Prism}
 */
Prism.prototype.setSession = function(sessionToken){
  this.session = {token: sessionToken}
  this.authenticated = true
  return this
}


/**
 * Select a prism and prepare for connection
 * @param {string} host
 * @param {number} port
 * @return {P}
 */
Prism.prototype.connect = function(host,port){
  var that = this
  return P.try(function(){
    if(host){
      that.opts.prism.host = host
      if(port) that.opts.prism.port = port
      that.connected = true
      that.api = api.setupAccess('prism',that.opts.prism)
    } else {
      if(!that.opts.prism.host) that.opts.prism.host = that.opts.domain
      if(!that.opts.prism.port) that.opts.prism.port = port || 8161
      that.connected = true
      that.api = api.prism(that.opts.prism)
    }
    return host || that.opts.prism.host
  })
}


/**
 * Authenticate the session
 * @param {string} username
 * @param {string} password
 * @return {P}
 */
Prism.prototype.login = function(username,password){
  var that = this
  return that.api.postAsync({
    url: that.api.url('/user/login'),
    json: {
      username: username || that.opts.username,
      password: password || that.opts.password
    }
  })
    .spread(that.api.validateResponse())
    .spread(function(res,body){
      if(!body.session)
        throw new UserError('Login failed, no session')
      that.session = body.session
      that.authenticated = true
      return that.session
    })
    .catch(that.api.handleNetworkError)
}


/**
 * Prepare call and renew session if needed
 * @param {object} request
 * @param {object} session
 * @return {P}
 */
Prism.prototype.prepare = function(request,session){
  var that = this
  if(!request) request = that.api
  if(!session) session = that.session
  var client = api.setSession(session,request,'X-STRETCHFS-Token')
  if(!that.isConnected()) throw new UserError('Not connected')
  if(!that.isAuthenticated()) throw new UserError('Not authenticated')
  return P.try(function(){
    return client
  })
}


/**
 * Logout
 * @return {P}
 */
Prism.prototype.logout = function(){
  var that = this
  var client = {}
  return that.prepare()
    .then(function(result){
      client = result
      return client.postAsync({
        url: client.url('/user/logout'),
        json: true
      })
    })
    .spread(that.api.validateResponse())
    .spread(function(res,body){
      that.authenticated = false
      return body
    })
    .catch(that.handleNetworkError)
}


/**
 * Content detail
 * @param {string} hash
 * @return {P}
 */
Prism.prototype.contentDetail = function(hash){
  var that = this
  var client = {}
  return that.prepare()
    .then(function(result){
      client = result
      return client.postAsync({
        url: client.url('/content/detail'),
        json: {
          hash: hash
        }
      })
    })
    .spread(that.api.validateResponse())
    .spread(function(res,body){
      return body
    })
    .catch(that.handleNetworkError)
}


/**
 * Upload a file from a path
 * @param {string} filepath
 * @return {P}
 */
Prism.prototype.contentUpload = function(filepath){
  var that = this
  var client = {}
  return that.prepare()
    .then(function(result){
      client = result
      return client.postAsync({
        url: client.url('/content/upload'),
        json: true,
        formData: {
          file: fs.createReadStream(path.resolve(filepath))
        }
      })
    })
    .spread(that.api.validateResponse())
    .spread(function(res,body){
      return body
    })
    .catch(that.handleNetworkError)
}


/**
 * Retrieve content from a URL
 * @param {object} request
 * @param {string} fileExtension
 * @return {P}
 */
Prism.prototype.contentRetrieve = function(request,fileExtension){
  var that = this
  var client = {}
  if(!fileExtension) fileExtension = path.extname(request.url).replace('.','')
  return that.prepare()
    .then(function(result){
      client = result
      return client.postAsync({
        url: client.url('/content/retrieve'),
        json: {
          request: request,
          extension: fileExtension
        }
      })
    })
    .spread(that.api.validateResponse())
    .spread(function(res,body){
      return body
    })
    .catch(that.handleNetworkError)
}


/**
 * Purchase Content
 * @param {string} hash
 * @param {string} ext
 * @param {array} referrer
 * @param {number} life
 * @param {string} ip
 * @return {P}
 */
Prism.prototype.contentPurchase = function(hash,ext,referrer,life,ip){
  var that = this
  var client = {}
  return that.prepare()
    .then(function(result){
      client = result
      return client.postAsync({
        url: client.url('/content/purchase'),
        json: {
          hash: hash,
          ext: ext,
          referrer: referrer,
          life: life,
          ip: ip
        }
      })
    })
    .spread(that.api.validateResponse())
    .spread(function(res,body){
      return body
    })
    .catch(that.handleNetworkError)
}


/**
 * Purchase Detail
 * @param {string} token
 * @return {P}
 */
Prism.prototype.contentPurchaseDetail = function(token){
  var that = this
  var client = {}
  return that.prepare()
    .then(function(result){
      client = result
      return client.postAsync({
        url: client.url('/content/purchase/detail'),
        json: {
          token: token
        }
      })
    })
    .spread(that.api.validateResponse())
    .spread(function(res,body){
      return body
    })
    .catch(that.handleNetworkError)
}


/**
 * Remove purchase
 * @param {string} token
 * @return {P}
 */
Prism.prototype.contentPurchaseRemove = function(token){
  var that = this
  var client = {}
  return that.prepare()
    .then(function(result){
      client = result
      return client.postAsync({
        url: client.url('/content/purchase/remove'),
        json: {
          token: token
        }
      })
    })
    .spread(that.api.validateResponse())
    .spread(function(res,body){
      return body
    })
    .catch(that.handleNetworkError)
}


/**
 * Output a purchase URL
 * @param {object} purchase
 * @param {string} name
 * @return {string}
 */
Prism.prototype.urlPurchase = function(purchase,name){
  var that = this
  name = name || 'video'
  return '//' + that.opts.domain + '/' +
    purchase.token + '/' + name
}


/**
 * Output a static URL
 * @param {string} hash
 * @param {string} name
 * @return {string}
 */
Prism.prototype.urlStatic = function(hash,name){
  var that = this
  name = name || 'file'
  return '//' + that.opts.domain + '/static/' +
    hash + '/' + name
}


/**
 * Create Job
 * @param {object} description
 * @param {integer} priority
 * @param {string} category - either augment or resource
 * @return {P}
 */
Prism.prototype.jobCreate = function(description,priority,category){
  var that = this
  var client = {}
  return that.prepare()
    .then(function(result){
      client = result
      return client.postAsync({
        url: client.url('/job/create'),
        json: {
          description: JSON.stringify(description),
          priority: priority || null,
          category: category || 'resource'
        }
      })
    })
    .spread(that.api.validateResponse())
    .spread(function(res,body){
      return body
    })
    .catch(that.handleNetworkError)
}


/**
 * Job detail
 * @param {string} handle
 * @return {P}
 */
Prism.prototype.jobDetail = function(handle){
  var that = this
  var client = {}
  return that.prepare()
    .then(function(result){
      client = result
      return client.postAsync({
        url: client.url('/job/detail'),
        json: {
          handle: handle
        }
      })
    })
    .spread(function(res,body){
      return body
    })
    .catch(that.handleNetworkError)
}


/**
 * Job update
 * @param {string} handle
 * @param {object} changes
 * @return {P}
 */
Prism.prototype.jobUpdate = function(handle,changes){
  var that = this
  var client = {}
  return that.prepare()
    .then(function(result){
      client = result
      return client.postAsync({
        url: client.url('/job/update'),
        json: changes
      })
    })
    .spread(that.api.validateResponse())
    .spread(function(res,body){
      return body
    })
    .catch(that.handleNetworkError)
}


/**
 * Job start
 * @param {string} handle
 * @return {P}
 */
Prism.prototype.jobStart = function(handle){
  var that = this
  var client = {}
  return that.prepare()
    .then(function(result){
      client = result
      return client.postAsync({
        url: client.url('/job/start'),
        json: {
          handle: handle
        }
      })
    })
    .spread(that.api.validateResponse())
    .spread(function(res,body){
      return body
    })
    .catch(that.handleNetworkError)
}


/**
 * Abort a job
 * @param {string} handle
 * @return {P}
 */
Prism.prototype.jobAbort = function(handle){
  var that = this
  var client = {}
  return that.prepare()
    .then(function(result){
      client = result
      return client.postAsync({
        url: client.url('/job/abort'),
        json: {
          handle: handle
        }
      })
    })
    .spread(that.api.validateResponse())
    .spread(function(res,body){
      return body
    })
    .catch(that.handleNetworkError)
}


/**
 * Job retry
 * @param {string} handle
 * @return {P}
 */
Prism.prototype.jobRetry = function(handle){
  var that = this
  var client = {}
  return that.prepare()
    .then(function(result){
      client = result
      return client.postAsync({
        url: client.url('/job/retry'),
        json: {
          handle: handle
        }
      })
    })
    .spread(that.api.validateResponse())
    .spread(function(res,body){
      return body
    })
    .catch(that.handleNetworkError)
}


/**
 * Job remove
 * @param {string} handle
 * @return {P}
 */
Prism.prototype.jobRemove = function(handle){
  var that = this
  var client = {}
  return that.prepare()
    .then(function(result){
      client = result
      return client.postAsync({
        url: client.url('/job/remove'),
        json: {
          handle: handle
        }
      })
    })
    .spread(that.api.validateResponse())
    .spread(function(res,body){
      return body
    })
    .catch(that.handleNetworkError)
}


/**
 * Job content exists
 * @param {string} handle
 * @param {string} file  - relative path to content
 * @return {P}
 */
Prism.prototype.jobContentExists = function(handle,file){
  var that = this
  var client = {}
  return that.prepare()
    .then(function(result){
      client = result
      return client.postAsync({
        url: client.url('/job/content/exists'),
        json: {
          handle: handle,
          file: file
        }
      })
    })
    .spread(that.api.validateResponse())
    .spread(function(res,body){
      return !!body.exists
    })
    .catch(that.handleNetworkError)
}


/**
 * Job content URL
 * @param {string} handle
 * @param {string} file
 * @return {string}
 */
Prism.prototype.jobContentUrl = function(handle,file){
  var that = this
  return 'https://' + that.opts.prism.host + ':' + that.opts.prism.port +
    '/job/content/download/' + handle + '/' + file
}


/**
 * Folder create
 * @param {string} folderPath
 * @return {P}
 */
Prism.prototype.folderCreate = function(folderPath){
  var that = this
  var client = {}
  return that.prepare()
    .then(function(result){
      client = result
      return client.postAsync({
        url: client.url('/file/folderCreate'),
        json: {
          path: folderPath
        }
      })
    })
    .spread(that.api.validateResponse())
    .spread(function(res,body){
      return body
    })
    .catch(that.handleNetworkError)
}


/**
 * Folder delete
 * @param {string} folderPath
 * @return {P}
 */
Prism.prototype.folderDelete = function(folderPath){
  var that = this
  var client = {}
  return that.prepare()
    .then(function(result){
      client = result
      return client.postAsync({
        url: client.url('/file/remove'),
        json: {
          path: folderPath
        }
      })
    })
    .spread(that.api.validateResponse())
    .spread(function(res,body){
      return body
    })
    .catch(that.handleNetworkError)
}


/**
 * File import
 * @param {string} folderPath
 * @param {array} fileList
 * @return {P}
 */
Prism.prototype.fileImport = function(folderPath,fileList){
  var that = this
  var client = {}
  return that.prepare()
    .then(function(result){
      client = result
      return client.postAsync({
        url: client.url('/file/import'),
        json: {
          folderPath: folderPath,
          urlText: (fileList || []).join('\n')
        }
      })
    })
    .spread(that.api.validateResponse())
    .spread(function(res,body){
      return body
    })
    .catch(that.handleNetworkError)
}


/**
 * File import list
 * @return {P}
 */
Prism.prototype.fileImportList = function(){
  var that = this
  var client = {}
  return that.prepare()
    .then(function(result){
      client = result
      return client.getAsync({
        url: client.url('/file/importList')
      })
    })
    .spread(that.api.validateResponse())
    .spread(function(res,body){
      return body
    })
    .catch(that.handleNetworkError)
}


/**
 * File list
 * @param {string} filePath
 * @return {P}
 */
Prism.prototype.fileList = function(filePath){
  var that = this
  var client = {}
  return that.prepare()
    .then(function(result){
      client = result
      return client.getAsync({
        url: client.url('/file/list'),
        qs: {
          path: filePath
        }
      })
    })
    .spread(that.api.validateResponse())
    .spread(function(res,body){
      return body
    })
    .catch(that.handleNetworkError)
}


/**
 * File detail
 * @param {string} filePath
 * @return {P}
 */
Prism.prototype.fileDetail = function(filePath){
  var that = this
  var client = {}
  return that.prepare()
    .then(function(result){
      client = result
      return client.getAsync({
        url: client.url('/file/detail'),
        qs: {
          path: filePath
        }
      })
    })
    .spread(that.api.validateResponse())
    .spread(function(res,body){
      return body
    })
    .catch(that.handleNetworkError)
}


/**
 * File download
 * @param {string} fileHandle
 * @return {P}
 */
Prism.prototype.fileDownload = function(fileHandle){
  var that = this
  var client = {}
  return that.prepare()
    .then(function(result){
      client = result
      return client.getAsync({
        url: client.url('/file/download'),
        qs: {
          handle: fileHandle
        }
      })
    })
    .spread(that.api.validateResponse())
    .spread(function(res,body){
      return body
    })
    .catch(that.handleNetworkError)
}


/**
 * File embed
 * @param {string} fileHandle
 * @return {P}
 */
Prism.prototype.fileEmbed = function(fileHandle){
  var that = this
  var client = {}
  return that.prepare()
    .then(function(result){
      client = result
      return client.getAsync({
        url: client.url('/file/embed'),
        qs: {
          handle: fileHandle
        }
      })
    })
    .spread(that.api.validateResponse())
    .spread(function(res,body){
      return body
    })
    .catch(that.handleNetworkError)
}


/**
 * Upload a file from a path
 * @param {string} localFile
 * @return {P}
 */
Prism.prototype.fileUpload = function(localFile){
  var that = this
  var client = {}
  return that.prepare()
    .then(function(result){
      client = result
      return client.postAsync({
        url: client.url('/file/upload'),
        json: true,
        formData: {
          file: fs.createReadStream(path.resolve(localFile))
        }
      })
    })
    .spread(that.api.validateResponse())
    .spread(function(res,body){
      return body
    })
    .catch(that.handleNetworkError)
}


/**
 * Move a File or Folder
 * @param {array} folderList
 * @param {array} fileList
 * @param {string} destinationPath
 * @return {P}
 */
Prism.prototype.fileMove = function(folderList,fileList,destinationPath){
  var that = this
  var client = {}
  return that.prepare()
    .then(function(result){
      client = result
      return client.postAsync({
        url: client.url('/file/move'),
        json: {
          folderList: folderList || [],
          fileList: fileList || [],
          destinationPath: destinationPath
        }
      })
    })
    .spread(that.api.validateResponse())
    .spread(function(res,body){
      return body
    })
    .catch(that.handleNetworkError)
}


/**
 * Delete a File
 * @param {string} filePath
 * @return {P}
 */
Prism.prototype.fileDelete = function(filePath){
  var that = this
  var client = {}
  return that.prepare()
    .then(function(result){
      client = result
      return client.postAsync({
        url: client.url('/file/remove'),
        json: {
          path: filePath
        }
      })
    })
    .spread(that.api.validateResponse())
    .spread(function(res,body){
      return body
    })
    .catch(that.handleNetworkError)
}


/**
 * Export Prism
 * @type {Prism}
 */
module.exports = Prism
