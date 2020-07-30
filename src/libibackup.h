#ifndef __MOBILEBACKUP2_H
#define __MOBILEBACKUP2_H

#include "libibackup/libibackup.h"

#if defined(WIN32) || defined(_WIN32)
#define PATH_SEPARATOR "\\"
#else
#define PATH_SEPARATOR "/"
#endif


struct libibackup_client_private {
    char* path;
};

#endif