//
// Created by tbutler on 7/7/2018.
//

#include <string>
#include "../include/sfs_api.hpp"
#include "../include/memory.hpp"
#include "../include/state.hpp"
#include "../include/curl.hpp"

size_t
SFS_Login(void *userp,std::string username,std::string password){
    auto *state = (StateStruct *) userp;
    PostOpts post;
    state->baseurl = "https://127.0.100.1:8161";
    post.uri = "/user/login";
    post.json = json_object_new_object();
    json_object_object_add(post.json, "tokenType", json_object_new_string("permanent"));
    json_object_object_add(post.json, "username", json_object_new_string(username.c_str()));
    json_object_object_add(post.json, "password", json_object_new_string(password.c_str()));
    CURLcode rv = PostCURL(userp,post);
    if(CURLE_OK == rv){
        //handle session data
        struct json_object *session;
        json_object_object_get_ex(state->curl.recvd.parsed,"session",&session);
        struct json_object *value;
        json_object_object_get_ex(session,"UserId",&value);
        state->session.UserId = json_object_get_int64(value);
        json_object_object_get_ex(session,"token",&value);
        state->session.token = json_object_get_string(value);
    }
    return rv;
}

size_t
SFS_List(void *userp,std::string path,std::string search){
    auto *state = (StateStruct *) userp;
    PostOpts post;
    post.uri = "/file/list";
    post.json = json_object_new_object();
    json_object_object_add(post.json, "token", json_object_new_string(state->session.token.c_str()));
    json_object_object_add(post.json, "json", json_object_new_boolean(TRUE));
    json_object_object_add(post.json, "path", json_object_new_string(path.c_str()));
    json_object_object_add(post.json, "search", json_object_new_string(search.c_str()));
    CURLcode rv = PostCURL(userp,post);
    if(CURLE_OK == rv){
        //handle directory data
        printf("%s succeeded, Response:\n%s\n",__FUNCTION__,json_object_to_json_string_ext(state->curl.recvd.parsed,JSON_C_TO_STRING_PRETTY));
    }
    return rv;
}
