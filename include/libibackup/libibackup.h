/*
 * libibackup.h
 *
 * Copyright (c) 2021 Rick Mark <rickmark@outlook.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
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

typedef struct collection collection_t;

/** Error Codes */
typedef enum {
    IBACKUP_E_SUCCESS = 0,          /**< Success */
    IBACKUP_E_INVALID_ARG = -1,     /**< Invalid argument */
    IBACKUP_E_PLIST_ERROR = -2,     /**< Property list parsing or serialization error */
    IBACKUP_E_DATA_ERROR = -3,      /**< Database or data read error */
    IBACKUP_E_UNKNOWN_ERROR = -256  /**< Unknown error */
} libibackup_error_t;

/** File type flags */
#define IBACKUP_FLAG_FILE 1            /**< Regular file */
#define IBACKUP_FLAG_DIRECTORY 2       /**< Directory */
#define IBACKUP_FLAG_SYMBOLIC_LINK 4   /**< Symbolic link */

typedef struct libibackup_client_private libibackup_client_private;
typedef libibackup_client_private *libibackup_client_t; /**< The client handle representing an open backup session. */

/**
 * File entry within a backup manifest.
 */
typedef struct {
    char* file_id;         /**< Unique backup file identifier (SHA-1 hash string). */
    char* domain;          /**< Backup domain name (e.g. CameraRollDomain). */
    char* relative_path;   /**< Path of the file relative to its domain. */
    char* target;          /**< Target path if the entry is a symbolic link, NULL otherwise. */
    uint32_t type;         /**< Entry type flags (IBACKUP_FLAG_FILE, IBACKUP_FLAG_DIRECTORY, or IBACKUP_FLAG_SYMBOLIC_LINK). */
} libibackup_file_entry_t;

/**
 * File metadata parsed from Manifest.db.
 */
typedef struct {
    uint32_t owner;        /**< File owner ID. */
    uint32_t group;        /**< File group ID. */
    uint64_t size;         /**< File size in bytes. */
    char* path;            /**< Full path. */
    char* target;          /**< Symlink target path, if applicable. */
} libibackup_file_metadata_t;

/**
 * Domain metrics summary.
 */
typedef struct {
    uint32_t file_count;       /**< Total number of regular files in the domain. */
    uint32_t directory_count;  /**< Total number of directories in the domain. */
    uint32_t symlink_count;    /**< Total number of symbolic links in the domain. */
} libibackup_domain_metrics_t;

/**
 * Preflight check to determine if the given directory path contains a valid backup structure.
 *
 * Verifies that the path is a directory containing Info.plist, Manifest.plist, and Manifest.db.
 *
 * @param path The path to the backup directory.
 * @return true if the directory is a valid backup path, false otherwise.
 */
bool libibackup_preflight_backup(const char* path);

/**
 * Combines a directory path and file name into a normalized path string.
 *
 * @param directory Base directory path.
 * @param file File name or relative path.
 * @return Dynamically allocated string containing the combined path. Caller must free with free() or libibackup_free().
 */
char* libibackup_combine_path(const char* directory, const char* file);

/**
 * Resolves a backup file ID to its physical path relative to the backup directory.
 *
 * In modern iOS backups, files are hashed and stored in subdirectories named after
 * the first two hex digits of the file ID (e.g., path/ab/abcdef...).
 *
 * @param client The libibackup client handle.
 * @param file_id The file ID (hash string).
 * @return Dynamically allocated string containing the full on-disk path. Caller must free with free() or libibackup_free().
 */
char* libibackup_get_path_for_file_id(libibackup_client_t client, const char* file_id);

/**
 * Opens a backup directory and initializes a client session handle.
 *
 * @param path Path to the iOS backup directory.
 * @param client Pointer to a libibackup_client_t that will receive the initialized client handle.
 * @return IBACKUP_E_SUCCESS on success, or an error code on failure.
 */
libibackup_error_t libibackup_open_backup(const char* path, libibackup_client_t* client);

/**
 * Retrieves a copy of the Info.plist property list from the backup.
 *
 * @param client The libibackup client handle.
 * @param info Pointer to a plist_t that will receive the copy of the Info.plist structure.
 *             The caller is responsible for freeing this with plist_free().
 * @return IBACKUP_E_SUCCESS on success, or an error code on failure.
 */
libibackup_error_t libibackup_get_info(libibackup_client_t client, plist_t* info);

/**
 * Retrieves the list of all distinct backup domains.
 *
 * @param client The libibackup client handle.
 * @param domains A collection_t instance to be populated with dynamically allocated domain name strings (char*).
 * @return IBACKUP_E_SUCCESS on success, or an error code on failure.
 */
libibackup_error_t libibackup_list_domains(libibackup_client_t client, /* of char* */ collection_t *domains);

/**
 * Retrieves the list of files and directories for a specified domain.
 *
 * @param client The libibackup client handle.
 * @param domain The domain name to query (e.g. "CameraRollDomain").
 * @param files A collection_t instance to be populated with libibackup_file_entry_t* pointers.
 * @return IBACKUP_E_SUCCESS on success, or an error code on failure.
 */
libibackup_error_t libibackup_list_files_for_domain(libibackup_client_t client, const char* domain, /* of libibackup_file_entry_t* */ collection_t *files);

/**
 * Resolves a backup file ID to its absolute on-disk path.
 *
 * @param client The libibackup client handle.
 * @param file_id The file ID (hash string).
 * @param full_path Pointer to a char* that will receive the allocated path string. Caller must free with free() or libibackup_free().
 * @return IBACKUP_E_SUCCESS on success, or an error code on failure.
 */
libibackup_error_t libibackup_get_file_by_id(libibackup_client_t client, const char* file_id, char** full_path);

/**
 * Removes a file entry from the backup's Manifest.db database.
 *
 * @param client The libibackup client handle.
 * @param file_id The file ID to remove from the manifest.
 * @return IBACKUP_E_SUCCESS on success, or an error code on failure.
 */
libibackup_error_t libibackup_remove_file_by_id(libibackup_client_t client, const char* file_id);

/**
 * Retrieves the raw plist metadata blob for a given file ID from Manifest.db.
 *
 * @param client The libibackup client handle.
 * @param file_id The file ID.
 * @param metadata Pointer to a plist_t that will receive the parsed plist metadata.
 *                 Caller is responsible for freeing with plist_free().
 * @return IBACKUP_E_SUCCESS on success, or an error code on failure.
 */
libibackup_error_t libibackup_get_raw_metadata_by_id(libibackup_client_t client, const char* file_id, plist_t* metadata);

/**
 * Parses and returns structured metadata for a given file ID.
 *
 * @param client The libibackup client handle.
 * @param file_id The file ID.
 * @param metadata Pointer to a libibackup_file_metadata_t struct that will receive the parsed metadata.
 * @return IBACKUP_E_SUCCESS on success, or an error code on failure.
 */
libibackup_error_t libibackup_get_metadata_by_id(libibackup_client_t client, const char* file_id, libibackup_file_metadata_t* metadata);

/**
 * Retrieves aggregate metrics (file count, directory count, symlink count) for a specific domain.
 *
 * @param client The libibackup client handle.
 * @param domain The domain name to query.
 * @param metrics Pointer to a libibackup_domain_metrics_t struct to populate with counts.
 * @return IBACKUP_E_SUCCESS on success, or an error code on failure.
 */
libibackup_error_t libibackup_get_domain_metrics(libibackup_client_t client, const char* domain, libibackup_domain_metrics_t* metrics);

/**
 * Closes the backup session and releases all associated resources.
 *
 * @param client The libibackup client handle to close.
 * @return IBACKUP_E_SUCCESS on success, or an error code on failure.
 */
libibackup_error_t libibackup_close(libibackup_client_t client);

/**
 * Helper function to free memory allocated by libibackup.
 *
 * @param obj Pointer to the memory block to free.
 */
void libibackup_free(void* obj);

#ifdef __cplusplus
}
#endif

#endif // __LIBIBACKUP_H