#include "libibackup.h"

#include <stdlib.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <plist/plist.h>
#include <libimobiledevice-glue/sha1.h>
#include <libimobiledevice-glue/debug.h>


int64_t libibackup_manifest_query_count(sqlite3* database, const char* query, const char* parameter) {
    sqlite3_stmt *count_statement;

    assert(database);
    assert(query);

    LIMD_log_message(LIMD_DEBUG, "backup", "Preparing Count Statement\n");

    sqlite3_prepare_v3(database, query, strlen(query), SQLITE_PREPARE_NORMALIZE, &count_statement, NULL);

    if (parameter != NULL) {
        sqlite3_bind_text(count_statement, 1, parameter, strlen(parameter), NULL);
    }

    if (sqlite3_step(count_statement) != SQLITE_ROW) {
        return IBACKUP_E_DATA_ERROR;
    }

    uint64_t count = sqlite3_column_int64(count_statement, 0);

    LIMD_log_message(LIMD_DEBUG, "backup", "Read count %llu\n", count);

    sqlite3_finalize(count_statement);

    LIMD_log_message(LIMD_DEBUG, "backup", "Finalizing Domain Count Statement\n");

    return count;
}

char* libibackup_ensure_directory(const char* path) {
    assert(path);

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

EXPORT char* libibackup_combine_path(const char* directory, const char* file) {
    assert(directory);
    assert(file);

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

EXPORT bool libibackup_preflight_backup(const char* path) {
    struct stat path_stat;
    stat(path, &path_stat);

    if (!S_ISDIR(path_stat.st_mode)) {
        return false;
    }

    return libibackup_preflight_test_file(path, "Info.plist") &&
            libibackup_preflight_test_file(path, "Manifest.plist") &&
            libibackup_preflight_test_file(path, "Manifest.db");
}

EXPORT libibackup_error_t libibackup_get_file_by_id(libibackup_client_t client, const char* file_id, char** full_path)
{
    char* file_component = malloc(strlen(file_id) + 4);
    file_component[0] = file_id[0];
    file_component[1] = file_id[1];
    file_component[2] = PATH_SEPARATOR[0];
    strncpy(file_component + 3, file_id, strlen(file_id));

    char* path = libibackup_combine_path(client->path, file_component);
    *full_path = path;
    free(file_component);

    LIMD_log_message(LIMD_DEBUG, "backup", "Full File Path for %s is %s\n", file_id, path);

    return IBACKUP_E_SUCCESS;
}

EXPORT libibackup_error_t libibackup_remove_file_by_id(libibackup_client_t client, const char* file_id) {
    sqlite3_stmt *delete_file_statement;
    char* file_path;
    libibackup_get_file_by_id(client, file_id, &file_path);

    sqlite3_prepare_v3(client->manifest, delete_file_query, strlen(delete_file_query), SQLITE_PREPARE_NORMALIZE, &delete_file_statement, NULL);

    sqlite3_bind_text(delete_file_statement, 1, file_path, strlen(file_path), NULL);

    sqlite3_step(delete_file_statement);

    return IBACKUP_E_SUCCESS;
}

EXPORT char* libibackup_get_path_for_file_id(libibackup_client_t client, const char* file_id) {
    char* file_path = malloc(strlen(file_id) + 4);
    file_path[0] = file_id[0];
    file_path[1] = file_id[1];
    file_path[2] = PATH_SEPARATOR[0];
    strcpy(file_path + 3, file_id);

    return libibackup_combine_path(client->path, file_path);
}

EXPORT libibackup_error_t libibackup_add_file(libibackup_client_t client, const char* domain, const char* relative_path, const void* data, const size_t length) {
    assert(client);
    
    unsigned char *file_hash = malloc(SHA1_HASH_LENGTH_BYTES + 1);
    memset(file_hash, 0, SHA1_HASH_BLOCK_SIZE_BYTES + 1);

    LIMD_SHA1(data, length, file_hash);
    file_hash[SHA1_HASH_LENGTH_BYTES] = '\0';

    char* full_data_path = libibackup_get_path_for_file_id(client, (const char*)file_hash);

    FILE* output_data_file = fopen(full_data_path, "w");

    fwrite(data, length, 1, output_data_file);

    fclose(output_data_file);

    sqlite3_stmt *insert_file_statement;
    sqlite3_prepare_v3(client->manifest, create_new_file_query, strlen(create_new_file_query), SQLITE_PREPARE_NORMALIZE, &insert_file_statement, NULL);

    if (sqlite3_step(insert_file_statement) != SQLITE_DONE) {

    }

    return IBACKUP_E_SUCCESS;
}

EXPORT libibackup_error_t libibackup_get_raw_metadata_by_id(libibackup_client_t client, const char* file_id, plist_t* metadata) {
    sqlite3_stmt *query_metadata;

    LIMD_log_message(LIMD_DEBUG, "backup", "Query for Metadata for ID %s\n", file_id);

    sqlite3_prepare_v3(client->manifest, file_metadata_query, strlen(file_metadata_query), SQLITE_PREPARE_NORMALIZE, &query_metadata, NULL);

    sqlite3_bind_text(query_metadata, 1, file_id, strlen(file_id), NULL);

    if (sqlite3_step(query_metadata) == SQLITE_ROW) {
        LIMD_log_message(LIMD_DEBUG, "backup", "Metadata for file found\n");

        const void* metadata_blob = sqlite3_column_blob(query_metadata, 0);
        int metadata_size = sqlite3_column_bytes(query_metadata, 0);

        plist_from_memory(metadata_blob, metadata_size, metadata);
    }

    return IBACKUP_E_SUCCESS;
}

EXPORT libibackup_error_t libibackup_open_backup(const char* path, libibackup_client_t* client) {
    assert(path);
    assert(client);

    if (libibackup_preflight_backup(path) == false) {
        return IBACKUP_E_INVALID_ARG;
    }

    struct libibackup_client_private* private_client = malloc(sizeof(struct libibackup_client_private));
    private_client->path = libibackup_ensure_directory(path);

    LIMD_log_message(LIMD_DEBUG, "backup", "Opening Info.plist\n");

    private_client->info = libibackup_load_plist(path, "Info.plist");

    LIMD_log_message(LIMD_DEBUG, "backup", "Opening Manifest.plist\n");

    private_client->manifest_info = libibackup_load_plist(path, "Manifest.plist");
    char* manifest_database_path = libibackup_combine_path(path, "Manifest.db");
    int db_result = sqlite3_open_v2(manifest_database_path, &private_client->manifest, SQLITE_OPEN_READWRITE, NULL);

    LIMD_log_message(LIMD_DEBUG, "backup", "Opening Manifest DB result: %d\n", db_result);


    *client = private_client;

    sqlite3_stmt *integrity_check;
    LIMD_log_message(LIMD_DEBUG, "backup", "Performing integrity check:\n");
    sqlite3_prepare_v3(private_client->manifest, integrity_check_query, strlen(integrity_check_query), SQLITE_PREPARE_NORMALIZE, &integrity_check, NULL);
    while (sqlite3_step(integrity_check) == SQLITE_ROW) {
        printf("%s\n", sqlite3_column_text(integrity_check, 0));
    }

    return IBACKUP_E_SUCCESS;
}

EXPORT libibackup_error_t libibackup_get_info(libibackup_client_t client, plist_t* info) {
    *info = plist_copy(client->info);

    return IBACKUP_E_SUCCESS;
}

EXPORT libibackup_error_t libibackup_list_domains(libibackup_client_t client, /* of char* */ collection_t *domains) {
    assert(client);
    assert(domains);

    uint32_t count = libibackup_manifest_query_count(client->manifest, domains_count_query, NULL);
    collection_ensure_capacity(domains, count);

    sqlite3_stmt *query_domains;
    int64_t index = 0;

    LIMD_log_message(LIMD_DEBUG, "backup", "Preparing Domain Statement\n");
    
    sqlite3_prepare_v3(client->manifest, domains_query, strlen(domains_query), SQLITE_PREPARE_NORMALIZE, &query_domains, NULL);

    while(sqlite3_step(query_domains) == SQLITE_ROW) {
        const char* domain_from_db = (const char*)sqlite3_column_text(query_domains, 0);

        LIMD_log_message(LIMD_DEBUG, "backup", "Found Domain: %s\n", domain_from_db);
        
        domains->list[index] = malloc(strlen(domain_from_db) + 1);
        strcpy(domains->list[index], domain_from_db);
        index++;
    }

    sqlite3_finalize(query_domains);

    return IBACKUP_E_SUCCESS;
}

EXPORT libibackup_error_t libibackup_get_domain_metrics(libibackup_client_t client, const char* domain, libibackup_domain_metrics_t* metrics) {
    sqlite3_stmt *metrics_query;
    sqlite3_prepare_v3(client->manifest, domain_count_file_grouped_query, strlen(domain_count_file_grouped_query), SQLITE_PREPARE_NORMALIZE, &metrics_query, NULL);
    sqlite3_bind_text(metrics_query, 1, domain, strlen(domain), NULL);

    metrics->file_count = 0;
    metrics->directory_count = 0;
    metrics->symlink_count = 0;

    while (sqlite3_step(metrics_query) == SQLITE_ROW) {
        uint32_t count = sqlite3_column_int(metrics_query, 0);
        switch (sqlite3_column_int(metrics_query, 1)) {
            case IBACKUP_FLAG_FILE:
                metrics->file_count = count;
                break;
            case IBACKUP_FLAG_DIRECTORY:
                metrics->directory_count = count;
                break;
            case IBACKUP_FLAG_SYMBOLIC_LINK:
                metrics->symlink_count = count;
                break;
        }
    }

    return IBACKUP_E_SUCCESS;
}

EXPORT libibackup_error_t libibackup_get_metadata_by_id(libibackup_client_t client, const char* file_id, libibackup_file_metadata_t* metadata) {
    memset(metadata, 0, sizeof(libibackup_file_metadata_t));

    plist_t raw_metadata;
    libibackup_get_raw_metadata_by_id(client, file_id, &raw_metadata);

    plist_t objects = plist_dict_get_item(raw_metadata, "$objects");
    plist_t mb_file = plist_array_get_item(objects, 1);
    plist_t size_item = plist_dict_get_item(mb_file, "Size");
    plist_t target_item = plist_dict_get_item(mb_file, "Target");
    plist_get_uint_val(size_item, &metadata->size);

    if (target_item != NULL) {
        uint64_t index;
        plist_get_uid_val(target_item, &index);
        plist_t target_string_value = plist_array_get_item(objects, index);
        plist_get_string_val(target_string_value, &metadata->target);

        plist_free(target_string_value);
        plist_free(target_item);
    }

    plist_free(size_item);
    plist_free(mb_file);
    plist_free(objects);

    plist_free(raw_metadata);

    return IBACKUP_E_SUCCESS;
}

EXPORT libibackup_error_t libibackup_list_files_for_domain(libibackup_client_t client, const char* domain, /* of libibackup_file_entry_t */ collection_t *files) {
    uint32_t count, index;

    count = libibackup_manifest_query_count(client->manifest, domain_count_file_query, domain);

    LIMD_log_message(LIMD_DEBUG, "backup", "Files Count for Domain %s is %d\n", domain, count);

    collection_ensure_capacity(files, count);

    sqlite3_stmt *query_files;

    int result = sqlite3_prepare_v3(client->manifest, domain_file_query, strlen(domain_file_query), SQLITE_PREPARE_NORMALIZE, &query_files, NULL);

    sqlite3_bind_text(query_files, 1, domain, strlen(domain), NULL);

    LIMD_log_message(LIMD_DEBUG, "backup", "File query prepare result %i\n", result);

    index = 0;
    while(sqlite3_step(query_files) == SQLITE_ROW) 
    {
        libibackup_file_entry_t *entry = malloc(sizeof(libibackup_file_entry_t));
        files->list[index] = entry;

        char* relative_path = (char*)sqlite3_column_text(query_files, 2);
        char* file_id = (char*)sqlite3_column_text(query_files, 0);

        entry->relative_path = malloc(strlen(relative_path) + 1);
        entry->domain = malloc(strlen(domain) + 1);
        entry->file_id = malloc(strlen(file_id) + 1);
        entry->type = sqlite3_column_int(query_files, 3);
        strcpy(entry->file_id, file_id);
        strcpy(entry->relative_path, relative_path);
        strcpy(entry->domain, domain);

        index++;
    }

    sqlite3_finalize(query_files);

    return IBACKUP_E_SUCCESS;
}

EXPORT libibackup_error_t libibackup_close(libibackup_client_t client) {
    if (client != NULL) {
        free(client->path);
        plist_free(client->info);
        plist_free(client->manifest_info);
        sqlite3_close_v2(client->manifest);
        free(client);
    }

    return IBACKUP_E_SUCCESS;
}

EXPORT void libibackup_free(void* object) {
    free(object);
}