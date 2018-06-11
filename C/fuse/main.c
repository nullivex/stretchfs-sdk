#include <sys/stat.h>
//#include <stdio.h>
//#include <string.h>
//#include <errno.h>
//#include <time.h>
//#include <sys/types.h>
//#include <fcntl.h>
//#include <io.h>

//#define DEBUG 1

/* libcurl (http://curl.haxx.se/libcurl/c) */
#include <curl/curl.h>
/* json-c (https://github.com/json-c/json-c) */
#include <json-c/json.h>

#define CONFIGFILE "config.json"

/*
 * Map low I/O functions for MS. This allows us to disable MS language
 * extensions for maximum portability.
 */
#define open            _open
#define read            _read
#define write           _write
#define close           _close
#define stat            _stat
#define fstat           _fstat
#define mkdir           _mkdir
#define snprintf        _snprintf
#if _MSC_VER <= 1200 /* Versions below VC++ 6 */
#define vsnprintf       _vsnprintf
#endif
#define O_RDONLY        _O_RDONLY
#define O_BINARY        _O_BINARY
#define O_CREAT         _O_CREAT
#define O_WRONLY        _O_WRONLY
#define O_TRUNC         _O_TRUNC
#define S_IREAD         _S_IREAD
#define S_IWRITE        _S_IWRITE
#define S_IFDIR         _S_IFDIR

struct json_object *cfg;

void cfg_get_string(const char * key, char passback[]);
void cfg_get_string(const char * key, char passback[]){
    struct json_object *value;
    json_object_object_get_ex(cfg,key,&value);
    sprintf(passback,"%s",json_object_get_string(value));
}

void sync();
void sync(){ fflush(stdout); fflush(stderr); }

int main(int argc, char *argv[]){
    struct stat st;
    if(stat(CONFIGFILE, &st) != 0){
        fprintf(stderr, "cannot stat file '%s': ", CONFIGFILE);
        perror("");
        exit(-1);
    }
    unsigned int size = (unsigned int)(st.st_size);
#ifdef DEBUG
    printf("Loading cfg from %s, size: %d\n",CONFIGFILE,size); sync();
#endif

    FILE *fd;
    errno_t err;
    if((err = fopen_s(&fd,CONFIGFILE,"r")) != 0){
        fprintf(stderr, "cannot open file '%s': %s\n", CONFIGFILE, strerror(err));
        exit(-1);
    } else {
        char *inbuf = NULL;
        inbuf = (char *) malloc(size+2);
        if(0>fread(inbuf,size,1,fd)){
            fprintf(stderr, "cannot read file '%s': ", CONFIGFILE);
            perror("");
            exit(-1);
        }
        fclose(fd);
        inbuf[size+1] = '\0';
        cfg = json_tokener_parse(inbuf);
        free(inbuf);
    }
#ifdef DEBUG
    printf("got cfg:\n%s\n",json_object_to_json_string_ext(cfg,JSON_C_TO_STRING_PRETTY)); sync();
#endif

    char baseurl[255], username[64], password[64];
    cfg_get_string("baseurl",baseurl);
    cfg_get_string("username",username);
    cfg_get_string("password",password);

    enum json_tokener_error jerr = json_tokener_success;    /* json parse error */

    /* In windows, this will init the winsock stuff */
    curl_global_init(CURL_GLOBAL_ALL);

    /* get a curl handle */
    CURL *curl;
    curl = curl_easy_init();
    if(curl) {
        char url[512];
        sprintf(url,"%s/user/login",baseurl);
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_REFERER, url); //"http://localhost:8160/");
        //curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        /* Headers */
        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Accept: application/json");
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        /* POST data */
        json_object *json;
        json = json_object_new_object();
        json_object_object_add(json, "tokenType", json_object_new_string("permanent"));
        json_object_object_add(json, "username", json_object_new_string(username));
        json_object_object_add(json, "password", json_object_new_string(password));
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_object_to_json_string_ext(json,JSON_C_TO_STRING_PLAIN));
        /* do the thing */
#ifdef DEBUG
        printf("hitting %s\nwith payload:\n%s\n",url,json_object_to_json_string_ext(json,JSON_C_TO_STRING_PRETTY));
        sync();
#endif
        CURLcode res;
        res = curl_easy_perform(curl);

        curl_slist_free_all(headers);

        /* Check for errors */
        if(res != CURLE_OK){
            fprintf(stderr,"curl_easy_perform() failed: %s\n",
                   curl_easy_strerror(res));
        }

        /* always cleanup */
        curl_easy_cleanup(curl);
    }
    curl_global_cleanup();
    return 0;
}