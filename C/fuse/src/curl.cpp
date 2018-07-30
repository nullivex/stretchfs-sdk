//
// CURL Functions
//

#include <string>
#include "../include/memory.hpp"
#include "../include/curl.hpp"
#include "../include/state.hpp"

static size_t
WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
#ifdef DEBUG
    printf("[");
#endif
    size_t realsize = size * nmemb;
    auto *mem = (MemoryStruct *)userp;

    size_t cur_pages = (mem->size / mem->page);
    size_t new_pages= ((mem->ptr + realsize + 1) / mem->page) + 1;
    if(cur_pages != new_pages){
        mem->memory = (char*)realloc(mem->memory, mem->page * new_pages);
        if(nullptr == mem->memory) {
            /* out of memory! */
            printf("not enough memory (realloc returned nullptr for %u->%u)\n",
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

bool
ResetCURL(void *userp){
    auto *state = (StateStruct *) userp;
    if(state->curl.handle){
        curl_easy_reset(state->curl.handle);
#ifdef DEBUG
        curl_easy_setopt(state->curl.handle, CURLOPT_VERBOSE, 1L);
#endif
        curl_easy_setopt(state->curl.handle, CURLOPT_USERAGENT, "sfs-fuse/1.0");
        curl_easy_setopt(state->curl.handle, CURLOPT_SSL_VERIFYPEER, 0L);

        /* Headers */
        state->curl.headers = curl_slist_append(state->curl.headers, "Accept: application/json");
        state->curl.headers = curl_slist_append(state->curl.headers, "Content-Type: application/json");
        if(0 != state->session.token.length())
            state->curl.headers = curl_slist_append(state->curl.headers, ("X-STRETCHFS-Token: " + state->session.token).c_str());
        curl_easy_setopt(state->curl.handle, CURLOPT_HTTPHEADER, state->curl.headers);

        /* we pass our 'chunk' struct to the callback function */
        ResetMemory((void *) &(state->curl.recvd));
        curl_easy_setopt(state->curl.handle, CURLOPT_WRITEDATA, (void *) &(state->curl.recvd));
        /* send all data to this callback */
        curl_easy_setopt(state->curl.handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
        return TRUE;
    } else return FALSE;
}

bool
InitCURL(void *userp){
    auto *state = (StateStruct *) userp;
    state->session.token.clear();

    /* In windows, this will init the winsock stuff */
    curl_global_init(CURL_GLOBAL_ALL);

    /* get a curl handle */
    state->curl.handle = curl_easy_init();
    return ResetCURL(userp);
}

CURLcode
PerformCURL(void *userp){
    auto *state = (StateStruct *) userp;

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
    auto *state = (StateStruct *) userp;
    ResetCURL(userp);
    std::string url = state->baseurl + post.uri;
    curl_easy_setopt(state->curl.handle, CURLOPT_URL, url.c_str());
    curl_easy_setopt(state->curl.handle, CURLOPT_REFERER, url.c_str()); //"http://localhost:8160/");
    /* POST data */
    curl_easy_setopt(state->curl.handle, CURLOPT_POST, 1L);
    curl_easy_setopt(state->curl.handle, CURLOPT_POSTFIELDS, json_object_to_json_string_ext(post.json,JSON_C_TO_STRING_PLAIN));
#ifdef DEBUG
    printf("hitting POST %s with payload:\n%s\n",url.c_str(),json_object_to_json_string_ext(post.json,JSON_C_TO_STRING_PRETTY));
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

CURLcode
GetCURL(void *userp,GetOpts get){
    auto *state = (StateStruct *) userp;
    ResetCURL(userp);
    std::string url;
    if(0 == get.args.length()){
        url = state->baseurl + get.uri;
    } else {
        url = state->baseurl + get.uri + "&" + get.args;
    }
    curl_easy_setopt(state->curl.handle, CURLOPT_URL, url.c_str());
    curl_easy_setopt(state->curl.handle, CURLOPT_REFERER, url.c_str()); //"http://localhost:8160/");
    curl_easy_setopt(state->curl.handle, CURLOPT_HTTPGET, 1L);
#ifdef DEBUG
    if(0 == get.args.length()) {
        printf("hitting GET %s with no args\n", (state->baseurl + get.uri).c_str());
    } else {
        printf("hitting GET %s with args:\n%s\n", (state->baseurl + get.uri).c_str(), get.args.c_str());
    }
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