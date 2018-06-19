#ifndef SFS_FUSE_CURL_H
#define SFS_FUSE_CURL_H

/* libcurl (http://curl.haxx.se/libcurl/c) */
#include <curl/curl.h>
/* json-c (https://github.com/json-c/json-c) */
#include <json-c/json.h>

#include "memory.h"

struct CURLStruct {
    CURL *handle;
    struct curl_slist *headers;
    CURLcode result;
    struct MemoryStruct recvd;
};

typedef struct {
    char uri[512];
    json_object *json;
} PostOpts;

void InitCURL(void *);
size_t PerformCURL(void *);
size_t PostCURL(void *,PostOpts);

#endif //SFS_FUSE_CURL_H
