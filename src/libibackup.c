#include "libibackup.h"

#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

char* libibackup_ensure_directory(const char* path) {
    char* full_path;

    if (path[strlen(path) - 1] != PATH_SEPARATOR[0]) {
        full_path = malloc(strlen(path) + 2);
        strcpy(full_path, path);
        strcat(full_path, PATH_SEPARATOR);
    }
    else {
        full_path = malloc(strlen(path) + 1);
        strcpy(full_path, path);
    }

    return full_path;
}

bool libibackup_preflight_test_file(const char* directory, const char* file) {
    struct stat path_stat;
    char* full_path;
    char* test_path;

    full_path = libibackup_ensure_directory(directory);

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

    struct libibackup_client_private* private_client = malloc(sizeof(struct libibackup_client_private));
    private_client->path = libibackup_ensure_directory(path);

    *client = private_client;

    return IBACKUP_E_SUCCESS;
}

libibackup_error_t libibackup_close(libibackup_client_t client) {
    if (client != NULL) {
        free(client->path);
        free(client);
    }

    return IBACKUP_E_SUCCESS;
}