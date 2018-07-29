//
// StateStruct and methods
//

#include "../include/state.hpp"

void sync(){ fflush(stdout); fflush(stderr); }

void
CFG_GetString(void *userp, const char *key, char *dest){
    auto *state = (StateStruct *) userp;
    struct json_object *value;
    json_object_object_get_ex(state->cfg,key,&value);
    sprintf(dest,"%s",json_object_get_string(value));
}

size_t
InitState(void *userp){
    auto *state = (StateStruct *) userp;
    state->curl.handle = nullptr;
    state->curl.headers = nullptr;

    struct stat st{};
    if(stat(CONFIGFILE, &st) != 0){
        fprintf(stderr, "cannot stat file '%s': ", CONFIGFILE);
        perror("");
        return FALSE;
    }
    auto size = (unsigned int)(st.st_size);
#ifdef DEBUG
    printf("Loading cfg from %s, size: %d\n",CONFIGFILE,size);
    sync();
#endif

    FILE *fd;
    errno_t err;
    if((err = fopen_s(&fd,CONFIGFILE,"r")) != 0){
        fprintf(stderr, "cannot open file '%s': %s\n", CONFIGFILE, strerror(err));
        return FALSE;
    } else {
        char *inbuf = nullptr;
        inbuf = (char *) malloc(size+1);
        if(0>fread(inbuf,size,1,fd)){
            fprintf(stderr, "cannot read file '%s': ", CONFIGFILE);
            perror("");
            exit(-1);
        }
        fclose(fd);
        inbuf[size] = '\0';
        state->cfg = json_tokener_parse(inbuf);
        free(inbuf);
    }
#ifdef DEBUG
    printf("got cfg:\n%s\n",json_object_to_json_string_ext(state->cfg,JSON_C_TO_STRING_PRETTY));
    sync();
#endif
    CFG_GetString(userp,"baseurl",state->baseurl);
    return TRUE;
}