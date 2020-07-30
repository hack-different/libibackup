#include "libibackup.h"

#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

bool libibackup_preflight_backup(const char* path) {
    struct stat path_stat;
    stat(path, &path_stat);

    if (!S_ISDIR(path_stat.st_mode)) {
        return false;
    }

    return true;
}

libibackup_error_t libibackup_open_backup(const char* path, libibackup_client_t* client) {
    if (libibackup_preflight_backup(path) == false) {
        return IBACKUP_E_INVALID_ARG;
    }

    client = malloc(sizeof(struct libibackup_client_private));


    return IBACKUP_E_SUCCESS;
}

libibackup_error_t libibackup_close(libibackup_client_t client) {
    if (client != NULL) {
        free(client);
    }

    return IBACKUP_E_SUCCESS;
}