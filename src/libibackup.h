#ifndef __MOBILEBACKUP2_H
#define __MOBILEBACKUP2_H

#include "libibackup/libibackup.h"

#include <plist/plist.h>
#include <sqlite3.h>

#if defined(WIN32) || defined(_WIN32)
#define PATH_SEPARATOR "\\"
#else
#define PATH_SEPARATOR "/"
#endif


struct libibackup_client_private {
    char* path;
    plist_t info;
    plist_t manifest_info;
    sqlite3* manifest;
};

#endif