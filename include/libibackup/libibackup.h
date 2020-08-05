#ifndef LIBIBACKUP_H

#include <stdbool.h>
#include <plist/plist.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Error Codes */
typedef enum {
    IBACKUP_E_SUCCESS = 0,
    IBACKUP_E_INVALID_ARG = -1,
    IBACKUP_E_PLIST_ERROR = -2,
    IBACKUP_E_DATA_ERROR = -3,
    IBACKUP_E_UNKNOWN_ERROR = -256
} libibackup_error_t;

#define IBACKUP_FLAG_FILE 1
#define IBACKUP_FLAG_DIRECTORY 2
#define IBACKUP_FLAG_SYMBOLIC_LINK 4

typedef struct libibackup_client_private libibackup_client_private;
typedef libibackup_client_private *libibackup_client_t; /**< The client handle. */

typedef struct {
    char* file_id;
    char* domain;
    char* relative_path;
} libibackup_file_entry_t;

void libibackup_set_debug(bool debug);

bool libibackup_preflight_backup(const char* path);

libibackup_error_t libibackup_open_backup(const char* path, libibackup_client_t* client);

libibackup_error_t libibackup_get_info(libibackup_client_t client, plist_t* info);

libibackup_error_t libibackup_list_domains(libibackup_client_t client, char*** domains);

libibackup_error_t libibackup_list_files_for_domain(libibackup_client_t client, char* domain, libibackup_file_entry_t*** entries);

libibackup_error_t libibackup_get_file_by_id(libibackup_client_t client, char* file_id, char** full_path);

libibackup_error_t libibackup_remove_file_by_id(libibackup_client_t client, char* file_id);

libibackup_error_t libibackup_get_file_metadata_by_id(libibackup_client_t client, char* file_id, plist_t* metadata);

libibackup_error_t libibackup_close(libibackup_client_t client);


#ifdef __cplusplus
}
#endif

#define LIBIBACKUP_H
#endif