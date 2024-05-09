stretchfs-sdk [![Build Status](https://travis-ci.org/nullivex/stretchfs-sdk.svg?branch=master)](https://travis-ci.org/nullivex/stretchfs-sdk)
========

StretchFS Software Development Kit

## Installation

```
$ npm install stretchfs-sdk
```

## Prism

The prism is the API end point of StretchFS. The Prism helper provides a
complete interface for working with a StretchFS cluster.

```js
const Prism = require('stretchfs-sdk').Prism

const sfs = new Prism({
  username: 'user',
  password: 'pass',
  domain: 'cdn.stretchfs.com',
  prism: {
   host: null,
   port: 8161
  }
})
sfs.setSession('session-token')

async function main () {
  const detail = await sfs.contentDetail('a03f181dc7dedcfb577511149b8844711efdb04f')
  console.log(detail)
}

main().then(() => { process.exit() })
```

## Turn off SSL

By default, the Prism is communicated through via SSL. The prism exposes a port
for plain text traffic. There is also the StretchFS proxy that can communicate
in plain text.

This is useful when communicating with StretchFS in a local environment.

```js
const Prism = require('stretchfs-sdk').Prism

const sfs = new Prism({
  username: 'user',
  password: 'pass',
  domain: '192.168.1.2:8080',
  prism: {
   host: '192.168.1.2',
   port: 8080,
   ssl: false
  }
})
sfs.setSession('session-token')

async function main () {
  const detail = await sfs.contentDetail('a03f181dc7dedcfb577511149b8844711efdb04f')
  console.log(detail)
}

main().then(() => { process.exit() })
```

## Direct API Usage

When using the provided Prism class is not enough.

```js
'use strict';
let stretchfs = require('stretchfs-sdk')

stretchfs.api.updateConfig({
  prism: {
    host: 'prism.stretchfs.io',
    port: 8161
  }
})

//store the user session
let session = {}

//setup our api and login
let prism = stretchfs.api.prism()
prism.postAsync({
  url: prism.url('/user/login'),
  json: {
    username: 'myusername',
    password: 'mypassword'
  }
})
  .spread(prism.validateResponse())
  .spread(function(res,body){
    console.log(body)
    session = body.session
  })
  .catch(prism.handleNetworkError)
  .catch(stretchfs.NetworkError,function(err){
    console.log('A network error occurred: ' + err.message)
  })
```

## HTTP Usage

Turn off SSL


## Mock Usage

```js
'use strict';
let stretchfs = require('stretchfs-sdk')

stretchfs.api.updateConfig({
  prism: {
    port: 3001,
    host: '127.0.0.1'
  }
})

describe('my test',function(){
  before(function(){
    return stretchfs.mock.start(3001,'127.0.0.1')
  })
  after(function(){
    return stretchfs.mock.stop()
  })
  it('should be up',function(){
    let prism = stretchfs.api.prism()
    return prism.postAsync(prism.url('/ping'))
      .spread(function(res,body){
        expect(body.pong).to.equal('pong')
      })
  })
})
```

## Changelog

### 4.1.2
* Add optional SSL support. Enables communication with an HTTP only proxy. Which
is useful for local environments and development.

### 4.1.1
* Update dependencies
* Update package json

### 4.1.0
* Add `contentPurchaseDetail` method for use in cache resolution.
* Update to latest dependencies.

### 4.0.5
* Remove NODE_TLS_REJECT_UNAUTHORIZED being set to 0
* Remove left over logging
* Update to latest dependencies

### 4.0.4
* Fix bug with blind default selection for `api.store`

### 4.0.3
* Implement custom `promisePipe` helper
* Update dependencies
* Upgrade testing

### 4.0.2
* Add file and folder management calls
* More reliably report errors

### 4.0.1
* Content purchase now takes the IP argument that will lock the purchase
to a requesting viewers IP.

### 4.0.0
* Update depdendencies to work with latest StretchFS 4.0
* Pin `promisepipe` @ 2.1.1 to preserve promise compatibility.
* Implement ES6

### 3.1.6
* Update port layout to new format, 8160 - 8169

### 3.1.5
* Rename session token, fix other leftover issues.

### 3.1.4
* Add missing keygen file, broken from rename

### 3.1.3
* urlPurchase will no longer append the purchase extension and will depend on
 the file name.
* urlStatic no longer takes the file extension just the file name with the
 extension

### 3.1.2
* Prism jobDetail no longer users api.validateResponse as jobDetail may
throw false errors just from reporting the job status

### 3.1.1
* Remove UserError being thrown by validate response
* Validate response will also now forward raw responses
* Update tp `promisepipe` 2.1.1

### 3.1.0
* Add Job API to Prism

### 3.0.1
* Does not export the Shredder object. Job object coming in 3.1
* Fix issues with `mime` 2.0 support
* Update license file to match package.json of GPL3+

### 3.0.0
* Updated to work with StretchFS 3.0.0 which is switching to Couchbase
* Implements Shredder into the StretchFS system which has been renamed to Job

### 2.2.2
* Upgrade dependencies
* Fix bug with authentication mapping

### 2.2.1
* Change mock structure to support the idea of the "hash" name for identifying
files vs the "sha1" name that predetermined the type of hash being used.

### 2.2.0
* Drop old access setup shortcuts that were related dynamic services
* BREAKING CHANGE: api.prism(), api.store() need to replaced with
api.setupAccess('prism',config.prism) and api.setupAccess('store',config.store)

### 2.1.0
* Update dependencies and bluebird implementation

### 2.0.2
* URL builder helpers no longer produce protocol forced URLs such as (http://)

### 2.0.1
* Fix issue with keygen passing configuration params
* Fix issue with Prism class wrongfully falling back on configuration params

### 2.0.0
* Update packages and dependencies
* Drop API methods removed in StretchFS 2.0.0
 * `passwordReset`

### 1.4.1

* Remove a mistakenly left console log.

### 1.4.0

* Upgraded to run with node 4.x
* Skipped version 1.3.x to match the main StretchFS release of 1.4.0
* Upgraded all dependencies
* Fixed bug that causes timeout `msecs not a number issue`

### 1.2.0
* Purchases now require the file extension and will no longer supply it this
is a breaking change to all integrations.

### 1.1.10
* Since node has started checking SSL certs without giving the user a chance
to intervene we  have to set `rejectUnauthorized: false` by default now. Have
also had to add `process.env.NODE_TLS_REJECT_UNAUTHORIZED = 0` unfortunately.

### 1.1.9
* Fix issues using latest version of node request.

### 1.1.8
* Use the domain to make prism requests to that SSL will validate properly.

### 1.1.7
* Fixed issue with keygen getting installed as the `ndt` command

### 1.1.6
* Enable sticky session support.
* Add helper for generating session keys.
* Prism login now takes username and password at call time `prism.login(un,pw)`

### 1.1.5
* Drop user session timeout, they are now sticky

### 1.1.4
* Fix and test error handling for validateResponse
* Change default maxSocket to Infinity
* Add session token name to setupSession call instead of config

### 1.1.3
* Increase default max sockets

### 1.1.2
* Expose more of the API subsystem so it can be extended properly

### 1.1.1
* Actually export Prism helper
* Added `urlPurchase` and `urlStatic` to Prism helper

### 1.1.0
* Added `Prism` helpers to access Prisms
* Added testing for new `Prism` helper
* Added `/content/retrieve`
* Added `/user/session/renew`

### 1.0.11
* Add `socket hang up` to Network error messages

### 1.0.10
* Add missing error message `ESOCKETTIMEDOUT`

### 1.0.9
* `api.handleNetworkError()` now handles all TCP/IP unix errors
* `api.handleNetworkError()` now maintains the original stack trace
* Added testing for `api.handleNetworkError()`

### 1.0.8
* Upgrade error objects with bluebird standards

### 1.0.7
* Bug fix to `api.handleNetworkError()`

### 1.0.6
* Better error catching in `api.handleNetworkError()`

### 1.0.5
* SSL uses PEM instead of crt/key

### 1.0.4
* Request wasn'st included as a dependency (was a dev dependency)

### 1.0.3
* Fix typo in ssl options

### 1.0.2
* Fixed issue with ssl options being populated by http server
* Remove preceding slash on content.relativePath

### 1.0.1
* Fix missing package for api helper

### 1.0.0
* Initial Release
