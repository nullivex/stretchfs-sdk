//
// StateStruct and methods
//

#include "state.h"

void sync(){ fflush(stdout); fflush(stderr); }

void
CFG_GetString(void *userp,char *key,char *dest){
    StateStruct *state = (StateStruct *) userp;
    struct json_object *value;
    json_object_object_get_ex(state->cfg,key,&value);
    sprintf(dest,"%s",json_object_get_string(value));
}

void
InitState(void *userp){
    StateStruct *state = (StateStruct *) userp;
    state->curl.handle = NULL;
    state->curl.headers = NULL;

    struct stat st;
    if(stat(CONFIGFILE, &st) != 0){
        fprintf(stderr, "cannot stat file '%s': ", CONFIGFILE);
        perror("");
        exit(-1);
    }
    unsigned int size = (unsigned int)(st.st_size);
#ifdef DEBUG
    printf("Loading cfg from %s, size: %d\n",CONFIGFILE,size);
    sync();
#endif

    FILE *fd;
    errno_t err;
    if((err = fopen_s(&fd,CONFIGFILE,"r")) != 0){
        fprintf(stderr, "cannot open file '%s': %s\n", CONFIGFILE, strerror(err));
        exit(-1);
    } else {
        char *inbuf = NULL;
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
}
