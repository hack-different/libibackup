/*
 * libibackup.h
 *
 * Copyright (C) 2021 Rick Mark <rickmark@outlook.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef __LIBIBACKUP_H
#define __LIBIBACKUP_H

#include <stdbool.h>
#include <stdint.h>
#include <plist/plist.h>
#include <libimobiledevice-glue/collection.h>

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
    char* target;
    uint32_t type;
} libibackup_file_entry_t;

typedef struct {
    uint32_t owner;
    uint32_t group;
    uint64_t size;
    char* path;
    char* target;
} libibackup_file_metadata_t;

typedef struct {
    uint32_t file_count;
    uint32_t directory_count;
    uint32_t symlink_count;
} libibackup_domain_metrics_t;

bool libibackup_preflight_backup(const char* path);

char* libibackup_combine_path(const char* directory, const char* file);

char* libibackup_get_path_for_file_id(libibackup_client_t client, const char* file_id);

libibackup_error_t libibackup_open_backup(const char* path, libibackup_client_t* client);

libibackup_error_t libibackup_get_info(libibackup_client_t client, plist_t* info);

libibackup_error_t libibackup_list_domains(libibackup_client_t client, /* of char* */ collection_t *domains);

libibackup_error_t libibackup_list_files_for_domain(libibackup_client_t client, const char* domain, /* of libibackup_file_entry_t* */ collection_t *files);

libibackup_error_t libibackup_get_file_by_id(libibackup_client_t client, const char* file_id, char** full_path);

libibackup_error_t libibackup_remove_file_by_id(libibackup_client_t client, const char* file_id);

libibackup_error_t libibackup_get_raw_metadata_by_id(libibackup_client_t client, const char* file_id, plist_t* metadata);

libibackup_error_t libibackup_get_metadata_by_id(libibackup_client_t client, const char* file_id, libibackup_file_metadata_t* metadata);

libibackup_error_t libibackup_get_domain_metrics(libibackup_client_t client, const char* domain, libibackup_domain_metrics_t* metrics);

libibackup_error_t libibackup_close(libibackup_client_t client);

void libibackup_free(void* obj);

#ifdef __cplusplus
}
#endif

#endif // __LIBIBACKUP_H