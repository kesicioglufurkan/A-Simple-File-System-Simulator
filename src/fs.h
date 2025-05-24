#ifndef FS_H
#define FS_H

#define DISK_FILE "disk/disk.sim"

#define MAX_FILES 128
#define METADATA_SIZE 4096            // 4 KB
#define FILENAME_MAX_LENGTH 32

#define DISK_PATH "disk/disk.sim"
#define DISK_SIZE (1024 * 1024) // 1 MB

#define BLOCK_SIZE 512

typedef struct {
    char name[FILENAME_MAX_LENGTH];
    int size;
    int start;
    int used;
} FileEntry;

typedef struct {
    int file_count;
    FileEntry files[MAX_FILES];
} Metadata;

// Fonksiyon prototipleri
void fs_format();
int fs_create(const char* filename);
int fs_delete(char* filename);
void fs_ls();
int fs_write(const char* filename, const char* data);
int fs_exists(const char* filename);
int fs_size(const char* filename);
int fs_read(const char* filename, int offset, int length, char* buffer);
int fs_append(const char* filename, const char* data, int size);
int fs_truncate(const char* filename, int new_size);
int fs_cat(const char* filename);
int fs_copy(const char* src_filename, const char* dest_filename);
int fs_diff(const char* file1, const char* file2);
int fs_rename(const char* old_name, const char* new_name);
int fs_defragment();
int fs_check_integrity();
int fs_backup(const char* backup_filename);
int fs_restore(const char* backup_filename);
void fs_log(const char* message);
int fs_mv(const char* old_name, const char* new_name);
int int_to_str(int value, char* buffer) ;
void my_perror(const char* msg);
int my_strcmp(const char* s1, const char* s2);
void my_strncpy(char* dest, const char* src, int n);
int my_strlen(const char* s);
void write_str(int fd, const char* str);
void my_perror(const char* msg) ;
void clear_console();
void press_any_key() ;

#endif
