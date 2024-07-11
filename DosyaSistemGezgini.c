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

        perror("Dosya a�ma hatas�");

        return -1;

    }

    return dosya_fd;

}



int dosya_yaz(char *dosya_adi, char *icerik) {

    int dosya_fd = open(dosya_adi, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);

    if (dosya_fd == -1) {

        perror("Dosya yazma hatas�");

        return -1;

    }

    ssize_t yazilan_byte_sayisi = write(dosya_fd, icerik, strlen(icerik));

    if (yazilan_byte_sayisi == -1) {

        perror("Dosyaya yazma hatas�");

        close(dosya_fd);

        return -1;

    }

    close(dosya_fd);

    return 0;

}



int dosya_kopyala(char *kaynak_dosya, char *hedef_dosya) {

    int kaynak_fd = dosya_ac(kaynak_dosya);

    if (kaynak_fd == -1) {

        printf("Kopyalama i�lemi ba�ar�s�z. Kaynak dosya a��lamad�.\n");

        return -1;

    }

    

    int hedef_fd = open(hedef_dosya, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);

    if (hedef_fd == -1) {

        perror("Hedef dosya a�ma hatas�");

        close(kaynak_fd);

        return -1;

    }

    

    char buf[1024];

    ssize_t okunan_byte_sayisi;

    while ((okunan_byte_sayisi = read(kaynak_fd, buf, sizeof(buf))) > 0) {

        if (write(hedef_fd, buf, okunan_byte_sayisi) != okunan_byte_sayisi) {

            perror("Hedef dosyaya yazma hatas�");

            close(kaynak_fd);

            close(hedef_fd);

            return -1;

        }

    }

    

    if (okunan_byte_sayisi == -1) {

        perror("Kaynak dosyadan okuma hatas�");

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

        printf("Dosya ta��ma i�lemi ba�ar�s�z.\n");

        return -1;

    }

    

    if (remove(kaynak_dosya) == -1) {

        perror("Kaynak dosya silme hatas�");

        return -1;

    }

    

    return 0;

}





int dosya_sil(char *dosya_adi) {

    if (remove(dosya_adi) == -1) {

        perror("Dosya silme hatas�");

        return -1;

    }

    printf("Dosya ba�ar�yla silindi.\n");

    return 0;

}



int dosya_duzenle(char *dosya_adi) {

    char *editor = getenv("EDITOR"); // Sistem edit�r�n� al�yoruz

    if (editor == NULL) {

        editor = "vi"; // Varsay�lan olarak vi kullan diyoruz

    }

    char komut[256];

    sprintf(komut, "%s %s", editor, dosya_adi); 

    if (system(komut) == -1) { // Edit�r� �al��t�r�yoruz burda

        perror("Edit�r ba�latma hatas�");

        return -1;

    }

    return 0;

}



int dosya_turu_degistir(char *dosya_adi, char *yeni_tur) {

    char eski_adi[256];

    strcpy(eski_adi, dosya_adi);

    char *nokta = strrchr(eski_adi, '.'); 

    if (nokta == NULL) {

        printf("Dosya uzant�s� bulunamad�.\n");

        return -1;

    }

    strcpy(nokta + 1, yeni_tur); 

    if (rename(dosya_adi, eski_adi) == -1) { 

        perror("Dosya t�r�n� de�i�tirme hatas�");

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

        printf("Ge�ersiz izinler!\n");

        return -1;

    }



    if (chmod(dosya_adi, izin) == -1) {

        perror("Dosya izinlerini ayarlama hatas�");

        return -1;

    }



    printf("Dosya izinleri ba�ar�yla g�ncellendi.\n");

    return 0;

}



void dosya_sec(char dosya_adlari[][MAX_FILE_NAME_LENGTH], int *dosya_sayisi) {

    int i = 0;

    char dosya_adi[MAX_FILE_NAME_LENGTH];



    printf("��lem yapmak istedi�iniz dosya adlar�n� giriniz (bitirmek i�in 'q' tu�una bas�n):\n");



    while (1) {

        printf("Dosya ad�: ");

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

    // mkdir fonksiyonu ile yeni bir dizin olu�turuyorum

    if (mkdir(dizin_adi, 0777) == -1) {

        perror("Dizin olu�turma hatas�");

        return -1; 

    } else {

        printf("Dizin ba�ar�yla olu�turuldu.\n");

        return 0; 

    }

}



int dizin_sil(char *dizin_adi) {

   

    DIR *dizin = opendir(dizin_adi);

    struct dirent *dosya;



    

    if (!dizin) {

        perror("Dizin a�ma hatas�");

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

                perror("Dosya silme hatas�");

                closedir(dizin);

                return -1;

            }

        }

    }



   

    closedir(dizin);



    // Dizini siliniyor

    if (rmdir(dizin_adi) == -1) {

        perror("Dizin silme hatas�");

        return -1;

    }



    printf("Dizin ba�ar�yla silindi.\n");

    return 0;

}



int dizin_degistir(char *yeni_dizin) {

    if (chdir(yeni_dizin) == -1) { 

        perror("Dizin de�i�tirme hatas�");

        return -1;

    }

    printf("Ge�erli dizin ba�ar�yla degistirildi.\n");

    return 0;

}





void alt_dizinleri_listele(char *dizin_adi) {

    DIR *dizin;

    struct dirent *dosya;

    struct stat dosya_bilgisi;

    char dosya_yol[256];



    dizin = opendir(dizin_adi);

    if (dizin == NULL) {

        perror("Dizin a�ma hatas�");

        return;

    }



    printf("Dosya/Dizin Ad�\t\t\t\t\t\tT�r\t\tBoyut\t\tDe�i�tirilme Tarihi\n");

    printf("=================================================================================\n");



    while ((dosya = readdir(dizin)) != NULL) {

        sprintf(dosya_yol, "%s/%s", dizin_adi, dosya->d_name);

        if (stat(dosya_yol, &dosya_bilgisi) == -1) {

            perror("Dosya bilgisi alma hatas�");

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

        perror("Kaynak dizin a�ma hatas�");

        return -1;

    }



    

    if (mkdir(hedef_dizin, 0777) == -1) {

        perror("Hedef dizin olu�turma hatas�");

        closedir(kaynak);

        return -1;

    }



    // Kaynak dizinindeki dosyalar� hedef dizinine kopyalal�yoruz

    struct dirent *dosya;

    while ((dosya = readdir(kaynak)) != NULL) {

        if (strcmp(dosya->d_name, ".") != 0 && strcmp(dosya->d_name, "..") != 0) {

            char kaynak_yol[PATH_MAX];

            char hedef_yol[PATH_MAX];

            snprintf(kaynak_yol, sizeof(kaynak_yol), "%s/%s", kaynak_dizin, dosya->d_name);

            snprintf(hedef_yol, sizeof(hedef_yol), "%s/%s", hedef_dizin, dosya->d_name);

            // E�er bir alt dizinse

            if (dosya->d_type == DT_DIR) {

                // Yeni bir kopya olu�tur

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

    printf("Dizin ba�ar�yla kopyaland�.\n");

    return 0;

}





int dizin_tasi(char *kaynak_dizin, char *hedef_dizin) {

    // Kaynak dizinini hedef dizinine kopyala

    if (dizin_kopyala(kaynak_dizin, hedef_dizin) == -1) {

        printf("Dizin ta��ma ba�ar�s�z.\n");

        return -1;

    }



    // Kaynak dizinini sil

    if (dizin_sil(kaynak_dizin) == -1) {

        printf("Kaynak dizin silinemedi.\n");

        return -1;

    }



    printf("Dizin ba�ar�yla ta��nd�.\n");

    return 0;

}







int dosya_ara(char *aranacak_metin) {

    DIR *dizin;

    struct dirent *dosya;

    struct stat dosya_bilgisi;

    char dosya_yol[256];



    dizin = opendir(".");

    if (dizin == NULL) {

        perror("Dizin a�ma hatas�");

        return -1;

    }

   

    printf("Aranan metni i�eren dosyalar:\n");

    printf("================================\n");



    while ((dosya = readdir(dizin)) != NULL) {

        sprintf(dosya_yol, "%s/%s", ".", dosya->d_name);

        if (stat(dosya_yol, &dosya_bilgisi) == -1) {

            perror("Dosya bilgisi alma hatas�");

            closedir(dizin);

            return -1;

        }

        if (S_ISREG(dosya_bilgisi.st_mode)) {

            FILE *dosya_ptr = fopen(dosya_yol, "r");

            if (dosya_ptr == NULL) {

                perror("Dosya a�ma hatas�");

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

        perror("Dizin a�ma hatas�");

        return;

    }



    printf("Filtrelenmi� dosyalar:\n");

    printf("=======================\n");



    while ((dosya = readdir(dizin)) != NULL) {

        sprintf(dosya_yol, "%s/%s", ".", dosya->d_name);

        if (stat(dosya_yol, &dosya_bilgisi) == -1) {

            perror("Dosya bilgisi alma hatas�");

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

            if (dosya->d_type == DT_REG) { // Sadece dosyalar� al

                dosyalar[dosya_sayisi++] = strdup(dosya->d_name);

            }

        }

        closedir(dizin);

        qsort(dosyalar, dosya_sayisi, sizeof(char *), karsilastir);



     

        printf("S�ralanm�� dosyalar:\n");

        for (int i = 0; i < dosya_sayisi; i++) {

            printf("%s\n", dosyalar[i]);

            free(dosyalar[i]); 

        }

    } else {

        printf("Mevcut dizin a��lamad�!\n");

    }

}



int main() {

    int secim;

     while (1) {

    printf("\n\nDosya Sistem Gezgini Uygulamas�na Ho�geldiniz\n\n");

    printf("L�tfen yapmak istedi�iniz i�lem i�in ilgili tu�a bas�n�z!\n\n");

    printf("*Dosya ��lemleri i�in 1'e\n");

    printf("*Dizin ��lemleri i�in 2'e\n");

    printf("*Arama ve Filtreleme i�lemleri i�in 3'e\n");

    printf("*��k�� i�in 4'e bas�n�z\n\n");



    printf("Se�iminizi yap�n�z: ");

    scanf("%d", &secim);



    switch (secim) {

        case 1:

            printf("\n\nDosya ��lemleri men�s�ne giri� yap�ld�.\n");

            printf("Dosya i�lemleri i�in ilgili tu�a bas�n�z!\n");

            printf("a->Dosya Ac\n");

            printf("y->Dosya Yaz\n");

            printf("k->Dosya Kopyala\n");

            printf("t->Dosya Tas�\n");

            printf("s->Dosya Sil\n");

            printf("d->Dosya Duzenle\n");

            printf("f->Dosya Turu Degistir\n");

            printf("i->Dosya �zinleri Ayarla\n");

            printf("b->Dosya Sec\n");

            printf("r->Ana Men�'ye D�n\n\n");



            char dosya_secim;

            printf("Dosya i�lemi se�iminizi yap�n�z: ");

            scanf(" %c", &dosya_secim);



            switch (dosya_secim) {

                case 'a':

                    char dosya_adi[100];

                        printf("A�mak istedi�iniz dosyan�n ad�n� giriniz: ");

                        scanf("%s", dosya_adi);

                        int dosya_fd = dosya_ac(dosya_adi);

                        if (dosya_fd != -1) {

                            printf("dosya acildi");

                        }

                    break;

                case 'y':

                      char dosya_adi_yaz[100];

                        printf("Yazmak istedi�iniz dosyan�n ad�n� giriniz: ");

                        scanf("%s", dosya_adi_yaz);

                        char icerik[1000];

                        printf("Dosyaya yaz�lacak i�eri�i giriniz: ");

                        scanf("%s", icerik);

                        int yazma_sonuc = dosya_yaz(dosya_adi_yaz, icerik);

                        if (yazma_sonuc == 0) {

                            printf("Dosya ba�ar�yla yaz�ld�.\n");

                        }

                    break;

                case 'k':

                     char kaynak_dosya[100];

                        printf("Kopyalanacak dosyan�n ad�n� giriniz: ");

                        scanf("%s", kaynak_dosya);

                        char hedef_dosya[100];

                        printf("Hedef dosyan�n ad�n� giriniz: ");

                        scanf("%s", hedef_dosya);

                        int kopyalama_sonuc = dosya_kopyala(kaynak_dosya, hedef_dosya);

                        if (kopyalama_sonuc == 0) {

                            printf("Dosya ba�ar�yla kopyaland�.\n");

                        }

                    break;

                case 't':

                    char tasi_kaynak_dosya[100];

                        printf("Ta��nacak dosyan�n ad�n� giriniz: ");

                        scanf("%s", tasi_kaynak_dosya);

                        char tasi_hedef_dosya[100];

                        printf("Hedef dosyan�n ad�n� giriniz: ");

                        scanf("%s", tasi_hedef_dosya);

                        int tasi_sonuc = dosya_tasi(tasi_kaynak_dosya, tasi_hedef_dosya);

                        if (tasi_sonuc == 0) {

                            printf("Dosya ba�ar�yla ta��nd�.\n");

                        }

                    break;

                case 's':

                     char silinecek_dosya[100];

                     printf("Silinecek dosyan�n ad�n� giriniz: ");

                     scanf("%s", silinecek_dosya);

                     int silme_sonuc = dosya_sil(silinecek_dosya);

                    if (silme_sonuc == 0) {

                    printf("Dosya ba�ar�yla silindi.\n");

        }

                    break;

                case 'd':

                    char duzenlenecek_dosya[100];

                      printf("D�zenlemek istedi�iniz dosyan�n ad�n� giriniz: ");

                    scanf("%s", duzenlenecek_dosya);

                    int duzenleme_sonuc = dosya_duzenle(duzenlenecek_dosya);

                 if (duzenleme_sonuc == 0) {

                printf("Dosya ba�ar�yla d�zenlendi.\n");

                  }

                    break;

                case 'f':

               printf("T�r�n� de�i�tirmek istedi�iniz dosyan�n ad�n� giriniz: ");

               scanf("%s", dosya_adi); 

               char yeni_tur[10];

                printf("Yeni t�r� giriniz (�rn: txt): ");

               scanf("%s", yeni_tur);

               int tur_degistirme_sonuc = dosya_turu_degistir(dosya_adi, yeni_tur);

               if (tur_degistirme_sonuc == 0) {

               printf("Dosyan�n t�r� ba�ar�yla de�i�tirildi.\n");

                 }   break;

                case 'i':

                    char izinler[4]; // izinlerimiz rwx, rw-, r-x, r-- olabilece�ini varsayal�m

                    printf("Dosya izinlerini ayarlamak i�in izinler dizisini giriniz (�rn: rwx): ");

                    scanf("%s", izinler);

                     int izin_ayarla_sonuc = dosya_izinleri_ayarla(dosya_adi, izinler);

                    if (izin_ayarla_sonuc == 0) {

                   printf("Dosya izinleri ba�ar�yla g�ncellendi.\n");

                    }

                    break;

                case 'b':

                      char dosya_adlari[MAX_FILE_COUNT][MAX_FILE_NAME_LENGTH];

                      int dosya_sayisi = 0;

                      dosya_sec(dosya_adlari, &dosya_sayisi);



                      printf("Se�ilen dosyalar:\n");

                    for (int i = 0; i < dosya_sayisi; i++) {

                   printf("%s\n", dosya_adlari[i]);

                    }

                    break;

                case 'r':

                        printf("Ana Men�'ye d�n�l�yor...\n");

                        continue; // D�ng�n�n ba��na git

                default:

                    printf("Ge�ersiz se�im!\n");

            }

            break;

        case 2:

            printf("\n\nDizin ��lemleri men�s�ne giri� yap�ld�.\n");

             printf("Dizin i�lemi i�in ilgili tu�a bas�n�z!\n");

            printf("o->Dizin Olu�tur\n");

            printf("s->Dizin Sil\n");

            printf("d->Dizin De�i�tir\n");

            printf("l->Alt Dizinleri Listele\n");

            printf("k->Dizin Kopyala\n");

            printf("t->Dizin Ta��\n");

             printf("r->Ana Men�'ye D�n\n\n");



            char dizin_secim;

            printf("Dizin i�lemi se�iminizi yap�n�z: ");

            scanf(" %c", &dizin_secim);



            switch (dizin_secim) {

                case 'o':

                    char dizin_adi[100];

                    printf("Olu�turulacak dizinin ad�n� giriniz: ");

                    scanf("%s", dizin_adi);



                   if (dizin_olustur(dizin_adi) == -1) {

                   printf("Dizin olu�turma ba�ar�s�z.\n");

                       }

                    break;

                case 's':

                  printf("Silinecek dizinin ad�n� giriniz: ");

                  scanf("%s", dizin_adi);



                  if (dizin_sil(dizin_adi) == -1) {

                  printf("Dizin silme ba�ar�s�z.\n");

                    }

                    break;

                case 'd':

                   char yeni_dizin[100];

                        printf("Ge�mek istedi�iniz dizinin yolunu giriniz: ");

                        scanf("%s", yeni_dizin);

                        int degistirme_sonuc = dizin_degistir(yeni_dizin);

                        if (degistirme_sonuc == 0) {

                            printf("Dizin ba�ar�yla degistirildi.\n");

                        }

                    break;

                case 'l':

                 char secilen_dizin[100];

                        printf("Listelemek istedi�iniz dizinin adini giriniz: ");

                       

                        scanf("%s", secilen_dizin);

                        alt_dizinleri_listele(secilen_dizin);

                        break;

                    break;

                case 'k':

                    char kaynak_dizin[100];

                    char hedef_dizin[100];

                    printf("Kopyalanacak dizinin ad�n� giriniz: ");

                    scanf("%s", kaynak_dizin);

                    printf("Hedef dizinin ad�n� giriniz: ");

                    scanf("%s", hedef_dizin);

                    if (dizin_kopyala(kaynak_dizin, hedef_dizin) == -1) {

                    printf("Dizin kopyalama ba�ar�s�z.\n");

                    }

                    break;

                case 't':

                    printf("Ta��nacak dizinin ad�n� giriniz: ");

                    scanf("%s", kaynak_dizin);

                    printf("Hedef dizinin ad�n� giriniz: ");

                    scanf("%s", hedef_dizin);

                     if (dizin_tasi(kaynak_dizin, hedef_dizin) == -1) {

                     printf("Dizin ta��ma ba�ar�s�z.\n");

                       }

                    break;

                case 'r':

                        printf("Ana Men�'ye d�n�l�yor...\n");

                        continue; 

                default:

                    printf("Ge�ersiz se�im!\n");

            }

            break;

        case 3:

            printf("\n\nArama ve Filtreleme i�lemleri men�s�ne giri� yap�ld�.\n");

            printf("yapmak istedi�iniz i�lem i�in ilgili tu�a bas�n�z!\n");

            printf("a->Dosya Ara\n");

            printf("f->Dosya Filtrele\n");

            printf("s->Dosya S�ralama\n");

             printf("r->Ana Men�'ye D�n\n\n");



            char arama_secim;

            printf("Arama ve filtreleme i�lemi se�iminizi yap�n�z:\n ");

            scanf(" %c", &arama_secim);



            switch (arama_secim) {

                case 'a':

                    char aranacak_metin[100];

                    printf("Aranacak metni giriniz: \n");

                   scanf("%s", aranacak_metin);

                   printf("\n",aranacak_metin,"\n");

                  if (dosya_ara(aranacak_metin) == -1) {

                  printf("Dosya arama ba�ar�s�z.\n");

                   }

                    break;

                case 'f':

                    char filtre_kriteri[100];

                    printf("Filtre kriterini giriniz: \n");

                    scanf("%s", filtre_kriteri);

                    dosya_filtrele(filtre_kriteri);

                    break;

                case 's':

                    printf("Dosya S�ralama i�lemi se�ildi.\n");



                     char siralama_kriteri[10];

                     printf("S�ralama kriterini girin (�rn. ad): \n");

                     scanf("%s", siralama_kriteri);



                    dosya_siralama(siralama_kriteri);

                    break;

                case 'r':

                        printf("Ana Men�'ye d�n�l�yor...\n");

                        continue;   

                default:

                    printf("Ge�ersiz se�im!\n");

            }

            break;

        case 4:

            printf("Program sonland�r�l�yor...\n");

                exit(0);

            break;

        default:

            printf("Ge�ersiz se�im!\n");

    }

}

    return 0;

}


