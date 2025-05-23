
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "fs.h"

#define STDIN 0
#define STDOUT 1


// Kullanıcıdan satır okur (enter'a kadar), \n yerine \0 koyar
void read_line(char* buffer, int max_len) {
    int i = 0;
    char ch;
    while (i < max_len - 1 && read(STDIN, &ch, 1) == 1 && ch != '\n') {
        buffer[i++] = ch;
    }
    buffer[i] = '\0';
}

// Basit sayı okuma (int için)
int read_int() {
    char buf[16];
    read_line(buf, sizeof(buf));
    return atoi(buf); // Eğer bu da sistem çağrısız isteniyorsa, basit int parse yazılabilir
}

void print_menu() {
    write_str(STDOUT, "\n--- SimpleFS Menü v3 ---\n");
    write_str(STDOUT, "1. Dosya oluştur\n");
    write_str(STDOUT, "2. Dosya sil\n");
    write_str(STDOUT, "3. ls\n");
    write_str(STDOUT, "4. write\n");
    write_str(STDOUT, "5. exit\n");

    write_str(STDOUT, "7. Dosya var mı?\n");
    write_str(STDOUT, "8. Dosya boyutunu öğren\n");
    write_str(STDOUT, "9. append\n");
    write_str(STDOUT, "10. trun\n");
    write_str(STDOUT, "11. cat\n");
    write_str(STDOUT, "12. copy\n");
    write_str(STDOUT, "13. diff\n");
    write_str(STDOUT, "14. rename\n");
    write_str(STDOUT, "15. frag\n");
    write_str(STDOUT, "16. check\n");
    write_str(STDOUT, "17. backup\n");
    write_str(STDOUT, "18. restore\n");
    write_str(STDOUT, "Seciminizi girin: ");
}

int main() {
    int secim;
    char dosya_adi[FILENAME_MAX_LENGTH];
    char veri[1024];
	char hedef_adi[128];

    fs_format();

    while (1) {
		clear_console();
        print_menu();
        secim = read_int();

        switch (secim) {
            case 1:
                write_str(STDOUT, "Dosya adı: ");
                read_line(dosya_adi, sizeof(dosya_adi));
                fs_create(dosya_adi);
                break;

            case 2:
                write_str(STDOUT, "Silinecek dosya adı: ");
                read_line(dosya_adi, sizeof(dosya_adi));
                fs_delete(dosya_adi);
                break;

            case 3:
                // clear screen komutu yok, yerine boş satır atabilirsin
                write_str(STDOUT, "\n\n\n\n\n");
                fs_ls();
                break;

            case 4:
                write_str(STDOUT, "Dosya adı: ");
                read_line(dosya_adi, sizeof(dosya_adi));
                write_str(STDOUT, "Yazılacak veri: ");
                read_line(veri, sizeof(veri));
                fs_write(dosya_adi, veri);
                break;

            case 5:
                write_str(STDOUT, "Cikiliyor...\n");
                _exit(0); // sadece sistem çağrısı olan çıkış
                break;

            case 7:
                write_str(STDOUT, "Kontrol edilecek dosya adı: ");
                read_line(dosya_adi, sizeof(dosya_adi));
                if (fs_exists(dosya_adi)) {
                    write_str(STDOUT, "Dosya mevcut.\n");
                } else {
                    write_str(STDOUT, "Dosya bulunamadi.\n");
                }
                break;
				
					case 8: {
    write_str(STDOUT, "Boyutu ogrenilecek dosya adi: ");
    read_line(dosya_adi, sizeof(dosya_adi));
    int boyut = fs_size(dosya_adi);

    if (boyut >= 0) {
        char msg[64] = "Dosya boyutu: ";
        char sayi[16];
        int_to_str(boyut, sayi);
        strcat(msg, sayi);
        strcat(msg, " bayt\n");
        write_str(STDOUT, msg);
    } else {
        write_str(STDOUT, "Dosya bulunamadi.\n");
    }
    break;
}


case 9: {
    write_str(STDOUT, "Veri eklenecek dosya adi: ");
    read_line(dosya_adi, sizeof(dosya_adi));

    write_str(STDOUT, "Eklenecek veri: ");
    read_line(veri, sizeof(veri));

    if (fs_append(dosya_adi, veri, strlen(veri)) == 0) {
        write_str(STDOUT, "Veri basariyla dosyanin sonuna eklendi.\n");
    } else {
        write_str(STDOUT, "Hata: Veri eklenemedi.\n");
    }
    break;
}

case 10: {
    write_str(STDOUT, "Kesilecek dosya adi: ");
    read_line(dosya_adi, sizeof(dosya_adi));

    write_str(STDOUT, "Yeni boyut: ");
    int new_size = read_int();

    if (fs_truncate(dosya_adi, new_size) == 0) {
        write_str(STDOUT, "Dosya basariyla kesildi.\n");
    } else {
        write_str(STDOUT, "Hata: Dosya kesilemedi.\n");
    }
    break;
}


case 11: {
    write_str(STDOUT, "Icerigi gosterilecek dosya adi: ");
    read_line(dosya_adi, sizeof(dosya_adi));

    if (fs_cat(dosya_adi) != 0) {
        write_str(STDOUT, "Hata: Dosya okunamadi.\n");
    }
    break;
}


case 12: {
    write_str(STDOUT, "Kopyalanacak kaynak dosya adi: ");
    read_line(dosya_adi, sizeof(dosya_adi));

    write_str(STDOUT, "Hedef dosya adi: ");
    read_line(hedef_adi, sizeof(hedef_adi));

    if (fs_copy(dosya_adi, hedef_adi) == 0) {
        write_str(STDOUT, "Dosya basariyla kopyalandi.\n");
    } else {
        write_str(STDOUT, "Dosya kopyalama hatasi.\n");
    }
    break;
}


case 13: {
    write_str(STDOUT, "Karsilastirilacak birinci dosya adi: ");
    read_line(dosya_adi, sizeof(dosya_adi));

    write_str(STDOUT, "Karsilastirilacak ikinci dosya adi: ");
    read_line(hedef_adi, sizeof(hedef_adi));

    int diff = fs_diff(dosya_adi, hedef_adi);
    if (diff == 0)
        write_str(STDOUT, "Dosyalar ayni.\n");
    else if (diff == 1)
        write_str(STDOUT, "Dosyalar farkli.\n");
    else
        write_str(STDOUT, "Dosya karsilastirma sirasinda hata olustu.\n");
    break;
}


case 14: {
    write_str(STDOUT, "Ismi degistirilecek dosya adi: ");
    read_line(dosya_adi, sizeof(dosya_adi));

    write_str(STDOUT, "Yeni dosya adi: ");
    read_line(hedef_adi, sizeof(hedef_adi));

    int result = fs_rename(dosya_adi, hedef_adi);
    if (result == 0)
        write_str(STDOUT, "Basarili.\n");
    else if (result == -1)
        write_str(STDOUT, "HATA!\n");
    else
        write_str(STDOUT, "Dosya isim degistirme sirasinda hata olustu.\n");
    break;
}


case 15: {
    int defrag = fs_defragment();
    if (defrag == 0)
        write_str(STDOUT, "Basarili.\n");
    else
        write_str(STDOUT, "Dosya fragmantasyonu sirasinda hata olustu.\n");
    break;
}


case 16: {
    int check = fs_check_integrity();
    if (check == 0)
        write_str(STDOUT, "Basarili.\n");
    else
        write_str(STDOUT, "Dosya kontrolu sirasinda hata olustu.\n");
    break;
}


case 17: {
    write_str(STDOUT, "Yedek dosya adi (orn: disk/backup.sim): ");
    read_line(dosya_adi, sizeof(dosya_adi));

    if (fs_backup(dosya_adi) == 0)
        write_str(STDOUT, "Yedekleme basarili.\n");
    else
        write_str(STDOUT, "Hata: Yedekleme basarisiz.\n");
    break;
}


case 18: {
    write_str(STDOUT, "Yuklenecek yedek dosya adi (orn: disk/backup.sim): ");
    read_line(dosya_adi, sizeof(dosya_adi));

    if (fs_restore(dosya_adi) == 0)
        write_str(STDOUT, "Geri yukleme basarili.\n");
    else
        write_str(STDOUT, "Hata: Geri yukleme basarisiz.\n");
    break;
}

case 19: {
    int fd = open("disk/fs.log", O_RDONLY);
    if (fd < 0) {
        write_str(STDOUT, "Log dosyasi acilamadi.\n");
        break;
    }

    char buffer[512];
    ssize_t bytes_read;
    while ((bytes_read = read(fd, buffer, sizeof(buffer))) > 0) {
        write(STDOUT, buffer, bytes_read);
    }

    close(fd);
    break;
}





         default:
    write_str(STDOUT, "Gecersiz secim.\n");
    system("clear");

        }
    }

    return 0;
}