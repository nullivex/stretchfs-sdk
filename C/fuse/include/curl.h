#ifndef SFS_FUSE_CURL_H
#define SFS_FUSE_CURL_H

/* libcurl (http://curl.haxx.se/libcurl/c) */
#include <curl/curl.h>
/* json-c (https://github.com/json-c/json-c) */
#include <json-c/json.h>

#include "memory.h"

typedef struct {
    CURL *handle;
    struct curl_slist *headers;
    CURLcode result;
    MemoryStruct recvd;
} CURLStruct;

typedef struct {
    char uri[512];
    json_object *json;
} PostOpts;

size_t InitCURL(void *);
CURLcode PerformCURL(void *);
CURLcode PostCURL(void *,PostOpts);

#endif //SFS_FUSE_CURL_H