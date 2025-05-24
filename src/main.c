
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "fs.h"

#define STDIN 0
#define STDOUT 1


void read_line(char* buffer, int max_len) {
    int i = 0;
    char ch;
    while (i < max_len - 1 && read(STDIN, &ch, 1) == 1 && ch != '\n') {
        buffer[i++] = ch;
    }
    buffer[i] = '\0';
}

int read_int() {
    char buf[16];
    read_line(buf, sizeof(buf));
    return atoi(buf);
}


void print_menu() {
    write_str(STDOUT, "\n===== SimpleFS Menü v3 =====\n\n");

    write_str(STDOUT, "--- Temel Dosya İşlemleri ---\n");
    write_str(STDOUT, " 1. Dosya oluştur                     -> (fs_create)\n");
    write_str(STDOUT, " 2. Dosya sil                         -> (fs_delete)\n");
    write_str(STDOUT, " 3. Dosya adını değiştir              -> (fs_rename)\n");

    write_str(STDOUT, "\n--- Dosya Okuma/Yazma ---\n");
    write_str(STDOUT, " 4. Dosyaya veri ekle                 -> (fs_write)\n");
    write_str(STDOUT, " 5. Dosyanın sonuna veri ekle        -> (fs_append)\n");
    write_str(STDOUT, " 6. Dosya truncate                    -> (fs_truncate)\n");
    write_str(STDOUT, " 7. Dosya içeriğini yazdır            -> (fs_cat)\n");

    write_str(STDOUT, "\n--- Dosya Bilgisi ve Sorgu ---\n");
    write_str(STDOUT, " 8. Dosya var mı?                     -> (fs_exists)\n");
    write_str(STDOUT, " 9. Dosya boyutu                      -> (fs_size)\n");
    write_str(STDOUT, "10. Dosyaları listele                 -> (fs_ls)\n");

    write_str(STDOUT, "\n--- Kopyalama ve Karşılaştırma ---\n");
    write_str(STDOUT, "11. Dosya kopyala                     -> (fs_copy)\n");
    write_str(STDOUT, "12. Dosyaları karşılaştır             -> (fs_diff)\n");

    write_str(STDOUT, "\n--- Sistem ve Bakım İşlemleri ---\n");
    write_str(STDOUT, "13. Dosya parçalanma gider            -> (fs_defragment)\n");
    write_str(STDOUT, "14. Sistem bütünlüğünü kontrol et     -> (fs_check_integrity)\n");

    write_str(STDOUT, "\n--- Yedekleme ve Geri Yükleme ---\n");
    write_str(STDOUT, "15. Disk yedeğini al                  -> (fs_backup)\n");
    write_str(STDOUT, "16. Yedek yükle                       -> (fs_restore)\n");
    write_str(STDOUT, "17. İşlem logunu görüntüle            -> (fs_log)\n");
    write_str(STDOUT, "18. Format at                         -> (fs_format)\n");

    write_str(STDOUT, "\n19. Çıkış                             -> Uygulamadan çıkış yapar\n");

    write_str(STDOUT, "\nLütfen bir seçenek giriniz: ");
}


int main() {
	
    int secim;
    char dosya_adi[FILENAME_MAX_LENGTH];
    char veri[1024];
	char hedef_adi[128];
	clear_console();
	
	write_str(STDOUT, "Lütfen tam ekranda kullanınız.\n");

    fs_format();

    while (1) {
		
		clear_console();	
        print_menu();
        secim = read_int();

        switch (secim) {
			
			
            case 1:
                write_str(STDOUT, "\nOluşturulacak dosya adı: ");
                read_line(dosya_adi, sizeof(dosya_adi));
                fs_create(dosya_adi);
				press_any_key();
                break;

            case 2:
                write_str(STDOUT, "\nSilinecek dosya adı: ");
                read_line(dosya_adi, sizeof(dosya_adi));
                fs_delete(dosya_adi);
				press_any_key();
                break;			
				
			case 3: 
				write_str(STDOUT, "\nIsmi degistirilecek dosya adi: ");
				read_line(dosya_adi, sizeof(dosya_adi));

				write_str(STDOUT, "\nYeni dosya adi: ");
				read_line(hedef_adi, sizeof(hedef_adi));

				int result = fs_rename(dosya_adi, hedef_adi);
				
				if (result == 0)
					write_str(STDOUT, "Basarili.\n");
				else if (result == -1)
				write_str(STDOUT, "HATA!\n");
				else
				write_str(STDOUT, "Dosya isim degistirme sirasinda hata olustu.\n");
				press_any_key();
				break;		

            case 4:
                write_str(STDOUT, "Dosya adı: ");
                read_line(dosya_adi, sizeof(dosya_adi));
                write_str(STDOUT, "Yazılacak veri: ");
                read_line(veri, sizeof(veri));
                fs_write(dosya_adi, veri);
				press_any_key();
                break;						
			
			case 5: 
				write_str(STDOUT, "Veri eklenecek dosya adi: ");
				read_line(dosya_adi, sizeof(dosya_adi));

				write_str(STDOUT, "Eklenecek veri: ");
				read_line(veri, sizeof(veri));

				if (fs_append(dosya_adi, veri, strlen(veri)) == 0) {
					write_str(STDOUT, "Veri basariyla dosyanin sonuna eklendi.\n");
				} else {
					write_str(STDOUT, "Hata: Veri eklenemedi.\n");
				}
				press_any_key();
				break;
			
			case 6: 
				write_str(STDOUT, "Kesilecek dosya adi: ");
				read_line(dosya_adi, sizeof(dosya_adi));

				write_str(STDOUT, "Yeni boyut: ");
				int new_size = read_int();

				if (fs_truncate(dosya_adi, new_size) == 0) {
					write_str(STDOUT, "Dosya basariyla kesildi.\n");
				} else {
				write_str(STDOUT, "Hata: Dosya kesilemedi.\n");
				}
				press_any_key();
				break;				
				
			case 7: 
				write_str(STDOUT, "Icerigi gosterilecek dosya adi: ");
				read_line(dosya_adi, sizeof(dosya_adi));

				if (fs_cat(dosya_adi) != 0) {
					write_str(STDOUT, "Hata: Dosya okunamadi.\n");
				}
				press_any_key();
				break;

            case 8:
                write_str(STDOUT, "Kontrol edilecek dosya adı: ");
                read_line(dosya_adi, sizeof(dosya_adi));
                if (fs_exists(dosya_adi)) {
                    write_str(STDOUT, "Dosya mevcut.\n");
                } else {
                    write_str(STDOUT, "Dosya bulunamadi.\n");
                }
				press_any_key();
                break;
				
			case 9: 
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
				press_any_key();
				break;

            case 10:
                write_str(STDOUT, "\n\n\n\n\n");
                fs_ls();
				press_any_key();
                break;

			case 11: 
				write_str(STDOUT, "Kopyalanacak kaynak dosya adi: ");
				read_line(dosya_adi, sizeof(dosya_adi));	

				write_str(STDOUT, "Hedef dosya adi: ");
				read_line(hedef_adi, sizeof(hedef_adi));

				if (fs_copy(dosya_adi, hedef_adi) == 0) {
					write_str(STDOUT, "Dosya basariyla kopyalandi.\n");
				} else {
					write_str(STDOUT, "Dosya kopyalama hatasi.\n");
				}
				press_any_key();
				break;

			case 12: 
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
				press_any_key();
				break;

			case 13: 
				int defrag = fs_defragment();
				
				if (defrag == 0)
					write_str(STDOUT, "Basarili.\n");
				else
					write_str(STDOUT, "Dosya fragmantasyonu sirasinda hata olustu.\n");
				press_any_key();
				break;

			case 14: 
				int check = fs_check_integrity();
				
				if (check == 0)
					write_str(STDOUT, "Basarili.\n");
				else
					write_str(STDOUT, "Dosya kontrolu sirasinda hata olustu.\n");
				press_any_key();
				break;

			case 15: 
				write_str(STDOUT, "Yedek dosya adi (orn: disk/backup.sim): ");
				read_line(dosya_adi, sizeof(dosya_adi));

				if (fs_backup(dosya_adi) == 0)
					write_str(STDOUT, "Yedekleme basarili.\n");
				else
					write_str(STDOUT, "Hata: Yedekleme basarisiz.\n");
				press_any_key();
				break;

			case 16: 
				write_str(STDOUT, "Yuklenecek yedek dosya adi (orn: disk/backup.sim): ");
				read_line(dosya_adi, sizeof(dosya_adi));

				if (fs_restore(dosya_adi) == 0)
					write_str(STDOUT, "Geri yukleme basarili.\n");
				else
					write_str(STDOUT, "Hata: Geri yukleme basarisiz.\n");
				press_any_key();
				break;

			case 17: 
				int fd = open("disk/fs.log", O_RDONLY);
				
				if (fd < 0) {
					write_str(STDOUT, "Log dosyasi acilamadi.\n");
					press_any_key();
				break;
				}

				char buffer[512];
				ssize_t bytes_read;
				while ((bytes_read = read(fd, buffer, sizeof(buffer))) > 0) {
				write(STDOUT, buffer, bytes_read);
				}

				close(fd);
				press_any_key();
				break;
				
			case 18:
                write_str(STDOUT, "Format atiliyor...\n");
				fs_format();
				press_any_key();
                break;

            case 19:
                write_str(STDOUT, "Cikiliyor...\n");
                _exit(0);
				press_any_key();
                break;
			
				default:
				write_str(STDOUT, "Gecersiz secim.\n");
				system("clear");
        }
    }

    return 0;
}