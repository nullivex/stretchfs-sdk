#ifndef SFS_FUSE_STATE_H
#define SFS_FUSE_STATE_H
#define CONFIGFILE "config.json"

#include <string>
#include <sys/stat.h>
#include "curl.hpp"

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

typedef struct {
    int64_t UserId;
    std::string token;
} SessionStruct;

typedef struct {
    struct json_object *cfg;
    std::string baseurl;
    CURLStruct curl;
    SessionStruct session;
} StateStruct;

void sync();
void CFG_GetString(void *, const char *, char *);
bool InitState(void *);

#endif //SFS_FUSE_STATE_H