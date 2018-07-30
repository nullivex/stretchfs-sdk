#include <string>
#include "include/memory.hpp"
#include "include/state.hpp"
#include "include/curl.hpp"
#include "include/sfs_api.hpp"

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
                   (unsigned int)(S.session.UserId), S.session.token.c_str()
            );
            sync();
            SFS_List((void *)&S,",,","");
            sync();
        }
    }
    /* always cleanup */
    curl_slist_free_all(S.curl.headers);
    curl_easy_cleanup(S.curl.handle);
    curl_global_cleanup();
    return 0;
}