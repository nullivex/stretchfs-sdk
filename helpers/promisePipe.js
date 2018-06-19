'use strict';
var P = require('bluebird');

function promiseFromStreams(streams) {
  return P.all(streams.map(function(stream) {
    return new P(function(resolve, reject) {
      if (stream === process.stdout || stream === process.stderr) {
        return resolve();
      }
      stream.on('error', function(streamErr) {
        var err = new Error(streamErr.message);
        err.source = stream;
        err.originalError = streamErr;
        reject(err);
      });
      stream.on('end', resolve);
      stream.on('close', resolve);
      stream.on('finish', resolve);
    });
  }));
}

function promisePipe() {
  var streams = Array.prototype.slice.call(arguments);
  var promise = promiseFromStreams(streams).then(function() {
    return streams;
  });
  var streamsStack = Array.prototype.slice.call(arguments);
  var current = streamsStack.shift();
  var next;
  while ((next = streamsStack.shift())){
    current.pipe(next);
    current = next;
  }
  return promise;
}

module.exports = promisePipe;
