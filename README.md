# libibackup

`libibackup` is a C library for inspecting and working with local Apple/iOS backup directories. It can open a backup, read backup metadata, enumerate domains and files, resolve backup file IDs to on-disk paths, inspect file metadata, and remove entries from the backup manifest.

The library is intended for tools and applications that need programmatic access to the contents of an iTunes/Finder-style iOS/iPadOS device backup.

## Features

- **Validation:** Validate whether a directory path contains a valid iOS backup structure (`Info.plist`, `Manifest.plist`, `Manifest.db`).
- **Session Management:** Safely open and close backup handles.
- **Backup Information:** Read `Info.plist` and `Manifest.plist` backup metadata as `plist_t` structures.
- **Domain Enumeration & Metrics:** List backup domains and query aggregate metrics (file, directory, and symlink counts) per domain.
- **File Enumeration:** List all files, directories, and symlinks associated with a specific domain.
- **Path Resolution:** Resolve hashed backup file IDs to their physical on-disk relative and absolute paths.
- **Metadata Inspection:** Read raw plist metadata from `Manifest.db` or retrieve structured file metadata (file size, owner, group, symlink target).
- **Manifest Modification:** Remove file records from the backup manifest database.

## Prerequisites & Dependencies

To build `libibackup`, you will need:
- A C99-compliant C compiler (e.g., `clang` or `gcc`)
- [CMake](https://cmake.org/) (version 3.31.6 or newer)
- `pkg-config`
- `libplist` (>= 2.0)
- `libimobiledevice-glue` (>= 1.0)
- `sqlite3`

### Installing Dependencies

#### macOS (Homebrew)
```bash
brew install cmake pkg-config libimobiledevice libimobiledevice-glue libplist sqlite3
```

#### Ubuntu / Debian
```bash
sudo apt-get update
sudo apt-get install -y cmake pkg-config build-essential \
    libimobiledevice-dev libimobiledevice-glue-dev libplist-dev libsqlite3-dev
```

## Building and Installing

### Building with CMake

1. Clone the repository:
   ```bash
   git clone https://github.com/rickmark/libibackup.git
   cd libibackup
   ```

2. Configure the build directory:
   ```bash
   cmake -B build
   ```

3. Build the library:
   ```bash
   cmake --build build
   ```

### Installing

To install the compiled library and headers to your system default location (e.g. `/usr/local`):

```bash
sudo cmake --install build
```

To install to a custom directory prefix:

```bash
cmake -B build -DCMAKE_INSTALL_PREFIX=/path/to/custom/prefix
cmake --build build
cmake --install build
```

## API Reference

Include the main header in your C/C++ project:
```c
#include <libibackup/libibackup.h>
```

### Data Types and Constants

#### Error Codes (`libibackup_error_t`)
```c
typedef enum {
    IBACKUP_E_SUCCESS = 0,          /* Success */
    IBACKUP_E_INVALID_ARG = -1,     /* Invalid argument provided */
    IBACKUP_E_PLIST_ERROR = -2,     /* Property list parsing/manipulation error */
    IBACKUP_E_DATA_ERROR = -3,      /* Database or data read error */
    IBACKUP_E_UNKNOWN_ERROR = -256  /* Unknown error */
} libibackup_error_t;
```

#### File Type Flags
```c
#define IBACKUP_FLAG_FILE           1  /* Regular file */
#define IBACKUP_FLAG_DIRECTORY      2  /* Directory */
#define IBACKUP_FLAG_SYMBOLIC_LINK  4  /* Symbolic link */
```

#### Client Handle (`libibackup_client_t`)
```c
typedef struct libibackup_client_private *libibackup_client_t;
```
An opaque handle representing an active backup session.

#### Structures

- **`libibackup_file_entry_t`**: Represents a file record in the backup manifest.
  ```c
  typedef struct {
      char* file_id;        /* Unique backup file identifier / SHA-1 hash */
      char* domain;         /* Backup domain name (e.g. CameraRollDomain) */
      char* relative_path;  /* Relative path within the domain */
      char* target;         /* Symlink target path (if symbolic link) */
      uint32_t type;        /* Type flags (IBACKUP_FLAG_FILE, etc.) */
  } libibackup_file_entry_t;
  ```

- **`libibackup_file_metadata_t`**: Parsed file metadata.
  ```c
  typedef struct {
      uint32_t owner;       /* File owner ID */
      uint32_t group;       /* File group ID */
      uint64_t size;        /* File size in bytes */
      char* path;           /* Path string */
      char* target;         /* Symbolic link target */
  } libibackup_file_metadata_t;
  ```

- **`libibackup_domain_metrics_t`**: Aggregated domain metrics.
  ```c
  typedef struct {
      uint32_t file_count;       /* Total regular files in domain */
      uint32_t directory_count;  /* Total directories in domain */
      uint32_t symlink_count;    /* Total symbolic links in domain */
  } libibackup_domain_metrics_t;
  ```

---

### Functions

#### Backup Validation and Session Management

- **`bool libibackup_preflight_backup(const char* path)`**  
  Checks if `path` is a valid backup directory containing required files (`Info.plist`, `Manifest.plist`, `Manifest.db`).  
  *Returns:* `true` if valid, `false` otherwise.

- **`libibackup_error_t libibackup_open_backup(const char* path, libibackup_client_t* client)`**  
  Opens the backup directory at `path` and initializes `*client`.  
  *Returns:* `IBACKUP_E_SUCCESS` on success, or an error code.

- **`libibackup_error_t libibackup_close(libibackup_client_t client)`**  
  Closes the backup session, closes underlying databases, and frees allocated client memory.  
  *Returns:* `IBACKUP_E_SUCCESS`.

#### Backup Information and Domains

- **`libibackup_error_t libibackup_get_info(libibackup_client_t client, plist_t* info)`**  
  Retrieves a copy of the backup's `Info.plist` data into `*info`.  
  *Note:* The caller is responsible for freeing `*info` with `plist_free()`.

- **`libibackup_error_t libibackup_list_domains(libibackup_client_t client, collection_t *domains)`**  
  Populates the `collection_t` structure with dynamically allocated domain names (`char*`).  
  *Note:* Caller is responsible for freeing each string in the collection and freeing the collection.

- **`libibackup_error_t libibackup_get_domain_metrics(libibackup_client_t client, const char* domain, libibackup_domain_metrics_t* metrics)`**  
  Populates `metrics` with counts of files, directories, and symlinks for the given `domain`.

#### Files and Metadata

- **`libibackup_error_t libibackup_list_files_for_domain(libibackup_client_t client, const char* domain, collection_t *files)`**  
  Populates `files` with pointers to dynamically allocated `libibackup_file_entry_t` structures for `domain`.

- **`libibackup_error_t libibackup_get_file_by_id(libibackup_client_t client, const char* file_id, char** full_path)`**  
  Resolves a `file_id` (hash) to its absolute on-disk path in `*full_path`.  
  *Note:* Caller must free `*full_path` using `free()` or `libibackup_free()`.

- **`char* libibackup_get_path_for_file_id(libibackup_client_t client, const char* file_id)`**  
  Resolves a `file_id` to its full on-disk path within the backup folder structure.  
  *Note:* Caller must free the returned string using `free()` or `libibackup_free()`.

- **`libibackup_error_t libibackup_get_raw_metadata_by_id(libibackup_client_t client, const char* file_id, plist_t* metadata)`**  
  Retrieves and parses the raw property list metadata for `file_id` from `Manifest.db`.  
  *Note:* Caller is responsible for freeing `*metadata` with `plist_free()`.

- **`libibackup_error_t libibackup_get_metadata_by_id(libibackup_client_t client, const char* file_id, libibackup_file_metadata_t* metadata)`**  
  Retrieves and parses structured metadata (such as file size and symlink target) for `file_id`.

- **`libibackup_error_t libibackup_remove_file_by_id(libibackup_client_t client, const char* file_id)`**  
  Deletes the file record with ID `file_id` from the `Manifest.db` database.

#### Utility Functions

- **`char* libibackup_combine_path(const char* directory, const char* file)`**  
  Combines directory and file path components with the appropriate system path separator. Caller must free the returned string.

- **`void libibackup_free(void* obj)`**  
  Helper function to free heap memory allocated by the library.

---

## Example Usage

The following example shows how to validate a backup, open a session, list domains, and iterate through files:

```c
#include <stdio.h>
#include <stdlib.h>
#include <libibackup/libibackup.h>

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <backup-path>\n", argv[0]);
        return 1;
    }

    const char *backup_path = argv[1];

    /* 1. Preflight check */
    if (!libibackup_preflight_backup(backup_path)) {
        fprintf(stderr, "Error: Invalid backup path: %s\n", backup_path);
        return 1;
    }

    /* 2. Open backup session */
    libibackup_client_t client = NULL;
    if (libibackup_open_backup(backup_path, &client) != IBACKUP_E_SUCCESS) {
        fprintf(stderr, "Error: Failed to open backup\n");
        return 1;
    }

    /* 3. List domains */
    collection_t *domains = collection_new();
    if (libibackup_list_domains(client, domains) == IBACKUP_E_SUCCESS) {
        int64_t i = 0;
        while (domains->list[i] != NULL) {
            const char *domain = (const char *)domains->list[i];
            libibackup_domain_metrics_t metrics;
            libibackup_get_domain_metrics(client, domain, &metrics);

            printf("Domain: %s (Files: %u, Dirs: %u, Symlinks: %u)\n",
                   domain, metrics.file_count, metrics.directory_count, metrics.symlink_count);

            /* List files in domain */
            collection_t *files = collection_new();
            if (libibackup_list_files_for_domain(client, domain, files) == IBACKUP_E_SUCCESS) {
                int64_t j = 0;
                while (files->list[j] != NULL) {
                    libibackup_file_entry_t *entry = (libibackup_file_entry_t *)files->list[j];
                    printf("  - [%s] %s (ID: %s)\n",
                           entry->type == IBACKUP_FLAG_FILE ? "FILE" :
                           entry->type == IBACKUP_FLAG_DIRECTORY ? "DIR" : "LINK",
                           entry->relative_path, entry->file_id);
                    j++;
                }
            }
            collection_free_all(files);

            free(domains->list[i]);
            i++;
        }
    }
    collection_free(domains);

    /* 4. Close session */
    libibackup_close(client);
    return 0;
}
```

## License

This project is licensed under the terms of the [MIT License](LICENSE).
