#ifndef SFS_FUSE_CURL_H
#define SFS_FUSE_CURL_H

#include <string>
/* libcurl (http://curl.haxx.se/libcurl/c) */
#include <curl/curl.h>
/* json-c (https://github.com/json-c/json-c) */
#include <json-c/json.h>

#include "memory.hpp"

typedef struct {
    CURL *handle;
    struct curl_slist *headers;
    CURLcode result;
    MemoryStruct recvd;
} CURLStruct;

typedef struct {
    std::string uri;
    json_object *json;
} PostOpts;

typedef struct {
    std::string uri;
    std::string args;
} GetOpts;

bool InitCURL(void *);
bool ResetCURL(void *);
CURLcode PerformCURL(void *);
CURLcode PostCURL(void *,PostOpts);
CURLcode GetCURL(void *,GetOpts);

#endif //SFS_FUSE_CURL_H