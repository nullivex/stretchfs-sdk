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
enum json_tokener_error jerr = json_tokener_success;    /* json parse error */

#ifdef _MSC_VER
#include <windows.h>
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
#endif

void sync();
void sync(){ fflush(stdout); fflush(stderr); }

#define CONFIGFILE "config.json"
#define PAGESIZE 4096
struct MemoryStruct {
    char *memory;
    size_t page;
    size_t size;
    size_t ptr;
    struct json_object *parsed;
};
struct CURLStruct {
    CURL *handle;
    struct curl_slist *headers;
    CURLcode result;
    struct MemoryStruct recvd;
};
struct StateStruct {
    struct json_object *cfg;
    char baseurl[255];
    char username[64];
    char password[64];
    struct CURLStruct curl;
};
void
InitState(void *userp) {
    struct StateStruct *state = (struct StateStruct *) userp;
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
    struct json_object *value;
    #define CFG_GET_STRING(KEY) \
        json_object_object_get_ex(state->cfg,#KEY,&value); \
        sprintf(state->KEY,"%s",json_object_get_string(value));
    CFG_GET_STRING(baseurl);
    CFG_GET_STRING(username);
    CFG_GET_STRING(password);
}

void
InitMemory(void *userp) {
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;
#ifdef _MSC_VER
    SYSTEM_INFO siSysInfo;
    GetSystemInfo(&siSysInfo);
    // Display the contents of the SYSTEM_INFO structure.
    /*
    printf("Hardware information: \n");
    printf("  OEM ID: %u\n", siSysInfo.dwOemId);
    printf("  Number of processors: %u\n",siSysInfo.dwNumberOfProcessors);
    printf("  Page size: %u\n", siSysInfo.dwPageSize);
    printf("  Processor type: %u\n", siSysInfo.dwProcessorType);
    printf("  Minimum application address: %lx\n",siSysInfo.lpMinimumApplicationAddress);
    printf("  Maximum application address: %lx\n",siSysInfo.lpMaximumApplicationAddress);
    printf("  Active processor mask: %u\n",siSysInfo.dwActiveProcessorMask);
     */
    mem->page = siSysInfo.dwPageSize;
#else
    mem->page = PAGESIZE;
#endif
    mem->memory = malloc(mem->page); /* will be grown as needed by the realloc in WriteMemoryCallback */
    mem->size = mem->page;           /* size is total alloc bytes */
    mem->ptr = 0;                    /* no data at this point */
    mem->memory[mem->ptr] = 0;       /* ensure null at first byte (cstr thus no need to memset entire page) */
#ifdef DEBUG
    printf("Allocated receive buffer (%u bytes)\n",(unsigned int)mem->size);
#endif
}

static size_t
WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
#ifdef DEBUG
    printf("[");
#endif
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;

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

void
InitCURL(void *userp) {
    struct StateStruct *state = (struct StateStruct *) userp;
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
    }
}
static size_t
PerformCURL(void *userp){
    struct StateStruct *state = (struct StateStruct *) userp;

    state->curl.result = curl_easy_perform(state->curl.handle);
    /* Check for errors */
    if(state->curl.result != CURLE_OK){
        fprintf(stderr,"curl_easy_perform() failed: %s\n",
                curl_easy_strerror(state->curl.result));
        return -1;
    } else return 0;
}

int main(int argc, char *argv[]){
    struct StateStruct S;
    InitState((void *)&S);
    InitMemory((void *)&(S.curl.recvd));
    InitCURL((void *)&S);
    if(S.curl.handle){
        char url[512];
        sprintf(url,"%s/user/login",S.baseurl);
        curl_easy_setopt(S.curl.handle, CURLOPT_URL, url);
        curl_easy_setopt(S.curl.handle, CURLOPT_REFERER, url); //"http://localhost:8160/");
        /* POST data */
        curl_easy_setopt(S.curl.handle, CURLOPT_POST, 1L);
        json_object *json;
        json = json_object_new_object();
        json_object_object_add(json, "tokenType", json_object_new_string("permanent"));
        json_object_object_add(json, "username", json_object_new_string(S.username));
        json_object_object_add(json, "password", json_object_new_string(S.password));
        curl_easy_setopt(S.curl.handle, CURLOPT_POSTFIELDS, json_object_to_json_string_ext(json,JSON_C_TO_STRING_PLAIN));

        /* do the thing */
#ifdef DEBUG
        printf("hitting %s\nwith payload:\n%s\n",url,json_object_to_json_string_ext(json,JSON_C_TO_STRING_PRETTY));
        sync();
#endif
        if(0 == PerformCURL((void *)&S)){
            S.curl.recvd.parsed = json_tokener_parse(S.curl.recvd.memory);
#ifdef DEBUG
            printf("\nResult:\n%s\n",S.curl.recvd.memory);
            printf("parsed result:\n%s\n",json_object_to_json_string_ext(S.curl.recvd.parsed,JSON_C_TO_STRING_PRETTY));
            sync();
#endif
        }

        /* always cleanup */
        curl_slist_free_all(S.curl.headers);
        curl_easy_cleanup(S.curl.handle);
    }
    curl_global_cleanup();
    return 0;
}