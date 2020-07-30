#include <libibackup/libibackup.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif


void list_domains(libibackup_client_t client) {
    printf("Listing Domains\n");
    char **domain_list;
    libibackup_list_domains(client, &domain_list);

    int64_t index = 0;
    while (domain_list[index] != NULL) {
        printf("Domain: %s\n", domain_list[index]);

        free(domain_list[index]);
        index++;
    }

    free(domain_list);
}

void list_files(libibackup_client_t client, char* domain) {
    printf("Listing files for domain %s\n", domain);
    libibackup_file_entry_t **file_list;

    libibackup_list_files_for_domain(client, domain, &file_list);

    int64_t index = 0;
    while (file_list[index] != NULL) {
        printf("File: %s\n", file_list[index]->relative_path);
        index++;
    }

    free(file_list);
}

int main(int argc, char **argv) {
    libibackup_set_debug(true);
    if (argc < 3) {
        printf("Invalid Arguments\n");
        return -1;
    }

    libibackup_client_t client;

    libibackup_open_backup(argv[2], &client);

    printf("Backup Opened\n");


    if (strcmp(argv[1], "list_domains") == 0) {
        list_domains(client);
    }
    if (strcmp(argv[1], "list_files") == 0) {
        list_files(client, argv[3]);
    }

    libibackup_close(client);

    return 0;
}

#ifdef __cplusplus
}
#endif