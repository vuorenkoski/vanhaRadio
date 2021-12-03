#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <wiringPi.h>
#include <fcntl.h>

#define READ_END 0
#define WRITE_END 1

const char *PATH_TO_MUSIC = "/home/pi/music";
const char *PATH_TO_MESSAGE = "/home/pi/control/radioViesti.txt";
const char *PATH_TO_RADIODAT = "/home/pi/radio.dat";
const char *PATH_TO_RADIO_COMMAND = "/var/www/html/soita_radiokanava.txt";
const char *PATH_TO_LIST_COMMAND = "/var/www/html/soita_lista.txt";
const char *PATH_TO_TRACK_COMMAND = "/var/www/html/soita_kappale.txt";
const char *PATH_TO_HTML_MESSAGE = "/var/www/html/kappale.txt";
const char *PATH_TO_RADIOSTATIONLIST = "/var/www/html/radiokanavat.txt";

int debug=0;
int fd[2], fd2[2];
int lista=0, uusiraita=1;
int nappi=0, radio=0, uusilista=1, aloitus=1, eka_raita=0;
int etsii=0, etsii_lkm=0, soitetut[100], soitetut_lkm;
int soitetutListat[30], soitetutListat_lkm=0, magneetti, raita, raitoja, listat;
char artist[150], title[150], year[50], lista_s[30][50];

void kappaleenDatanLahetys(char* title, char* artist, char* year) {
  FILE *fp;
  int i;
  char buffer[300];

  fp=fopen(PATH_TO_HTML_MESSAGE,"w");
  fprintf(fp,"Kappale: %s\n",title);
  fprintf(fp,"Lista: %s\n",artist);
  fprintf(fp,"Vuosi: %s\n",year);
  fclose(fp);
  sprintf(buffer,"%s:%s:%s",title,artist,year);
  for (i=0;i<strlen(buffer);i++) {
    if (buffer[i]==' ') buffer[i]='_';
  }
  fp=fopen(PATH_TO_MESSAGE, "w");
  fputs(buffer,fp);
  fclose(fp);
}

void radiokanava (int kanava, int mpeg123putki) {
  int i;
  char address[200], station[50], buffer[200];
  FILE *fp;

  fp=fopen(PATH_TO_RADIOSTATIONLIST,"r");
  for (i=0;i<kanava;i++) {
    fgets(address,200,fp);
    fgets(address,200,fp);
  }
  fgets(station,50,fp);
  station[strlen(station)-1]='\0';
  fgets(address,200,fp);
  fclose(fp);

  sprintf(buffer,"STOP\n");
  write(mpeg123putki,buffer,(strlen(buffer)));
  sprintf(buffer,"load %s",address);
  write(mpeg123putki,buffer,(strlen(buffer)));
  kappaleenDatanLahetys(station,"radio","");
  radio=1;
}

void pinAlustus() {
  wiringPiSetup();
  pinMode(0,INPUT);
  pinMode(1,INPUT);
  pinMode(2,INPUT);
  pinMode(3,INPUT);
  pinMode(7,INPUT);
  pullUpDnControl(0,PUD_UP);
  pullUpDnControl(1,PUD_UP);
  pullUpDnControl(2,PUD_UP);
  pullUpDnControl(3,PUD_UP);
}

void tarkistaNapit() {
  int i;

  i=digitalRead(0)+(2*digitalRead(1))+(digitalRead(2)*4)+(digitalRead(3)*8);
  if ((nappi!=0) && (i==11)) {
    nappi=0;
    radio=0;
    uusilista=1;
    for (i=0;i<listat;i++) soitetutListat[i]=0;
    soitetutListat_lkm=0;
    aloitus=1;
  }
  if ((nappi!=1) && (i==8)) {
    nappi=1;
    radiokanava(2,fd[WRITE_END]);
  }
  if ((nappi!=2) && (i==12)) {
    nappi=2;
    radiokanava(1,fd[WRITE_END]);
  }
  if ((nappi!=3) && (i==0)) {
    nappi=3;
    radiokanava(2,fd[WRITE_END]);
  }
}

void tarkistaKanavavalitsin() {
  char buffer[200];
  if ((digitalRead(7)!=magneetti) && (radio==0)) {
    magneetti=digitalRead(7);
    delay(200);
    if (debug) printf("Magneetti:%i\n",magneetti);
    if (etsii==0) { // etsintä alkaa
      etsii=1;
      sprintf(buffer,"load %s/hum.mp3\n", PATH_TO_MUSIC);
      write(fd[WRITE_END],buffer,(strlen(buffer)));
      etsii_lkm=0;
    } else {
      etsii_lkm++;
      if (etsii_lkm==3) {
        uusilista=1;
        eka_raita=1;
        etsii=0;
      }
    }
  }
}

void soitaRaita (char* songfile) {
  char buffer[200];

  sprintf(buffer,"load %s\n",songfile);
  write(fd[WRITE_END],buffer,(strlen(buffer)));
  strcpy(artist,"???");
  strcpy(title,"???");
  strcpy(year,"");
}

void tarkistaHtmlkaskyt() {
  int songid,i;
  char buffer[200],songfile[200];
  FILE *fp;
  // Tarkistetaanko onko html käskyä kappaleesta
  if (access(PATH_TO_TRACK_COMMAND, F_OK)==0) {
    if (debug) printf("uusi raita käsky\n");
    fp=fopen(PATH_TO_TRACK_COMMAND,"r");
    fgets(buffer,10,fp);
    songid=atoi(buffer);
    fclose(fp);
    remove(PATH_TO_TRACK_COMMAND);
    sprintf(buffer,"%s/files.txt",PATH_TO_MUSIC);
    if (debug) printf("luetaan tiedosto: %s, haetaan numero %i\n",buffer,songid);

    fp=fopen(buffer,"r");
    for (i=0;i<songid;i++) {
      fgets(songfile,200,fp);
    }
    fgets(songfile,200,fp);
    if (debug) printf("löytyi tiedosto: %s\n",songfile);
    soitaRaita(songfile);
    radio=0;
  }

  // Tarkistetaan onko html käskyä listasa
  if (access(PATH_TO_LIST_COMMAND, F_OK)==0) {
    fp=fopen(PATH_TO_LIST_COMMAND,"r");
    fgets(buffer,10,fp);
    lista=atoi(buffer);
    aloitus=1;
    uusilista=1;
    fclose(fp);
    remove(PATH_TO_LIST_COMMAND);
  }

  // Tarkistetaan onko html käskyä radioasemasta
  if (access(PATH_TO_RADIO_COMMAND, F_OK)==0) {
    fp=fopen(PATH_TO_RADIO_COMMAND,"r");
    fgets(buffer,10,fp);
    radiokanava(atoi(buffer),fd[WRITE_END]);
    fclose(fp);
    remove(PATH_TO_RADIO_COMMAND);
  }
}

void uusiRaitaSoimaan () {
  int i,j,k;
  char buffer[200], raita_s[150];
  FILE *fp;

  soitetut_lkm++;
  if (soitetut_lkm>raitoja) {
    uusilista=1;
  } else {
    if (lista==0) { // jos vanha radio paalla
      if (access(PATH_TO_RADIODAT, F_OK)==0) {
        fp=fopen(PATH_TO_RADIODAT,"r");
        k=0;
        for (j=0;j<raitoja;j++) {
          soitetut[j]=fgetc(fp)-'0';
          if (soitetut[j]==0) k++;
        }
        fclose(fp);
      } else { // radio.dat tiedostoa ei ole, alustetaan nollilla
        for (j=0;j<raitoja;j++) {
          soitetut[j]=0;
          k++;
        }
      }
      do {
        raita=(rand()%raitoja);
        if (debug) printf("satunnaisluku:%i\n",raita);
      } while (soitetut[raita]==1);
      soitetut[raita]=1;

      if (k<2) {
        for (j=0;j<raitoja;j++) soitetut[j]=0;
      }

      fp=fopen(PATH_TO_RADIODAT,"w");
      for (j=0;j<raitoja;j++) fputc(soitetut[j]+'0',fp);
      fclose(fp);
    } else { // jos on muuta musiikkia
      do {
        raita=(rand()%raitoja);
        if (debug) printf("satunnaisluku:%i\n",raita);
      } while (soitetut[raita+1]==1);
      soitetut[raita+1]=1;
    }

    // etsi musiikkitiedoston nimi listalta
    sprintf(buffer,"%s/%s.txt", PATH_TO_MUSIC, lista_s[lista]);
    fp=fopen(buffer,"r");
    fgets(raita_s,150,fp);
    for (i=0;i<(raita+1);i++) fgets(raita_s,150,fp);
    raita_s[strlen(raita_s)-1]='\0';
    fclose(fp);

    // laitetaan raita soimaan
    sprintf(buffer,"%s/%s/%s\n",PATH_TO_MUSIC, lista_s[lista], raita_s);
    soitaRaita(buffer);

    if (debug) printf("ladataan raita %i: %s\n",raita,buffer);
    if (eka_raita) {
       sprintf(buffer,"JUMP +30s\n");
       write(fd[WRITE_END],buffer,(strlen(buffer)));
       eka_raita=0;
    }
    sleep(1);
    magneetti=digitalRead(7);
  }
}

void uusiListaSoimaan () {
  int i;
  FILE *fp;
  char buffer[200], nr_s[5];

  if (aloitus) {
    aloitus=0;
  } else {
    if (soitetutListat_lkm==listat) {
      for (i=0;i<listat;i++) soitetutListat[i]=0;
      soitetutListat_lkm=0;
    }
    do {
      lista=(rand()%listat);
    } while (soitetutListat[lista]==1);
  }
  soitetutListat[lista]=1;
  soitetutListat_lkm++;
  uusiraita=1;
  sprintf(buffer,"%s/%s.txt", PATH_TO_MUSIC, lista_s[lista]);
  fp=fopen(buffer,"r");
  fgets(nr_s,5,fp);
  raitoja=atoi(nr_s);
  fclose(fp);
  if (debug) printf("Listalla raitoja:%i\n",raitoja);
  for (i=0;i<raitoja;i++) soitetut[i+1]=0;
  soitetut_lkm=0;
}

void demoni() {
  int i,size,kohta=0;
  char c,buffer[200];
  time_t t;
  FILE *fp;

  srand((unsigned) time(&t));
  magneetti=digitalRead(7);

  // Ladataan albumien tiedot
  sprintf(buffer,"%s/albums.txt", PATH_TO_MUSIC);
  fp=fopen(buffer,"r");
  i=0;
  while (fgets(lista_s[i],50,fp)!=NULL)
  {
    lista_s[i][strlen(lista_s[i])-1]='\0';
    soitetutListat[i]=0;
    i++;
  }
  listat=i;
  fclose(fp);

  // odotaa kunnes mpg123 antaa ok viestin
  i=0;
  do {
    size=read(fd2[READ_END],&c,1);
    if (size==1) { buffer[i]=c; i++; }
  } while (c!='\n');
  buffer[i-1]='\0';
  if (debug) {
    printf("viesti:%s---\n",buffer);
    if (strncmp(buffer,"@R MPG123 (ThOr) v8",18)==0) printf("MPG123 ok\n");
  }

  while (1) {
    size=read(fd2[READ_END],&c,1);
    if (size==1) {
      buffer[kohta]=c;
      kohta++;
      if (c=='\n') {           // on tullut viesti
        buffer[kohta-1]='\0';
        kohta=0;
        if (radio==0) {
          if (!etsii) {
            if (strncmp(buffer,"@I ID3v2.album:",14)==0) strcpy(artist,&buffer[15]);
            if (strncmp(buffer,"@I ID3v2.title:",14)==0) strcpy(title,&buffer[15]);
            if (strncmp(buffer,"@I ID3v2.year:",13)==0) strcpy(year,&buffer[14]);
            if (strncmp(buffer,"@P 2",4)==0) kappaleenDatanLahetys(title, artist, year); // play koodi
            if (strncmp(buffer,"@P 0",4)==0) uusiraita=1; // end koodi
          } else { // jos etsintä on päällä
            if (strncmp(buffer,"@P 0",4)==0) { // jos viritysääni loppunut, aloitetaan alusta
              sprintf(buffer,"load /home/pi/music/hum.mp3\n");
              write(fd[WRITE_END],buffer,(strlen(buffer)));
            }
          }
        }
      }
    }

    if (uusilista) {
      uusilista=0;
      uusiListaSoimaan();
    }
    if (uusiraita) {
      uusiraita=0;
      uusiRaitaSoimaan();
    }
    tarkistaNapit();
    tarkistaHtmlkaskyt();
    tarkistaKanavavalitsin();
  }
}


void main()
{
  pid_t child_pid;

  if (pipe(fd)==-1) perror("pipe error");
  pipe(fd2);
  child_pid=fork();
  if (child_pid==0) {
     dup2(fd[READ_END],STDIN_FILENO);
     dup2(fd2[WRITE_END],STDOUT_FILENO);
     close(fd[READ_END]);
     close(fd[WRITE_END]);
     close(fd2[READ_END]);
     close(fd2[WRITE_END]);

     execlp("/usr/bin/mpg123","/usr/bin/mpg123","-R",NULL); //here I need to execute whatever program was given to user_input
     exit(1); //making sure to avoid fork bomb
  } else {
    close(fd[READ_END]);
    close(fd2[WRITE_END]);
    fcntl(fd2[READ_END], F_SETFL, fcntl(fd2[READ_END], F_GETFL) | O_NONBLOCK); // jotta read ei jää odottamaan merkkiä
    pinAlustus();
    demoni();
  }
}
