#include "include/memory.h"
#include "include/state.h"
#include "include/curl.h"

static size_t
SFS_Login(void *userp,char username[],char password[]){
    auto StateStruct *state = (StateStruct *) userp;
    PostOpts post;
    sprintf(post.uri,"/user/login");
    post.json = json_object_new_object();
    json_object_object_add(post.json, "tokenType", json_object_new_string("permanent"));
    json_object_object_add(post.json, "username", json_object_new_string(username));
    json_object_object_add(post.json, "password", json_object_new_string(password));
    CURLcode rv = PostCURL(userp,post);
    if(CURLE_OK == rv){
        //handle session data
        struct json_object *session;
        json_object_object_get_ex(state->curl.recvd.parsed,"session",&session);
        struct json_object *value;
        json_object_object_get_ex(session,"UserId",&value);
        state->session.UserId = json_object_get_int64(value);
        json_object_object_get_ex(session,"token",&value);
        sprintf(state->session.token,"%s",json_object_get_string(value));
    }
    return rv;
}

int main(int argc, char *argv[]){
    StateStruct S;
    if(InitState((void *)&S) &&
       InitMemory((void *)&(S.curl.recvd)) &&
       InitCURL((void *)&S)
    ){
        /* do the thing */
        char username[64];
        char password[64];
        CFG_GetString((void *)&S,"username",username);
        CFG_GetString((void *)&S,"password",password);
        if(0 == SFS_Login((void *)&S,username,password)){
            printf("SFS Login succeeded, userId [%u] token [%s]\n",
                   (unsigned int)(S.session.UserId), S.session.token
            );
            sync();
        }
    }
    /* always cleanup */
    curl_slist_free_all(S.curl.headers);
    curl_easy_cleanup(S.curl.handle);
    curl_global_cleanup();
    return 0;
}