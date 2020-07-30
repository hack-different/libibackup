#include "libibackup.h"

#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

bool libibackup_preflight_test_file(const char* directory, const char* file) {
    struct stat path_stat;
    char* full_path;
    char* test_path;

    if (directory[strlen(directory) - 1] != PATH_SEPARATOR[0]) {
        full_path = malloc(strlen(directory) + 2);
        strcpy(full_path, directory);
        strcat(full_path, PATH_SEPARATOR);
    }
    else {
        full_path = malloc(strlen(directory) + 1);
        strcpy(full_path, directory);
    }

    test_path = malloc(strlen(full_path) + strlen(file) + 1);
    strcpy(test_path, full_path);
    strcat(test_path, file);

    free(full_path);

    stat(test_path, &path_stat);
    free(test_path);

    return S_ISREG(path_stat.st_mode);
}

bool libibackup_preflight_backup(const char* path) {
    struct stat path_stat;
    stat(path, &path_stat);

    if (!S_ISDIR(path_stat.st_mode)) {
        return false;
    }

    return libibackup_preflight_test_file(path, "Info.plist") &&
            libibackup_preflight_test_file(path, "Manifest.plist") &&
            libibackup_preflight_test_file(path, "Manifest.db");
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