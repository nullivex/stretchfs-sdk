oose-sdk Travis Here
========

OOSE Software Development Kit

## API Usage

```
$ npm install oose-sdk --save
```

```js
'use strict';
var oose = require('oose-sdk')

oose.api.updateConfig({
  prism: {
    host: 'prism.oose.io',
    port: 5972
  }
})

//store the user session
var session = {}

//setup our api and login
var prism = oose.api.prism()
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
  .catch(oose.NetworkError,function(err){
    console.log('A network error occurred: ' + err.message)
  })
```

## Mock Usage

```js
'use strict';
var oose = require('oose-sdk')

oose.api.updateConfig({
  prism: {
    port: 3001,
    host: '127.0.0.1'
  }
})

describe('my test',function(){
  before(function(){
    return oose.mock.start(3001,'127.0.0.1')
  })
  after(function(){
    return oose.mock.stop()
  })
  it('should be up',function(){
    var prism = oose.api.prism()
    return prism.postAsync(prism.url('/ping'))
      .spread(function(res,body){
        expect(body.pong).to.equal('pong')
      })
  })
})
```

## Changelog

### 1.0.0
* Initial Release