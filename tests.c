#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "lib_tar.h"

/**
 * You are free to use this file to write tests for your implementation
 */

void debug_dump(const uint8_t *bytes, size_t len) {
    for (int i = 0; i < len;) {
        printf("%04x:  ", (int) i);

        for (int j = 0; j < 16 && i + j < len; j++) {
            printf("%02x ", bytes[i + j]);
        }
        printf("\t");
        for (int j = 0; j < 16 && i < len; j++, i++) {
            printf("%c ", bytes[i]);
        }
        printf("\n");
    }
}

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Usage: %s tar_file\n", argv[0]);
        return -1;
    }

    int fd = open(argv[1] , O_RDONLY);
    if (fd == -1) {
        perror("open(tar_file)");
        return -1;
    }
	
	char *path = "tet/tete/der"; //faire un test avec path < newpath (dans list)
	
    int check = check_archive(fd);
    printf("check_archive returned %d\n\n", check);
	
	int ext = exists(fd, path);
    printf("exists returned %d\n\n", ext);
	
	int dir = is_dir(fd, path);
    printf("is_dir returned %d\n\n", dir);
	
	int file = is_file(fd, path);
    printf("is_file returned %d\n\n", file);
	
	int link = is_symlink(fd, path);
    printf("is_symlink returned %d\n\n", link);
	
	char** entries = (char **) malloc(sizeof(char*)*10);
	size_t no_entries = check;
	for(int j=0; j<check; j++){
		entries[j] = (char*) malloc(sizeof(char)*100);
	}

	int liste = list(fd, path, entries, &no_entries);
	for(int i= 0; i<no_entries; i++){
		printf("entries %d: %s\n", i, entries[i]);
	}
	printf("list returned %d\n\n", liste);
	free(entries);
    return 0;
}