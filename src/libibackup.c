#include "libibackup.h"

#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <plist/plist.h>

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

char* libibackup_combine_path(const char* directory, const char* file) {
    char* full_path;
    char* file_path;

    full_path = libibackup_ensure_directory(directory);

    file_path = malloc(strlen(full_path) + strlen(file) + 1);
    strcpy(file_path, full_path);
    strcat(file_path, file);

    free(full_path);

    return file_path;
}

plist_t libibackup_load_plist(const char* directory, const char* file) {
    plist_t plist;
    char* data;
    struct stat path_stat;
    FILE* file_handle;

    char* file_path = libibackup_combine_path(directory, file);
    stat(file_path, &path_stat);

    data = malloc(path_stat.st_size);

    file_handle = fopen(file_path, "r");
    fread(data, 1, path_stat.st_size, file_handle);
    fclose(file_handle);

    plist_from_memory(data, path_stat.st_size, &plist);

    free(file_path);

    return plist;
}

bool libibackup_preflight_test_file(const char* directory, const char* file) {
    struct stat path_stat;
    char* file_path;

    file_path = libibackup_combine_path(directory, file);

    stat(file_path, &path_stat);
    free(file_path);

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
    private_client->info = libibackup_load_plist(path, "Info.plist");
    private_client->manifest_info = libibackup_load_plist(path, "Manifest.plist");
    char* manifest_database_path = libibackup_combine_path(path, "Manifest.db");
    sqlite3_open_v2(manifest_database_path, &private_client->manifest, SQLITE_OPEN_READONLY, NULL);

    *client = private_client;

    return IBACKUP_E_SUCCESS;
}

libibackup_error_t libibackup_close(libibackup_client_t client) {
    if (client != NULL) {
        free(client->path);
        plist_free(client->info);
        plist_free(client->manifest_info);
        sqlite3_close_v2(client->manifest);
        free(client);
    }

    return IBACKUP_E_SUCCESS;
}