#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <sys/time.h>
#include <termios.h>
#include <errno.h>


#include "fs.h"

void fs_format() {
    int fd = open(DISK_FILE, O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (fd == -1) {
        const char* msg = "Disk dosyası oluşturulamadı\n";
        write(STDERR_FILENO, msg, strlen(msg));
        _exit(1);
    }

    // 1 MB'lık boş disk verisi oluştur
    char zero = 0;
    for (int i = 0; i < DISK_SIZE; i++) {
        write(fd, &zero, 1);
    }

    // Başlangıca dosya sayısını sıfır olarak yaz
    lseek(fd, 0, SEEK_SET);
    int file_count = 0;
    write(fd, &file_count, sizeof(int));

    close(fd);

    const char* success = "Dosya sistemi başarıyla formatlandı.\n";
    write(STDOUT_FILENO, success, strlen(success));
}

int fs_create(const char* filename) {
    int fd = open(DISK_PATH, O_RDWR);
    if (fd < 0) {
        const char* err = "Disk açılamadı\n";
        write(STDERR_FILENO, err, strlen(err));
        return -1;
    }

    Metadata meta;
    lseek(fd, 0, SEEK_SET);
    read(fd, &meta, sizeof(Metadata));

    // Aynı isimde dosya var mı
    for (int i = 0; i < MAX_FILES; i++) {
        if (meta.files[i].used && strcmp(meta.files[i].name, filename) == 0) {
            const char* err = "Hata: Aynı isimde dosya zaten var.\n";
            write(STDOUT_FILENO, err, strlen(err));
            close(fd);
            return -1;
        }
    }

    // Boş slot bul
    int slot = -1;
    for (int i = 0; i < MAX_FILES; i++) {
        if (!meta.files[i].used) {
            slot = i;
            break;
        }
    }

    if (slot == -1) {
        const char* err = "Hata: Dosya tablosu dolu.\n";
        write(STDOUT_FILENO, err, strlen(err));
        close(fd);
        return -1;
    }

    // Dosya için başlangıç konumu bul
    int last_end = sizeof(Metadata);
    for (int i = 0; i < MAX_FILES; i++) {
        if (meta.files[i].used) {
            int end = meta.files[i].start + meta.files[i].size;
            if (end > last_end) {
                last_end = end;
            }
        }
    }

    strncpy(meta.files[slot].name, filename, FILENAME_MAX_LENGTH);
    meta.files[slot].size = 0;
    meta.files[slot].start = last_end;
    meta.files[slot].used = 1;
    meta.file_count++;

    lseek(fd, 0, SEEK_SET);
    write(fd, &meta, sizeof(Metadata));
    close(fd);

write_str(STDOUT_FILENO, "Dosya oluşturuldu: ");
write(STDOUT_FILENO, filename, my_strlen(filename));
write_str(STDOUT_FILENO, "\n");
    return 0;
}

int fs_delete(char* filename) {
    int fd = open(DISK_PATH, O_RDWR);
    if (fd < 0) {
        const char* err = "Disk açılamadı\n";
        write(STDERR_FILENO, err, strlen(err));
        return -1;
    }

    Metadata meta;
    lseek(fd, 0, SEEK_SET);
    read(fd, &meta, sizeof(Metadata));

    for (int i = 0; i < MAX_FILES; i++) {
        if (meta.files[i].used && strcmp(meta.files[i].name, filename) == 0) {
            meta.files[i].used = 0;
            meta.file_count--;

            lseek(fd, 0, SEEK_SET);
            write(fd, &meta, sizeof(Metadata));
            close(fd);

            //char msg[128];
            int len = 0;
            len += write(STDOUT_FILENO, "Dosya silindi: ", 15);
            len += write(STDOUT_FILENO, filename, strlen(filename));
            write(STDOUT_FILENO, "\n", 1);
            return 0;
        }
    }

    const char* not_found = "Hata: Dosya bulunamadı.\n";
    write(STDOUT_FILENO, not_found, strlen(not_found));
    close(fd);
    return -1;
}

void fs_ls() {
    int fd = open(DISK_PATH, O_RDONLY);
    if (fd < 0) {
        const char* err = "Disk açılamadı\n";
        write(STDERR_FILENO, err, strlen(err));
        return;
    }

    Metadata meta;
    lseek(fd, 0, SEEK_SET);
    read(fd, &meta, sizeof(Metadata));
    close(fd);

    const char* header = "--- Dosyalar ---\n";
    write(STDOUT_FILENO, header, strlen(header));

    int found = 0;
    for (int i = 0; i < MAX_FILES; i++) {
        if (meta.files[i].used) {
            found = 1;

            //char buffer[256];
            int len = 0;

            len += write(STDOUT_FILENO, "Ad: ", 4);
            len += write(STDOUT_FILENO, meta.files[i].name, strlen(meta.files[i].name));
            len += write(STDOUT_FILENO, " | Boyut: ", 10);

            char size_buf[32];
            int size_len = int_to_str(meta.files[i].size, size_buf);
            write(STDOUT_FILENO, size_buf, size_len);

            write(STDOUT_FILENO, " byte\n", 6);
        }
    }

    if (!found) {
        const char* none = "Hiç dosya yok.\n";
        write(STDOUT_FILENO, none, strlen(none));
    }
}

int fs_write(const char* filename, const char* data) {
    int fd = open(DISK_PATH, O_RDWR);
    if (fd < 0) {
        const char* err = "Disk açılamadı\n";
        write(STDERR_FILENO, err, strlen(err));
        return -1;
    }

    Metadata meta;
    lseek(fd, 0, SEEK_SET);
    read(fd, &meta, sizeof(Metadata));

    int data_len = strlen(data);

    for (int i = 0; i < MAX_FILES; i++) {
        if (meta.files[i].used && strcmp(meta.files[i].name, filename) == 0) {
            if (meta.files[i].start + data_len > DISK_SIZE) {
                const char* err = "Hata: Veri diske sığmıyor.\n";
                write(STDOUT_FILENO, err, strlen(err));
                close(fd);
                return -1;
            }

            lseek(fd, meta.files[i].start, SEEK_SET);
            write(fd, data, data_len);
            meta.files[i].size = data_len;

            lseek(fd, 0, SEEK_SET);
            write(fd, &meta, sizeof(Metadata));
            close(fd);

            write(STDOUT_FILENO, "Yazma başarılı: ", 16);
            write(STDOUT_FILENO, filename, strlen(filename));
            write(STDOUT_FILENO, "\n", 1);
            return 0;
        }
    }

    const char* not_found = "Hata: Dosya bulunamadı.\n";
    write(STDOUT_FILENO, not_found, strlen(not_found));
    close(fd);
    return -1;
}

int fs_exists(const char* filename) {
    int fd = open(DISK_PATH, O_RDONLY);
    if (fd == -1) return 0;

    Metadata meta;
    read(fd, &meta, sizeof(Metadata));
    close(fd);

    for (int i = 0; i < MAX_FILES; i++) {
        if (meta.files[i].used && strcmp(meta.files[i].name, filename) == 0) {
            return 1;
        }
    }

    return 0;
}

int fs_size(const char* filename) {
    int fd = open(DISK_PATH, O_RDONLY);
    if (fd == -1) return -1;

    Metadata meta;
    if (read(fd, &meta, sizeof(Metadata)) != sizeof(Metadata)) {
        close(fd);
        return -1;
    }

    close(fd);

    for (int i = 0; i < MAX_FILES; i++) {
        if (meta.files[i].used) {
            int j = 0;
            while (filename[j] && filename[j] == meta.files[i].name[j]) j++;
            if (filename[j] == '\0' && meta.files[i].name[j] == '\0') {
                return meta.files[i].size;
            }
        }
    }

    return -1; // Dosya yok
}

int fs_read(const char* filename, int offset, int length, char* buffer) {
    int fd = open(DISK_PATH, O_RDONLY);
    if (fd < 0) return -1;

    Metadata meta;
    if (read(fd, &meta, sizeof(Metadata)) != sizeof(Metadata)) {
        close(fd);
        return -1;
    }

    for (int i = 0; i < MAX_FILES; i++) {
        if (meta.files[i].used) {
            int j = 0;
            while (filename[j] && filename[j] == meta.files[i].name[j]) j++;
            if (filename[j] == '\0' && meta.files[i].name[j] == '\0') {
                if (offset + length > meta.files[i].size) {
                    close(fd);
                    return -1;
                }

                lseek(fd, meta.files[i].start + offset, SEEK_SET);
                int bytes_read = read(fd, buffer, length);
                close(fd);

                if (bytes_read != length) return -1;
                return bytes_read;
            }
        }
    }

    close(fd);
    return -1;
}

int my_strcmp(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++; s2++;
    }
    return (unsigned char)*s1 - (unsigned char)*s2;
}

void my_strncpy(char* dest, const char* src, int n) {
    int i;
    for (i = 0; i < n - 1 && src[i]; i++) {
        dest[i] = src[i];
    }
    dest[i] = '\0';
}

int my_strlen(const char* s) {
    int len = 0;
    while (s[len]) len++;
    return len;
}

void write_str(int fd, const char* str) {
    write(fd, str, my_strlen(str));
}

void my_perror(const char* msg) {
    write_str(STDERR_FILENO, msg);
    write_str(STDERR_FILENO, "\n");
}

int fs_append(const char* filename, const char* data, int size) {
    int fd = open("disk/disk.sim", O_RDWR);
    if (fd < 0) {
        my_perror("disk.sim açılamadı");
        return -1;
    }

    Metadata meta;
    if (lseek(fd, 0, SEEK_SET) == -1) {
        my_perror("lseek başarısız");
        close(fd);
        return -1;
    }

    if (read(fd, &meta, sizeof(Metadata)) != sizeof(Metadata)) {
        my_perror("Metadata okunamadı");
        close(fd);
        return -1;
    }

    int i;
    FileEntry *file = NULL;
    for (i = 0; i < meta.file_count; i++) {
        if (meta.files[i].used && my_strcmp(meta.files[i].name, filename) == 0) {
            file = &meta.files[i];
            break;
        }
    }

    if (!file) {
        write_str(STDERR_FILENO, "Dosya bulunamadı: ");
        write_str(STDERR_FILENO, filename);
        write_str(STDERR_FILENO, "\n");
        close(fd);
        return -1;
    }

    off_t write_offset = file->start + file->size;
    if (write_offset + size > DISK_SIZE) {
        write_str(STDERR_FILENO, "Disk dolu, veri eklenemedi.\n");
        close(fd);
        return -1;
    }

    if (lseek(fd, write_offset, SEEK_SET) == -1) {
        my_perror("lseek başarısız");
        close(fd);
        return -1;
    }

    ssize_t written = write(fd, data, size);
    if (written != size) {
        my_perror("Veri yazılamadı");
        close(fd);
        return -1;
    }

    file->size += size;

    if (lseek(fd, 0, SEEK_SET) == -1) {
        my_perror("lseek başarısız");
        close(fd);
        return -1;
    }

    if (write(fd, &meta, sizeof(Metadata)) != sizeof(Metadata)) {
        my_perror("Metadata güncellenemedi");
        close(fd);
        return -1;
    }

    close(fd);
    return 0;
}

int read_metadata(int fd, Metadata* meta) {
    if (lseek(fd, 0, SEEK_SET) == -1) return -1;
    if (read(fd, meta, sizeof(Metadata)) != sizeof(Metadata)) return -1;
    return 0;
}

int write_metadata(int fd, Metadata* meta) {
    if (lseek(fd, 0, SEEK_SET) == -1) return -1;
    if (write(fd, meta, sizeof(Metadata)) != sizeof(Metadata)) return -1;
    return 0;
}

int fs_truncate(const char* filename, int new_size) {
    int fd = open("disk/disk.sim", O_RDWR);
    if (fd < 0) return -1;

    Metadata meta;
    if (read_metadata(fd, &meta) != 0) {
        close(fd);
        return -1;
    }

    for (int i = 0; i < meta.file_count; i++) {
        if (meta.files[i].used && my_strcmp(meta.files[i].name, filename) == 0) {
            if (new_size < 0 || new_size > meta.files[i].size) {
                close(fd);
                return -1;
            }
            meta.files[i].size = new_size;

            if (write_metadata(fd, &meta) != 0) {
                close(fd);
                return -1;
            }
            close(fd);
            return 0;
        }
    }

    close(fd);
    return -1;
}

int fs_cat(const char* filename) {
    int fd = open("disk/disk.sim", O_RDONLY);
    if (fd < 0) {
        my_perror("Disk açılamadı");
        return -1;
    }

    Metadata meta;
    if (read_metadata(fd, &meta) != 0) {
        write_str(STDOUT_FILENO, "Metadata okunamadı.\n");
        close(fd);
        return -1;
    }

    for (int i = 0; i < meta.file_count; i++) {
        if (meta.files[i].used && my_strcmp(meta.files[i].name, filename) == 0) {
            int size = meta.files[i].size;
            int start = meta.files[i].start;

            char* buffer = (char*)malloc(size + 1);
            if (!buffer) {
                write_str(STDOUT_FILENO, "Bellek ayrılamadı.\n");
                close(fd);
                return -1;
            }

            if (lseek(fd, start, SEEK_SET) == -1) {
                write_str(STDOUT_FILENO, "lseek hatası.\n");
                free(buffer);
                close(fd);
                return -1;
            }

            if (read(fd, buffer, size) != size) {
                write_str(STDOUT_FILENO, "read hatası.\n");
                free(buffer);
                close(fd);
                return -1;
            }

            buffer[size] = '\0';
            write_str(STDOUT_FILENO, "Dosya içeriği:\n");
            write(STDOUT_FILENO, buffer, size);
            write_str(STDOUT_FILENO, "\n");

            free(buffer);
            close(fd);
            return 0;
        }
    }

    write_str(STDOUT_FILENO, "Dosya bulunamadı.\n");
    close(fd);
    return -1;
}

int fs_copy(const char* src_filename, const char* dest_filename) {
    if (!fs_exists(src_filename)) {
        write_str(STDERR_FILENO, "Kaynak dosya bulunamadı.\n");
        return -1;
    }

    if (fs_exists(dest_filename)) {
        write_str(STDERR_FILENO, "Hedef dosya zaten mevcut.\n");
        return -1;
    }

    int src_size = fs_size(src_filename);
    if (src_size < 0) {
        write_str(STDERR_FILENO, "Kaynak dosyanın boyutu alınamadı.\n");
        return -1;
    }

    char* buffer = malloc(src_size);
    if (!buffer) {
        write_str(STDERR_FILENO, "Bellek hatası.\n");
        return -1;
    }

    if (fs_read(src_filename, 0, src_size, buffer) < 0) {
        write_str(STDERR_FILENO, "Kaynak dosya okunamadı.\n");
        free(buffer);
        return -1;
    }

    if (fs_create(dest_filename) < 0) {
        write_str(STDERR_FILENO, "Hedef dosya oluşturulamadı.\n");
        free(buffer);
        return -1;
    }

    if (fs_write(dest_filename, buffer) < 0) {
        write_str(STDERR_FILENO, "Hedef dosyaya yazılamadı.\n");
        free(buffer);
        return -1;
    }

    free(buffer);
    return 0;
}


int fs_diff(const char* file1, const char* file2) {
    if (!fs_exists(file1) || !fs_exists(file2)) {
        write_str(STDERR_FILENO, "Dosyalardan biri mevcut değil.\n");
        return -1;
    }

    int size1 = fs_size(file1);
    int size2 = fs_size(file2);

    if (size1 != size2) {
        return 1;
    }

    char* buffer1 = (char*)malloc(size1);
    char* buffer2 = (char*)malloc(size2);

    if (!buffer1 || !buffer2) {
        write_str(STDERR_FILENO, "Bellek hatası.\n");
        if (buffer1) free(buffer1);
        if (buffer2) free(buffer2);
        return -1;
    }

    if (fs_read(file1, 0, size1, buffer1) < 0 || fs_read(file2, 0, size2, buffer2) < 0) {
        write_str(STDERR_FILENO, "Dosya okuma hatası.\n");
        free(buffer1);
        free(buffer2);
        return -1;
    }

    int diff = 0;
    for (int i = 0; i < size1; i++) {
        if (buffer1[i] != buffer2[i]) {
            diff = 1;
            break;
        }
    }

    free(buffer1);
    free(buffer2);

    return diff;
}

int fs_rename(const char* old_name, const char* new_name) {
    int fd = open(DISK_PATH, O_RDWR);
    if (fd < 0) {
        write_str(STDERR_FILENO, "Disk açılamadı\n");
        return -1;
    }

    Metadata meta;
    if (read_metadata(fd, &meta) != 0) {
        close(fd);
        return -1;
    }

    for (int i = 0; i < meta.file_count; i++) {
        if (meta.files[i].used && my_strcmp(meta.files[i].name, new_name) == 0) {
            write_str(STDERR_FILENO, "Hata: '");
            write(fd, new_name, my_strlen(new_name));
            write_str(STDERR_FILENO, "' isminde zaten bir dosya var.\n");
            close(fd);
            return -1;
        }
    }

    for (int i = 0; i < meta.file_count; i++) {
        if (meta.files[i].used && my_strcmp(meta.files[i].name, old_name) == 0) {
            my_strncpy(meta.files[i].name, new_name, FILENAME_MAX_LENGTH);

            if (write_metadata(fd, &meta) != 0) {
                close(fd);
                return -1;
            }

            close(fd);

            write_str(STDOUT_FILENO, "Dosya adı '");
            write(STDOUT_FILENO, new_name, my_strlen(new_name));
            write_str(STDOUT_FILENO, "' olarak değiştirildi.\n");

            return 0;
        }
    }

    write_str(STDERR_FILENO, "Hata: '");
    write(STDERR_FILENO, old_name, my_strlen(old_name));
    write_str(STDERR_FILENO, "' isminde bir dosya bulunamadı.\n");

    close(fd);
    return -1;
}


int fs_defragment() {
    int fd = open(DISK_PATH, O_RDWR);
    if (fd < 0) {
        const char* err = "Disk açılamadı\n";
        write(STDOUT_FILENO, err, strlen(err));
        return -1;
    }

    Metadata meta;
    if (read_metadata(fd, &meta) != 0) {
        close(fd);
        return -1;
    }

    int current_offset = 0;
    char temp[BLOCK_SIZE]; // Geçici buffer

    for (int i = 0; i < meta.file_count; i++) {
        if (meta.files[i].used) {
            if (meta.files[i].start != current_offset) {
                int remaining = meta.files[i].size;
                int old_offset = meta.files[i].start;

                while (remaining > 0) {
                    int to_read = remaining < BLOCK_SIZE ? remaining : BLOCK_SIZE;
                    lseek(fd, METADATA_SIZE + old_offset, SEEK_SET);
                    read(fd, temp, to_read);

                    lseek(fd, METADATA_SIZE + current_offset, SEEK_SET);
                    write(fd, temp, to_read);

                    remaining -= to_read;
                    old_offset += to_read;
                    current_offset += to_read;
                }

                meta.files[i].start = current_offset - meta.files[i].size;
            } else {
                current_offset += meta.files[i].size;
            }
        }
    }

    write_metadata(fd, &meta);
    close(fd);

    const char* msg = "Disk başarıyla defragment edildi.\n";
    write(STDOUT_FILENO, msg, strlen(msg));
    return 0;
}

int fs_check_integrity() {
    int fd = open(DISK_PATH, O_RDONLY);
    if (fd < 0) {
        const char* err = "Disk açılamadı\n";
        write(STDOUT_FILENO, err, strlen(err));
        return -1;
    }

    Metadata meta;
    if (read_metadata(fd, &meta) != 0) {
        close(fd);
        return -1;
    }

    int integrity_ok = 1;
    for (int i = 0; i < meta.file_count; i++) {
        if (meta.files[i].used) {
			if (meta.files[i].start + meta.files[i].size > DISK_SIZE - METADATA_SIZE) {
				write_str(STDOUT_FILENO, "Hatalı dosya: ");
				write(STDOUT_FILENO, meta.files[i].name, my_strlen(meta.files[i].name));
				write_str(STDOUT_FILENO, " (veri diskin dışına taşıyor)\n");
				integrity_ok = 0;
			}
        }
    }

    close(fd);

    if (integrity_ok) {
        const char* msg = "Tüm dosyalar tutarlı.\n";
        write(STDOUT_FILENO, msg, strlen(msg));
        return 0;
    } else {
        return -1;
    }
}

int fs_backup(const char* backup_filename) {
    int src_fd = open(DISK_PATH, O_RDONLY);
    if (src_fd < 0) {
        const char* err = "Yedek alınacak disk açılamadı\n";
        write(STDOUT_FILENO, err, strlen(err));
        return -1;
    }

    int dest_fd = open(backup_filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (dest_fd < 0) {
        const char* err = "Yedek dosyası oluşturulamadı\n";
        write(STDOUT_FILENO, err, strlen(err));
        close(src_fd);
        return -1;
    }

    char buffer[4096];
    ssize_t bytes_read;
    while ((bytes_read = read(src_fd, buffer, sizeof(buffer))) > 0) {
        if (write(dest_fd, buffer, bytes_read) != bytes_read) {
            const char* err = "Yedekleme sırasında yazma hatası\n";
            write(STDOUT_FILENO, err, strlen(err));
            close(src_fd);
            close(dest_fd);
            return -1;
        }
    }

    close(src_fd);
    close(dest_fd);
    const char* msg = "SYedekleme başarılı\n";
    write(STDOUT_FILENO, msg, strlen(msg));
    return 0;
}


int fs_restore(const char* backup_filename) {
    int backup_fd = open(backup_filename, O_RDONLY);
    if (backup_fd < 0) {
        const char* msg = "Yedek dosyası açılamadı\n";
        write(STDOUT_FILENO, msg, strlen(msg));
        return -1;
    }

    int disk_fd = open(DISK_PATH, O_WRONLY | O_TRUNC);
    if (disk_fd < 0) {
        const char* msg = "Disk dosyası açılamadı veya oluşturulamadı\n";
        write(STDOUT_FILENO, msg, strlen(msg));
        close(backup_fd);
        return -1;
    }

    char buffer[4096];
    ssize_t bytes_read;
    while ((bytes_read = read(backup_fd, buffer, sizeof(buffer))) > 0) {
        if (write(disk_fd, buffer, bytes_read) != bytes_read) {
            const char* msg = "Geri yükleme sırasında yazma hatası\n";
            write(STDOUT_FILENO, msg, strlen(msg));
            close(backup_fd);
            close(disk_fd);
            return -1;
        }
    }

    close(backup_fd);
    close(disk_fd);

    const char* msg = "Geri yükleme başarılı\n";
    write(STDOUT_FILENO, msg, strlen(msg));
    return 0;
}

void fs_log(const char* message) {
    int fd = open("disk/fs.log", O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd < 0) return;

    struct timeval tv;
    gettimeofday(&tv, NULL); // Unix epoch time in seconds + microseconds

    //char entry[512];
    int len = 0;

    len += write(fd, "[timestamp: ", 12);

    char timestamp[32];
    int tlen = 0;

    long sec = tv.tv_sec;
    int i = 30;
    timestamp[i--] = '\0';
    do {
        timestamp[i--] = '0' + (sec % 10);
        sec /= 10;
    } while (sec > 0);
    tlen = 30 - i;

    write(fd, &timestamp[i + 1], tlen);
    write(fd, "] ", 2);
    write(fd, message, strlen(message));
    write(fd, "\n", 1);

    close(fd);
}


int fs_mv(const char* old_name, const char* new_name) {
    // Aslında rename işlemidir
    return fs_rename(old_name, new_name);
}

int int_to_str(int value, char* buffer) {
    int i = 0;
    if (value == 0) {
        buffer[i++] = '0';
        buffer[i] = '\0';
        return i;
    }

    int is_negative = 0;
    if (value < 0) {
        is_negative = 1;
        value = -value;
    }

    char temp[20];
    while (value > 0) {
        temp[i++] = (value % 10) + '0';
        value /= 10;
    }

    int j = 0;
    if (is_negative) {
        buffer[j++] = '-';
    }

    while (i > 0) {
        buffer[j++] = temp[--i];
    }

    buffer[j] = '\0';
    return j;
}

void clear_console() {
    const char* cmd = "\033[2J\033[H";
    write(STDOUT_FILENO, cmd, strlen(cmd)); 
}
void press_any_key() {
    struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt); 
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);       // Canonical mode ve echo kapat
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    const char* msg = "Devam etmek için bir tuşa basın...\n";
    write(STDOUT_FILENO, msg, strlen(msg));

    char c;
    read(STDIN_FILENO, &c, 1);             

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt); // Ayarları geri yükle
}
