//
// Created by tbutler on 7/7/2018.
//

#ifndef SFS_FUSE_SFS_API_H
#define SFS_FUSE_SFS_API_H

#include <cstdarg>
#include <string>

size_t SFS_Login(void *, std::string, std::string);
size_t SFS_List(void *,std::string,std::string);

#endif //SFS_FUSE_SFS_API_H
