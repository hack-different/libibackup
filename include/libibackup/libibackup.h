#ifndef LIBIBACKUP_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Error Codes */
typedef enum {
    IBACKUP_E_SUCCESS = 0,
    IBACKUP_E_INVALID_ARG = -1,
    IBACKUP_E_PLIST_ERROR = -2,
    IBACKUP_E_UNKNOWN_ERROR = -256
} libibackup_error_t;

typedef struct libibackup_client_private libibackup_client_private;
typedef libibackup_client_private *libibackup_client_t; /**< The client handle. */

bool libibackup_preflight_backup(const char* path);

libibackup_error_t libibackup_open_backup(const char* path, libibackup_client_t* client);

libibackup_error_t libibackup_close(libibackup_client_t client);


#ifdef __cplusplus
}
#endif

#define LIBIBACKUP_H
#endif