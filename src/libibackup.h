#ifndef __MOBILEBACKUP2_H
#define __MOBILEBACKUP2_H

#include "libibackup/libibackup.h"

#include <sqlite3.h>

#define EXPORT __attribute__((visibility("default")))

#if defined(WIN32) || defined(_WIN32)
#define PATH_SEPARATOR "\\"
#else
#define PATH_SEPARATOR "/"
#endif

const char* integrity_check_query = "PRAGMA integrity_check(1000)";
const char* domains_query = "SELECT DISTINCT domain FROM Files";
const char* domains_count_query = "SELECT COUNT(DISTINCT domain) FROM Files";
const char* domain_count_file_query = "SELECT COUNT(*) FROM Files WHERE domain = ?";
const char* domain_file_query = "SELECT fileID, domain, relativePath, flags FROM Files WHERE domain = ?";
const char* file_metadata_query = "SELECT file FROM Files WHERE fileID = ?";
const char* file_query = "SELECT fileID, domain, relativePath, flags, file FROM Files WHERE fileID = ?";
const char* delete_file_query = "DELETE FROM Files WHERE fileID = ?";
const char* create_new_file_query = "INSERT INTO Files (fileID, domain, relativePath, flags, file) VALUES (?, ?, ?, 1, ?)";
const char* update_file_query = "UPDATE Files SET fileID = ? WHERE fileID = ?";
const char* update_file_metadata_query = "UPDATE Files SET file = ? WHERE fileID = ?";

struct libibackup_client_private {
    char* path;
    plist_t info;
    plist_t manifest_info;
    sqlite3* manifest;
};

#endif