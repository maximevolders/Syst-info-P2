#include "lib_tar.h"

/**
 * Checks whether the archive is valid.
 *
 * Each non-null header of a valid archive has:
 *  - a magic value of "ustar" and a null,
 *  - a version value of "00" and no null,
 *  - a correct checksum
 *
 * @param tar_fd A file descriptor pointing to the start of a file supposed to contain a tar archive.
 *
 * @return a zero or positive value if the archive is valid, representing the number of headers in the archive,
 *         -1 if the archive contains a header with an invalid magic value,
 *         -2 if the archive contains a header with an invalid version value,
 *         -3 if the archive contains a header with an invalid checksum value
 */
int check_archive(int tar_fd) {
	tar_header_t file;
	read(tar_fd, &file, 512);
	int nbr_headers=0;
	while(file.name[0] != '\0'){
		if((strcmp((const char*) file.magic, (const char*) TMAGIC) != 0) || (strlen(file.magic)+1 != TMAGLEN)){
			return -1;
		}
		if(file.version[0] != '0' || file.version[1] != '0'){
			return -2;
		}
		if(TAR_INT(file.chksum) != count(&file)){
			return -3;
		}
		nbr_headers++;
		int taille = TAR_INT(file.size);
		for(int i = 0; i < taille; i+=512){
			read(tar_fd, &file, 512);
		}
		read(tar_fd, &file, 512);
	}
	lseek(tar_fd, 0, SEEK_SET);
    return nbr_headers;
}

/**
 * Checks whether an entry exists in the archive.
 *
 * @param tar_fd A file descriptor pointing to the start of a tar archive file.
 * @param path A path to an entry in the archive.
 *
 * @return zero if no entry at the given path exists in the archive,
 *         any other value otherwise.
 */
int exists(int tar_fd, char *path) {
	tar_header_t file;
	read(tar_fd, &file, 512);
	while(file.name[0] != '\0'){
		if(strcmp((const char*) file.name, (const char*) path) == 0){
			lseek(tar_fd, 0, SEEK_SET);
			return 1;
		}
		
		int taille = TAR_INT(file.size);
		for(int i = 0; i < taille; i+=512){
			read(tar_fd, &file, 512);
		}
		read(tar_fd, &file, 512);
	}
	lseek(tar_fd, 0, SEEK_SET);
    return 0;
}

/**
 * Checks whether an entry exists in the archive and is a directory.
 *
 * @param tar_fd A file descriptor pointing to the start of a tar archive file.
 * @param path A path to an entry in the archive.
 *
 * @return zero if no entry at the given path exists in the archive or the entry is not a directory,
 *         any other value otherwise.
 */

int is_dir(int tar_fd, char *path) {
	tar_header_t file;
	read(tar_fd, &file, 512);
	
	while(file.name[0] != '\0'){
		if(strcmp((const char*) file.name, (const char*) path) == 0 && file.typeflag == DIRTYPE){
			lseek(tar_fd, 0, SEEK_SET);
			return 1;
		}
		
		int taille = TAR_INT(file.size);
		for(int i = 0; i < taille; i+=512){
			read(tar_fd, &file, 512);
		}
		read(tar_fd, &file, 512);
	}
	lseek(tar_fd, 0, SEEK_SET);
    return 0;
}

/**
 * Checks whether an entry exists in the archive and is a file.
 *
 * @param tar_fd A file descriptor pointing to the start of a tar archive file.
 * @param path A path to an entry in the archive.
 *
 * @return zero if no entry at the given path exists in the archive or the entry is not a file,
 *         any other value otherwise.
 */

int is_file(int tar_fd, char *path) {
	tar_header_t file;
	read(tar_fd, &file, 512);
	while(file.name[0] != '\0'){
		if(strcmp((const char*) file.name, (const char*) path) == 0 && (file.typeflag == REGTYPE || file.typeflag == AREGTYPE)){
			lseek(tar_fd, 0, SEEK_SET);
			return 1;
		}
		
		int taille = TAR_INT(file.size);
		for(int i = 0; i < taille; i+=512){
			read(tar_fd, &file, 512);
		}
		read(tar_fd, &file, 512);
	}
	lseek(tar_fd,0 ,SEEK_SET);
    return 0;
}

/**
 * Checks whether an entry exists in the archive and is a symlink.
 *
 * @param tar_fd A file descriptor pointing to the start of a tar archive file.
 * @param path A path to an entry in the archive.
 * @return zero if no entry at the given path exists in the archive or the entry is not symlink,
 *         any other value otherwise.
 */
int is_symlink(int tar_fd, char *path) {
	tar_header_t file;
	read(tar_fd, &file, 512);
	while(file.name[0] != '\0'){
		if(strcmp((const char*) file.name, (const char*) path) == 0 && (file.typeflag == LNKTYPE || file.typeflag == SYMTYPE)){
			lseek(tar_fd, 0, SEEK_SET);
			return 1;
		}
		
		int taille = TAR_INT(file.size);
		for(int i = 0; i < taille; i+=512){
			read(tar_fd, &file, 512);
		}
		read(tar_fd, &file, 512);
	}
	lseek(tar_fd, 0, SEEK_SET);
    return 0;
}


/**
 * Lists the entries at a given path in the archive.
 *
 * @param tar_fd A file descriptor pointing to the start of a tar archive file.
 * @param path A path to an entry in the archive. If the entry is a symlink, it must be resolved to its linked-to entry.
 * @param entries An array of char arrays, each one is long enough to contain a tar entry
 * @param no_entries An in-out argument.
 *                   The caller set it to the number of entry in entries.
 *                   The callee set it to the number of entry listed.
 *
 * @return zero if no directory at the given path exists in the archive,
 *         any other value otherwise.
 */
int list(int tar_fd, char *path, char **entries, size_t *no_entries) {
	if(is_symlink(tar_fd, path)){
		tar_header_t file;
		read(tar_fd, &file, 512);
		while(strcmp((const char*) file.name, (const char*) path) != 0){
			int taille = TAR_INT(file.size);
			for(int i = 0; i < taille; i+=512){
				read(tar_fd, &file, 512);
			}
			read(tar_fd, &file, 512);
		}
		lseek(tar_fd, 0, SEEK_SET);
		return list(tar_fd, file.linkname, entries, no_entries);
	}
	
	if(is_dir(tar_fd, path)){
		tar_header_t file;
		read(tar_fd, &file, 512);
		int nbr_entr = 0;
		while(file.name[0] != '\0'){
			char* chemin = (char*) malloc(sizeof(char)*100);
			memcpy(chemin, file.name, strlen(path));
			if(strcmp(path, chemin) == 0 && strcmp(path, file.name) != 0){
				strcpy(entries[nbr_entr++], file.name);
			}
			free(chemin);
			
			int taille = TAR_INT(file.size);
			for(int i = 0; i < taille; i+=512){
				read(tar_fd, &file, 512);
			}
			read(tar_fd, &file, 512);
		}
		lseek(tar_fd, 0, SEEK_SET);
		*no_entries = nbr_entr;
		return 1;
	}
	lseek(tar_fd, 0, SEEK_SET);
	*no_entries = 0;
    return 0;
}

/**
 * Reads a file at a given path in the archive.
 *
 * @param tar_fd A file descriptor pointing to the start of a tar archive file.
 * @param path A path to an entry in the archive to read from.  If the entry is a symlink, it must be resolved to its linked-to entry.
 * @param offset An offset in the file from which to start reading from, zero indicates the start of the file.
 * @param dest A destination buffer to read the given file into.
 * @param len An in-out argument.
 *            The caller set it to the size of dest.
 *            The callee set it to the number of bytes written to dest.
 *
 * @return -1 if no entry at the given path exists in the archive or the entry is not a file,
 *         -2 if the offset is outside the file total length,
 *         zero if the file was read in its entirety into the destination buffer,
 *         a positive value if the file was partially read, representing the remaining bytes left to be read.
 *
 */
ssize_t read_file(int tar_fd, char *path, size_t offset, uint8_t *dest, size_t *len) {
	if(is_symlink(tar_fd, path)){
		tar_header_t link;
		read(tar_fd, &link, 512);
		while(strcmp((const char*) link.name, (const char*) path) != 0){
			int taille = TAR_INT(link.size);
			for(int i = 0; i < taille; i+=512){
				read(tar_fd, &link, 512);
			}
			read(tar_fd, &link, 512);
		}
		lseek(tar_fd, 0, SEEK_SET);
		return read_file(tar_fd, link.linkname, offset, dest, len);
	}
	
	if(!is_file(tar_fd, path)){
		*len = 0;
		return -1;
	}
	
	tar_header_t file;
	read(tar_fd, &file, 512);
	
	while(strcmp((const char*) file.name, (const char*) path) != 0){
		int taille = TAR_INT(file.size);
		for(int i = 0; i < taille; i+=512){
			read(tar_fd, &file, 512);
		}
		read(tar_fd, &file, 512);
	}
	int taille = TAR_INT(file.size);

	if(offset > taille){
		*len = 0;
		lseek(tar_fd, 0, SEEK_SET);
		return -2;
	}
	
	if(*len > taille){
		*len = taille;
	}
	
	read(tar_fd, dest, offset);
	*len = read(tar_fd, dest, *len);
	
    lseek(tar_fd, 0, SEEK_SET);
	if(*len > taille - offset){
		return 0; 
	}
	return (taille - offset -*len);
}


/**
 * Additional functions
 *
 */

/**
 * Calculates the sum of all bytes in the header bloc.
 *
 * @param file A tar header.
 *
 * @return the sum of all bytes in the header bloc.
 *
 */
int count(tar_header_t* file) {
	int sum = 0;
	unsigned char* byte = (unsigned char*) file;
	for(int i=0; i<512; i++){
		if(i<148 || i>155){ // doesn't count the bytes in chksum[]
			sum+=byte[i];
		}
		else{ // adds the chksum[] bytes as if they were equal to " "
			sum+=040;
		}
	}
	return sum;
}