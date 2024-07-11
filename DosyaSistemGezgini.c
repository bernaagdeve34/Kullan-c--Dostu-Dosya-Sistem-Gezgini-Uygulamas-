#include <stdio.h>

#include <stdlib.h>

#include <unistd.h>

#include <fcntl.h>

#include <errno.h>

#include <string.h>

#include <sys/stat.h>

#include <sys/types.h>

#include <dirent.h>

#include <time.h>

#include <fnmatch.h>





#define MAX_FILE_COUNT 10

#define MAX_FILE_NAME_LENGTH 100



int dosya_ac(char *dosya_adi) {

    int dosya_fd = open(dosya_adi, O_RDONLY);

    if (dosya_fd == -1) {

        perror("Dosya açma hatasý");

        return -1;

    }

    return dosya_fd;

}



int dosya_yaz(char *dosya_adi, char *icerik) {

    int dosya_fd = open(dosya_adi, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);

    if (dosya_fd == -1) {

        perror("Dosya yazma hatasý");

        return -1;

    }

    ssize_t yazilan_byte_sayisi = write(dosya_fd, icerik, strlen(icerik));

    if (yazilan_byte_sayisi == -1) {

        perror("Dosyaya yazma hatasý");

        close(dosya_fd);

        return -1;

    }

    close(dosya_fd);

    return 0;

}



int dosya_kopyala(char *kaynak_dosya, char *hedef_dosya) {

    int kaynak_fd = dosya_ac(kaynak_dosya);

    if (kaynak_fd == -1) {

        printf("Kopyalama iþlemi baþarýsýz. Kaynak dosya açýlamadý.\n");

        return -1;

    }

    

    int hedef_fd = open(hedef_dosya, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);

    if (hedef_fd == -1) {

        perror("Hedef dosya açma hatasý");

        close(kaynak_fd);

        return -1;

    }

    

    char buf[1024];

    ssize_t okunan_byte_sayisi;

    while ((okunan_byte_sayisi = read(kaynak_fd, buf, sizeof(buf))) > 0) {

        if (write(hedef_fd, buf, okunan_byte_sayisi) != okunan_byte_sayisi) {

            perror("Hedef dosyaya yazma hatasý");

            close(kaynak_fd);

            close(hedef_fd);

            return -1;

        }

    }

    

    if (okunan_byte_sayisi == -1) {

        perror("Kaynak dosyadan okuma hatasý");

        close(kaynak_fd);

        close(hedef_fd);

        return -1;

    }

    

    close(kaynak_fd);

    close(hedef_fd);

    return 0;

}



int dosya_tasi(char *kaynak_dosya, char *hedef_dosya) {

    int kopyalama_sonuc = dosya_kopyala(kaynak_dosya, hedef_dosya);

    if (kopyalama_sonuc == -1) {

        printf("Dosya taþýma iþlemi baþarýsýz.\n");

        return -1;

    }

    

    if (remove(kaynak_dosya) == -1) {

        perror("Kaynak dosya silme hatasý");

        return -1;

    }

    

    return 0;

}





int dosya_sil(char *dosya_adi) {

    if (remove(dosya_adi) == -1) {

        perror("Dosya silme hatasý");

        return -1;

    }

    printf("Dosya baþarýyla silindi.\n");

    return 0;

}



int dosya_duzenle(char *dosya_adi) {

    char *editor = getenv("EDITOR"); // Sistem editörünü alýyoruz

    if (editor == NULL) {

        editor = "vi"; // Varsayýlan olarak vi kullan diyoruz

    }

    char komut[256];

    sprintf(komut, "%s %s", editor, dosya_adi); 

    if (system(komut) == -1) { // Editörü çalýþtýrýyoruz burda

        perror("Editör baþlatma hatasý");

        return -1;

    }

    return 0;

}



int dosya_turu_degistir(char *dosya_adi, char *yeni_tur) {

    char eski_adi[256];

    strcpy(eski_adi, dosya_adi);

    char *nokta = strrchr(eski_adi, '.'); 

    if (nokta == NULL) {

        printf("Dosya uzantýsý bulunamadý.\n");

        return -1;

    }

    strcpy(nokta + 1, yeni_tur); 

    if (rename(dosya_adi, eski_adi) == -1) { 

        perror("Dosya türünü deðiþtirme hatasý");

        return -1;

    }

    return 0;

}



int dosya_izinleri_ayarla(char *dosya_adi, char *izinler) {

    mode_t izin;

    if (strcmp(izinler, "rwx") == 0) {

        izin = S_IRWXU;

    } else if (strcmp(izinler, "rw-") == 0) {

        izin = S_IRUSR | S_IWUSR;

    } else if (strcmp(izinler, "r-x") == 0) {

        izin = S_IRUSR | S_IXUSR;

    } else if (strcmp(izinler, "r--") == 0) {

        izin = S_IRUSR;

    } else {

        printf("Geçersiz izinler!\n");

        return -1;

    }



    if (chmod(dosya_adi, izin) == -1) {

        perror("Dosya izinlerini ayarlama hatasý");

        return -1;

    }



    printf("Dosya izinleri baþarýyla güncellendi.\n");

    return 0;

}



void dosya_sec(char dosya_adlari[][MAX_FILE_NAME_LENGTH], int *dosya_sayisi) {

    int i = 0;

    char dosya_adi[MAX_FILE_NAME_LENGTH];



    printf("Ýþlem yapmak istediðiniz dosya adlarýný giriniz (bitirmek için 'q' tuþuna basýn):\n");



    while (1) {

        printf("Dosya adý: ");

        scanf("%s", dosya_adi);



        if (strcmp(dosya_adi, "q") == 0 || i >= MAX_FILE_COUNT) {

            break;

        }



        strcpy(dosya_adlari[i], dosya_adi);

        i++;

    }



    *dosya_sayisi = i;

}



int dizin_olustur(char *dizin_adi) {

    // mkdir fonksiyonu ile yeni bir dizin oluþturuyorum

    if (mkdir(dizin_adi, 0777) == -1) {

        perror("Dizin oluþturma hatasý");

        return -1; 

    } else {

        printf("Dizin baþarýyla oluþturuldu.\n");

        return 0; 

    }

}



int dizin_sil(char *dizin_adi) {

   

    DIR *dizin = opendir(dizin_adi);

    struct dirent *dosya;



    

    if (!dizin) {

        perror("Dizin açma hatasý");

        return -1;

    }



    

    while ((dosya = readdir(dizin)) != NULL) {

        if (dosya->d_type == DT_DIR) {

            if (strcmp(dosya->d_name, ".") != 0 && strcmp(dosya->d_name, "..") != 0) {

                char yol[PATH_MAX];

                snprintf(yol, sizeof(yol), "%s/%s", dizin_adi, dosya->d_name);

                dizin_sil(yol);

            }

        } else {

           

            char dosya_yolu[PATH_MAX];

            snprintf(dosya_yolu, sizeof(dosya_yolu), "%s/%s", dizin_adi, dosya->d_name);

            if (unlink(dosya_yolu) == -1) {

                perror("Dosya silme hatasý");

                closedir(dizin);

                return -1;

            }

        }

    }



   

    closedir(dizin);



    // Dizini siliniyor

    if (rmdir(dizin_adi) == -1) {

        perror("Dizin silme hatasý");

        return -1;

    }



    printf("Dizin baþarýyla silindi.\n");

    return 0;

}



int dizin_degistir(char *yeni_dizin) {

    if (chdir(yeni_dizin) == -1) { 

        perror("Dizin deðiþtirme hatasý");

        return -1;

    }

    printf("Geçerli dizin baþarýyla degistirildi.\n");

    return 0;

}





void alt_dizinleri_listele(char *dizin_adi) {

    DIR *dizin;

    struct dirent *dosya;

    struct stat dosya_bilgisi;

    char dosya_yol[256];



    dizin = opendir(dizin_adi);

    if (dizin == NULL) {

        perror("Dizin açma hatasý");

        return;

    }



    printf("Dosya/Dizin Adý\t\t\t\t\t\tTür\t\tBoyut\t\tDeðiþtirilme Tarihi\n");

    printf("=================================================================================\n");



    while ((dosya = readdir(dizin)) != NULL) {

        sprintf(dosya_yol, "%s/%s", dizin_adi, dosya->d_name);

        if (stat(dosya_yol, &dosya_bilgisi) == -1) {

            perror("Dosya bilgisi alma hatasý");

            closedir(dizin);

            return;

        }

        printf("%-40s\t", dosya->d_name);

        if (S_ISDIR(dosya_bilgisi.st_mode)) {

            printf("Dizin\t");

        } else {

            printf("Dosya\t");

        }

        printf("%ld\t\t", dosya_bilgisi.st_size);

        printf("%s", ctime(&dosya_bilgisi.st_mtime));

    }



    closedir(dizin);

}



int dizin_kopyala(char *kaynak_dizin, char *hedef_dizin) {

    

    DIR *kaynak = opendir(kaynak_dizin);

    if (!kaynak) {

        perror("Kaynak dizin açma hatasý");

        return -1;

    }



    

    if (mkdir(hedef_dizin, 0777) == -1) {

        perror("Hedef dizin oluþturma hatasý");

        closedir(kaynak);

        return -1;

    }



    // Kaynak dizinindeki dosyalarý hedef dizinine kopyalalýyoruz

    struct dirent *dosya;

    while ((dosya = readdir(kaynak)) != NULL) {

        if (strcmp(dosya->d_name, ".") != 0 && strcmp(dosya->d_name, "..") != 0) {

            char kaynak_yol[PATH_MAX];

            char hedef_yol[PATH_MAX];

            snprintf(kaynak_yol, sizeof(kaynak_yol), "%s/%s", kaynak_dizin, dosya->d_name);

            snprintf(hedef_yol, sizeof(hedef_yol), "%s/%s", hedef_dizin, dosya->d_name);

            // Eðer bir alt dizinse

            if (dosya->d_type == DT_DIR) {

                // Yeni bir kopya oluþtur

                if (dizin_kopyala(kaynak_yol, hedef_yol) == -1) {

                    closedir(kaynak);

                    return -1;

                }

            } else {

               

                if (dosya_kopyala(kaynak_yol, hedef_yol) == -1) {

                    closedir(kaynak);

                    return -1;

                }

            }

        }

    }



    

    closedir(kaynak);

    printf("Dizin baþarýyla kopyalandý.\n");

    return 0;

}





int dizin_tasi(char *kaynak_dizin, char *hedef_dizin) {

    // Kaynak dizinini hedef dizinine kopyala

    if (dizin_kopyala(kaynak_dizin, hedef_dizin) == -1) {

        printf("Dizin taþýma baþarýsýz.\n");

        return -1;

    }



    // Kaynak dizinini sil

    if (dizin_sil(kaynak_dizin) == -1) {

        printf("Kaynak dizin silinemedi.\n");

        return -1;

    }



    printf("Dizin baþarýyla taþýndý.\n");

    return 0;

}







int dosya_ara(char *aranacak_metin) {

    DIR *dizin;

    struct dirent *dosya;

    struct stat dosya_bilgisi;

    char dosya_yol[256];



    dizin = opendir(".");

    if (dizin == NULL) {

        perror("Dizin açma hatasý");

        return -1;

    }

   

    printf("Aranan metni içeren dosyalar:\n");

    printf("================================\n");



    while ((dosya = readdir(dizin)) != NULL) {

        sprintf(dosya_yol, "%s/%s", ".", dosya->d_name);

        if (stat(dosya_yol, &dosya_bilgisi) == -1) {

            perror("Dosya bilgisi alma hatasý");

            closedir(dizin);

            return -1;

        }

        if (S_ISREG(dosya_bilgisi.st_mode)) {

            FILE *dosya_ptr = fopen(dosya_yol, "r");

            if (dosya_ptr == NULL) {

                perror("Dosya açma hatasý");

                continue;

            }

            char satir[512];

            while (fgets(satir, sizeof(satir), dosya_ptr) != NULL) {

                if (strstr(satir, aranacak_metin) != NULL) {

                    printf("%s\n", dosya->d_name);

                    break;

                }

            }

            fclose(dosya_ptr);

        }

    }



    closedir(dizin);

    return 0;

}





void dosya_filtrele(char *filtre_kriteri) {

    DIR *dizin;

    struct dirent *dosya;

    struct stat dosya_bilgisi;

    char dosya_yol[256];



    dizin = opendir(".");

    if (dizin == NULL) {

        perror("Dizin açma hatasý");

        return;

    }



    printf("Filtrelenmiþ dosyalar:\n");

    printf("=======================\n");



    while ((dosya = readdir(dizin)) != NULL) {

        sprintf(dosya_yol, "%s/%s", ".", dosya->d_name);

        if (stat(dosya_yol, &dosya_bilgisi) == -1) {

            perror("Dosya bilgisi alma hatasý");

            closedir(dizin);

            return;

        }

        if (fnmatch(filtre_kriteri, dosya->d_name, FNM_PATHNAME) == 0) {

            printf("%s\n", dosya->d_name);

        }

    }



    closedir(dizin);

}





int karsilastir(const void *a, const void *b) {

    const char *dosya1 = *(const char **)a;

    const char *dosya2 = *(const char **)b;

    return strcmp(dosya1, dosya2);

}



void dosya_siralama(char *siralama_kriteri) {



    DIR *dizin;

    struct dirent *dosya;

    char *dosyalar[100]; 



    dizin = opendir(".");

    if (dizin) {

        int dosya_sayisi = 0;

        while ((dosya = readdir(dizin)) != NULL) {

            if (dosya->d_type == DT_REG) { // Sadece dosyalarý al

                dosyalar[dosya_sayisi++] = strdup(dosya->d_name);

            }

        }

        closedir(dizin);

        qsort(dosyalar, dosya_sayisi, sizeof(char *), karsilastir);



     

        printf("Sýralanmýþ dosyalar:\n");

        for (int i = 0; i < dosya_sayisi; i++) {

            printf("%s\n", dosyalar[i]);

            free(dosyalar[i]); 

        }

    } else {

        printf("Mevcut dizin açýlamadý!\n");

    }

}



int main() {

    int secim;

     while (1) {

    printf("\n\nDosya Sistem Gezgini Uygulamasýna Hoþgeldiniz\n\n");

    printf("Lütfen yapmak istediðiniz iþlem için ilgili tuþa basýnýz!\n\n");

    printf("*Dosya Ýþlemleri için 1'e\n");

    printf("*Dizin Ýþlemleri için 2'e\n");

    printf("*Arama ve Filtreleme iþlemleri için 3'e\n");

    printf("*Çýkýþ için 4'e basýnýz\n\n");



    printf("Seçiminizi yapýnýz: ");

    scanf("%d", &secim);



    switch (secim) {

        case 1:

            printf("\n\nDosya Ýþlemleri menüsüne giriþ yapýldý.\n");

            printf("Dosya iþlemleri için ilgili tuþa basýnýz!\n");

            printf("a->Dosya Ac\n");

            printf("y->Dosya Yaz\n");

            printf("k->Dosya Kopyala\n");

            printf("t->Dosya Tasý\n");

            printf("s->Dosya Sil\n");

            printf("d->Dosya Duzenle\n");

            printf("f->Dosya Turu Degistir\n");

            printf("i->Dosya Ýzinleri Ayarla\n");

            printf("b->Dosya Sec\n");

            printf("r->Ana Menü'ye Dön\n\n");



            char dosya_secim;

            printf("Dosya iþlemi seçiminizi yapýnýz: ");

            scanf(" %c", &dosya_secim);



            switch (dosya_secim) {

                case 'a':

                    char dosya_adi[100];

                        printf("Açmak istediðiniz dosyanýn adýný giriniz: ");

                        scanf("%s", dosya_adi);

                        int dosya_fd = dosya_ac(dosya_adi);

                        if (dosya_fd != -1) {

                            printf("dosya acildi");

                        }

                    break;

                case 'y':

                      char dosya_adi_yaz[100];

                        printf("Yazmak istediðiniz dosyanýn adýný giriniz: ");

                        scanf("%s", dosya_adi_yaz);

                        char icerik[1000];

                        printf("Dosyaya yazýlacak içeriði giriniz: ");

                        scanf("%s", icerik);

                        int yazma_sonuc = dosya_yaz(dosya_adi_yaz, icerik);

                        if (yazma_sonuc == 0) {

                            printf("Dosya baþarýyla yazýldý.\n");

                        }

                    break;

                case 'k':

                     char kaynak_dosya[100];

                        printf("Kopyalanacak dosyanýn adýný giriniz: ");

                        scanf("%s", kaynak_dosya);

                        char hedef_dosya[100];

                        printf("Hedef dosyanýn adýný giriniz: ");

                        scanf("%s", hedef_dosya);

                        int kopyalama_sonuc = dosya_kopyala(kaynak_dosya, hedef_dosya);

                        if (kopyalama_sonuc == 0) {

                            printf("Dosya baþarýyla kopyalandý.\n");

                        }

                    break;

                case 't':

                    char tasi_kaynak_dosya[100];

                        printf("Taþýnacak dosyanýn adýný giriniz: ");

                        scanf("%s", tasi_kaynak_dosya);

                        char tasi_hedef_dosya[100];

                        printf("Hedef dosyanýn adýný giriniz: ");

                        scanf("%s", tasi_hedef_dosya);

                        int tasi_sonuc = dosya_tasi(tasi_kaynak_dosya, tasi_hedef_dosya);

                        if (tasi_sonuc == 0) {

                            printf("Dosya baþarýyla taþýndý.\n");

                        }

                    break;

                case 's':

                     char silinecek_dosya[100];

                     printf("Silinecek dosyanýn adýný giriniz: ");

                     scanf("%s", silinecek_dosya);

                     int silme_sonuc = dosya_sil(silinecek_dosya);

                    if (silme_sonuc == 0) {

                    printf("Dosya baþarýyla silindi.\n");

        }

                    break;

                case 'd':

                    char duzenlenecek_dosya[100];

                      printf("Düzenlemek istediðiniz dosyanýn adýný giriniz: ");

                    scanf("%s", duzenlenecek_dosya);

                    int duzenleme_sonuc = dosya_duzenle(duzenlenecek_dosya);

                 if (duzenleme_sonuc == 0) {

                printf("Dosya baþarýyla düzenlendi.\n");

                  }

                    break;

                case 'f':

               printf("Türünü deðiþtirmek istediðiniz dosyanýn adýný giriniz: ");

               scanf("%s", dosya_adi); 

               char yeni_tur[10];

                printf("Yeni türü giriniz (örn: txt): ");

               scanf("%s", yeni_tur);

               int tur_degistirme_sonuc = dosya_turu_degistir(dosya_adi, yeni_tur);

               if (tur_degistirme_sonuc == 0) {

               printf("Dosyanýn türü baþarýyla deðiþtirildi.\n");

                 }   break;

                case 'i':

                    char izinler[4]; // izinlerimiz rwx, rw-, r-x, r-- olabileceðini varsayalým

                    printf("Dosya izinlerini ayarlamak için izinler dizisini giriniz (örn: rwx): ");

                    scanf("%s", izinler);

                     int izin_ayarla_sonuc = dosya_izinleri_ayarla(dosya_adi, izinler);

                    if (izin_ayarla_sonuc == 0) {

                   printf("Dosya izinleri baþarýyla güncellendi.\n");

                    }

                    break;

                case 'b':

                      char dosya_adlari[MAX_FILE_COUNT][MAX_FILE_NAME_LENGTH];

                      int dosya_sayisi = 0;

                      dosya_sec(dosya_adlari, &dosya_sayisi);



                      printf("Seçilen dosyalar:\n");

                    for (int i = 0; i < dosya_sayisi; i++) {

                   printf("%s\n", dosya_adlari[i]);

                    }

                    break;

                case 'r':

                        printf("Ana Menü'ye dönülüyor...\n");

                        continue; // Döngünün baþýna git

                default:

                    printf("Geçersiz seçim!\n");

            }

            break;

        case 2:

            printf("\n\nDizin Ýþlemleri menüsüne giriþ yapýldý.\n");

             printf("Dizin iþlemi için ilgili tuþa basýnýz!\n");

            printf("o->Dizin Oluþtur\n");

            printf("s->Dizin Sil\n");

            printf("d->Dizin Deðiþtir\n");

            printf("l->Alt Dizinleri Listele\n");

            printf("k->Dizin Kopyala\n");

            printf("t->Dizin Taþý\n");

             printf("r->Ana Menü'ye Dön\n\n");



            char dizin_secim;

            printf("Dizin iþlemi seçiminizi yapýnýz: ");

            scanf(" %c", &dizin_secim);



            switch (dizin_secim) {

                case 'o':

                    char dizin_adi[100];

                    printf("Oluþturulacak dizinin adýný giriniz: ");

                    scanf("%s", dizin_adi);



                   if (dizin_olustur(dizin_adi) == -1) {

                   printf("Dizin oluþturma baþarýsýz.\n");

                       }

                    break;

                case 's':

                  printf("Silinecek dizinin adýný giriniz: ");

                  scanf("%s", dizin_adi);



                  if (dizin_sil(dizin_adi) == -1) {

                  printf("Dizin silme baþarýsýz.\n");

                    }

                    break;

                case 'd':

                   char yeni_dizin[100];

                        printf("Geçmek istediðiniz dizinin yolunu giriniz: ");

                        scanf("%s", yeni_dizin);

                        int degistirme_sonuc = dizin_degistir(yeni_dizin);

                        if (degistirme_sonuc == 0) {

                            printf("Dizin baþarýyla degistirildi.\n");

                        }

                    break;

                case 'l':

                 char secilen_dizin[100];

                        printf("Listelemek istediðiniz dizinin adini giriniz: ");

                       

                        scanf("%s", secilen_dizin);

                        alt_dizinleri_listele(secilen_dizin);

                        break;

                    break;

                case 'k':

                    char kaynak_dizin[100];

                    char hedef_dizin[100];

                    printf("Kopyalanacak dizinin adýný giriniz: ");

                    scanf("%s", kaynak_dizin);

                    printf("Hedef dizinin adýný giriniz: ");

                    scanf("%s", hedef_dizin);

                    if (dizin_kopyala(kaynak_dizin, hedef_dizin) == -1) {

                    printf("Dizin kopyalama baþarýsýz.\n");

                    }

                    break;

                case 't':

                    printf("Taþýnacak dizinin adýný giriniz: ");

                    scanf("%s", kaynak_dizin);

                    printf("Hedef dizinin adýný giriniz: ");

                    scanf("%s", hedef_dizin);

                     if (dizin_tasi(kaynak_dizin, hedef_dizin) == -1) {

                     printf("Dizin taþýma baþarýsýz.\n");

                       }

                    break;

                case 'r':

                        printf("Ana Menü'ye dönülüyor...\n");

                        continue; 

                default:

                    printf("Geçersiz seçim!\n");

            }

            break;

        case 3:

            printf("\n\nArama ve Filtreleme iþlemleri menüsüne giriþ yapýldý.\n");

            printf("yapmak istediðiniz iþlem için ilgili tuþa basýnýz!\n");

            printf("a->Dosya Ara\n");

            printf("f->Dosya Filtrele\n");

            printf("s->Dosya Sýralama\n");

             printf("r->Ana Menü'ye Dön\n\n");



            char arama_secim;

            printf("Arama ve filtreleme iþlemi seçiminizi yapýnýz:\n ");

            scanf(" %c", &arama_secim);



            switch (arama_secim) {

                case 'a':

                    char aranacak_metin[100];

                    printf("Aranacak metni giriniz: \n");

                   scanf("%s", aranacak_metin);

                   printf("\n",aranacak_metin,"\n");

                  if (dosya_ara(aranacak_metin) == -1) {

                  printf("Dosya arama baþarýsýz.\n");

                   }

                    break;

                case 'f':

                    char filtre_kriteri[100];

                    printf("Filtre kriterini giriniz: \n");

                    scanf("%s", filtre_kriteri);

                    dosya_filtrele(filtre_kriteri);

                    break;

                case 's':

                    printf("Dosya Sýralama iþlemi seçildi.\n");



                     char siralama_kriteri[10];

                     printf("Sýralama kriterini girin (örn. ad): \n");

                     scanf("%s", siralama_kriteri);



                    dosya_siralama(siralama_kriteri);

                    break;

                case 'r':

                        printf("Ana Menü'ye dönülüyor...\n");

                        continue;   

                default:

                    printf("Geçersiz seçim!\n");

            }

            break;

        case 4:

            printf("Program sonlandýrýlýyor...\n");

                exit(0);

            break;

        default:

            printf("Geçersiz seçim!\n");

    }

}

    return 0;

}


