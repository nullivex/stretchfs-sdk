//
// CURL Functions
//

#include "../include/memory.h"
#include "../include/curl.h"
#include "../include/state.h"

static size_t
WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
#ifdef DEBUG
    printf("[");
#endif
    size_t realsize = size * nmemb;
    auto MemoryStruct *mem = (MemoryStruct *)userp;

    size_t cur_pages = (mem->size / mem->page);
    size_t new_pages= ((mem->ptr + realsize + 1) / mem->page) + 1;
    if(cur_pages != new_pages){
        mem->memory = realloc(mem->memory, mem->page * new_pages);
        if(mem->memory == NULL) {
            /* out of memory! */
            printf("not enough memory (realloc returned NULL for %u->%u)\n",
                   (unsigned int)(mem->page * cur_pages),
                   (unsigned int)(mem->page * new_pages)
            );
            return 0;
        } else {
            mem->size = mem->page * new_pages;
        }
#ifdef DEBUG
        printf("^");
#endif
    }

    memcpy(&(mem->memory[mem->ptr]), contents, realsize);
    mem->ptr += realsize;
    mem->memory[mem->ptr] = 0;

#ifdef DEBUG
    printf("%u]",(unsigned int)realsize);
    sync();
#endif
    return realsize;
}

size_t
InitCURL(void *userp){
    auto StateStruct *state = (StateStruct *) userp;
    /* In windows, this will init the winsock stuff */
    curl_global_init(CURL_GLOBAL_ALL);

    /* get a curl handle */
    state->curl.handle = curl_easy_init();
    if(state->curl.handle){
        curl_easy_setopt(state->curl.handle, CURLOPT_USERAGENT, "sfs-fuse/1.0");
        //curl_easy_setopt(state->curl.handle, CURLOPT_VERBOSE, 1L);
        curl_easy_setopt(state->curl.handle, CURLOPT_SSL_VERIFYPEER, 0L);

        /* Headers */
        state->curl.headers = curl_slist_append(state->curl.headers, "Accept: application/json");
        state->curl.headers = curl_slist_append(state->curl.headers, "Content-Type: application/json");
        curl_easy_setopt(state->curl.handle, CURLOPT_HTTPHEADER, state->curl.headers);

        /* we pass our 'chunk' struct to the callback function */
        curl_easy_setopt(state->curl.handle, CURLOPT_WRITEDATA, (void *) &(state->curl.recvd));
        /* send all data to this callback */
        curl_easy_setopt(state->curl.handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
        return TRUE;
    } else return FALSE;
}

CURLcode
PerformCURL(void *userp){
    auto StateStruct *state = (StateStruct *) userp;

    state->curl.result = curl_easy_perform(state->curl.handle);
    /* Check for errors */
    if(CURLE_OK != state->curl.result){
        fprintf(stderr,"curl_easy_perform() failed: %s\n",
                curl_easy_strerror(state->curl.result));
    }
    return state->curl.result;
}

CURLcode
PostCURL(void *userp,PostOpts post){
    auto StateStruct *state = (StateStruct *) userp;
    char url[512];
    sprintf(url,"%s/user/login",state->baseurl);
    curl_easy_setopt(state->curl.handle, CURLOPT_URL, url);
    curl_easy_setopt(state->curl.handle, CURLOPT_REFERER, url); //"http://localhost:8160/");
    /* POST data */
    curl_easy_setopt(state->curl.handle, CURLOPT_POST, 1L);
    curl_easy_setopt(state->curl.handle, CURLOPT_POSTFIELDS, json_object_to_json_string_ext(post.json,JSON_C_TO_STRING_PLAIN));
#ifdef DEBUG
    printf("hitting POST %s with payload:\n%s\n",url,json_object_to_json_string_ext(post.json,JSON_C_TO_STRING_PRETTY));
    sync();
#endif
    CURLcode rv = PerformCURL(userp);
    if(CURLE_OK == rv){
        state->curl.recvd.parsed = json_tokener_parse(state->curl.recvd.memory);
#ifdef DEBUG
        printf("\nResult:\n%s\n", state->curl.recvd.memory);
        printf("parsed result:\n%s\n", json_object_to_json_string_ext(state->curl.recvd.parsed,JSON_C_TO_STRING_PRETTY));
        sync();
#endif
    }
    return rv;
}