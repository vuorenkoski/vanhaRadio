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

int debug=0;

int lahetys1(char* toiminto, char* teksti)
{
  FILE *in;
  extern FILE *popen();
  char buff[512];
  int onnistui=0;

  sprintf(buff,"wget -O - -q \"http://www.vuorenkoski.fi/control/kasky.php?toiminto=%s&teksti=%s\"",toiminto,teksti);
  if (in = popen(buff, "r"))
  {
    onnistui='0'-fgetc(in);
  }

  pclose(in);
  return onnistui;
}


int lahetys2(char* teksti)
{
  FILE *fp;
  fp=fopen("/home/pi/control/radioViesti.txt", "w");
  fputs(teksti,fp);
  fclose(fp);
}

int lahetys3(char* teksti)
{
  FILE *fp;
  fp=fopen("/home/pi/control/raitaNyt.txt", "w");
  fputs(teksti,fp);
  fclose(fp);
}

void kappaleenDatanLahetys(char* title, char* artist, char* year) {
  FILE *fp;
  int i;
  char buffer[300];

  fp=fopen("/var/www/html/kappale.txt","w");
  fprintf(fp,"Kappale: %s\n",title);
  fprintf(fp,"Lista: %s\n",artist);
  fprintf(fp,"Vuosi: %s\n",year);
  fclose(fp);
  sprintf(buffer,"%s:%s:%s",title,artist,year);
  for (i=0;i<strlen(buffer);i++)
  {
    if (buffer[i]==' ') buffer[i]='_';
  }
  lahetys2(buffer);
  sprintf(buffer,"%s\n%s\n%s\n",title,artist,year);
  lahetys3(buffer);
}

void radiokanava (int kanava, int mpeg123putki) {
  int i;
  char address[200], station[50], buffer[200];
  FILE *fp;

  fp=fopen("/var/www/html/radiokanavat.txt","r");
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
}


void main()
{
  int lopetus,i,j,k,uusilista,uusiraita,lista,listat,raitoja,raita,size,kohta,nappi,soitetut[100],soitetut_lkm;
  int soitetutListat[20],soitetutListat_lkm,magneetti,eka_raita,etsii,etsii_lkm,radio,aloitus,songid;
  char lista_s[20][50],nr_s[5],raita_s[150],buffer[200],c,artist[150],title[150],year[50],songfile[200];
  time_t t;
  FILE *fp,*fp2;

  int fd[2], fd2[2];
  pid_t child_pid;

  lopetus=0;
  kohta=0;
  etsii=0;
  nappi=0;
  radio=0;

  srand((unsigned) time(&t));

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

  magneetti=digitalRead(7);

  if (pipe(fd)==-1) perror("pipe error");
  pipe(fd2);
  child_pid=fork();
  if (child_pid==0) //if the current process is a child of the main process
  {
     dup2(fd[READ_END],STDIN_FILENO);
     dup2(fd2[WRITE_END],STDOUT_FILENO);
     close(fd[READ_END]);
     close(fd[WRITE_END]);
     close(fd2[READ_END]);
     close(fd2[WRITE_END]);

     execlp("/usr/bin/mpg123","/usr/bin/mpg123","-R",NULL); //here I need to execute whatever program was given to user_input
     exit(1); //making sure to avoid fork bomb
  } else
  {
    close(fd[READ_END]);
    close(fd2[WRITE_END]);
    fcntl(fd2[READ_END], F_SETFL, fcntl(fd2[READ_END], F_GETFL) | O_NONBLOCK); // jotta read ei jää odottamaan merkkiä

    fp=fopen("/home/pi/music/albums.txt","r");
    i=0;
    while (fgets(lista_s[i],50,fp)!=NULL)
    {
      lista_s[i][strlen(lista_s[i])-1]='\0';
      soitetutListat[i]=0;
      i++;
    }
    listat=i;
    fclose(fp);

    // aloitetaan radiolla
    uusilista=1;
    aloitus=1;
    lista=0;
    eka_raita=0;
    uusiraita=1;
    soitetutListat_lkm=0;

    i=0; // odotaa kunnes mpg123 antaa ok viestin
    do {
      size=read(fd2[READ_END],&c,1);
      if (size==1) { buffer[i]=c; i++; }
    } while (c!='\n');
    buffer[i-1]='\0';
//    printf("viesti:%s---\n",buffer);
//    if (strncmp(buffer,"@R MPG123 (ThOr) v8",18)==0) printf("MPG123 ok\n");


    while (1)
    {
      size=read(fd2[READ_END],&c,1);
      if (size==1)
      {
        buffer[kohta]=c;
        kohta++;
        if (c=='\n')
        {           // on tullut viesti
          buffer[kohta-1]='\0';
//          printf("%s\n",buffer);
          kohta=0;
          if (radio==0)
          {
            if (!etsii)
            {
              if (strncmp(buffer,"@I ID3v2.album:",14)==0) strcpy(artist,&buffer[15]);
              if (strncmp(buffer,"@I ID3v2.title:",14)==0) strcpy(title,&buffer[15]);
              if (strncmp(buffer,"@I ID3v2.year:",13)==0) strcpy(year,&buffer[14]);
              if (strncmp(buffer,"@P 2",4)==0) // play koodi
              {
                kappaleenDatanLahetys(title, artist, year);
              }
              if (strncmp(buffer,"@P 0",4)==0) uusiraita=1; // end koodi
            } else // jos etsintä on päällä
            {
              if (strncmp(buffer,"@P 0",4)==0) // jos viritysääni loppunut, aloitetaan alusta
              {
                sprintf(buffer,"load /home/pi/music/hum.mp3\n");
                write(fd[WRITE_END],buffer,(strlen(buffer)));
              }
            }
          }
        }
      }

      if (uusilista)
      {
        uusilista=0;
        if (aloitus)
        {
          aloitus=0;
        } else
        {
          if (soitetutListat_lkm==listat)
          {
            for (i=0;i<listat;i++) soitetutListat[i]=0;
            soitetutListat_lkm=0;
          }
          do { lista=(rand()%listat); } while (soitetutListat[lista]==1);
        }
        soitetutListat[lista]=1;
        soitetutListat_lkm++;
        if (debug) printf("listoja kuunneltu=%i\n",soitetutListat_lkm);
        if (debug) printf("Uusi lista nr. %i valittu:%s\n",lista,lista_s[lista]);
        uusiraita=1;
        sprintf(buffer,"/home/pi/music/%s.txt",lista_s[lista]);
        fp=fopen(buffer,"r");
        fgets(nr_s,5,fp);
        raitoja=atoi(nr_s);
        fclose(fp);
        if (debug) printf("Listalla raitoja:%i\n",raitoja);
        for (i=0;i<raitoja;i++) soitetut[i+1]=0;
        soitetut_lkm=0;
      }

      if (uusiraita)
      {
        uusiraita=0;
        soitetut_lkm++;
        if (soitetut_lkm>raitoja) uusilista=1; else
        {
          // jos vanha radio paalla
          if (lista==0)
          {
            fp=fopen("/home/pi/radio.dat","r");
            k=0;
            for (j=0;j<raitoja;j++)
            {
              soitetut[j]=fgetc(fp)-'0';
              if (soitetut[j]==0) k++;
            }
            fclose(fp);
            do
            {
              raita=(rand()%raitoja);
              if (debug) printf("satunnaisluku:%i\n",raita);
            } while (soitetut[raita]==1);
            soitetut[raita]=1;

            if (k<2) { for (j=0;j<raitoja;j++) soitetut[j]=0; }

            fp=fopen("/home/pi/radio.dat","w");
            for (j=0;j<raitoja;j++) fputc(soitetut[j]+'0',fp);
            fclose(fp);
          } else

          // jos on muuta musiikkia
          {
            do
            {
              raita=(rand()%raitoja);
              if (debug) printf("satunnaisluku:%i\n",raita);
            } while (soitetut[raita+1]==1);
            soitetut[raita+1]=1;
          }

          // etsi musiikkitiedoston nimi listalta
          sprintf(buffer,"/home/pi/music/%s.txt",lista_s[lista]);
          fp=fopen(buffer,"r");
          fgets(nr_s,5,fp);
          for (i=0;i<(raita+1);i++) fgets(raita_s,150,fp);
          raita_s[strlen(raita_s)-1]='\0';
          fclose(fp);

          // laitetaan raita soimaan
          sprintf(buffer,"load /home/pi/music/%s/%s\n",lista_s[lista],raita_s);
          write(fd[WRITE_END],buffer,(strlen(buffer)));
          strcpy(artist,"???");
          strcpy(title,"???");
          strcpy(year,"");

          if (debug) printf("ladataan raita %i: %s\n",raita,buffer);
          if (eka_raita)
          {
            sprintf(buffer,"JUMP +30s\n");
            write(fd[WRITE_END],buffer,(strlen(buffer)));
            eka_raita=0;
          }
          sleep(1);
          magneetti=digitalRead(7);
        }
      }

      i=digitalRead(0)+(2*digitalRead(1))+(digitalRead(2)*4)+(digitalRead(3)*8);
      if ((nappi!=0) && (i==11))
      {
        nappi=0;
//        printf("nappi0\n");
        radio=0;
        uusilista=1;
        for (i=0;i<listat;i++) soitetutListat[i]=0;
        soitetutListat_lkm=0;
        aloitus=1;
      }
      if ((nappi!=1) && (i==8))
      {
        nappi=1;
//        printf("nappi1\n");
        radiokanava(2,fd[WRITE_END]);
        radio=1;
      }
      if ((nappi!=2) && (i==12))
      {
        nappi=2;
//        printf("nappi2\n");
        radiokanava(1,fd[WRITE_END]);
        radio=1;
      }
      if ((nappi!=3) && (i==0))
      {
        nappi=3;
//        printf("nappi3\n");
        radiokanava(2,fd[WRITE_END]);
        radio=1;
      }

      // Tarkistetaanko onko html käskyä kappaleesta
      if (access("/var/www/html/soita_kappale.txt", F_OK)==0) {
        fp2=fopen("/var/www/html/soita_kappale.txt","r");
        fgets(buffer,10,fp2);
        songid=atoi(buffer);
        fclose(fp2);
        remove("/var/www/html/soita_kappale.txt");
        fp2=fopen("/home/pi/music/files.txt","r");
        for (i=0;i<songid;i++) {
            fgets(songfile,200,fp2);
        }
        fgets(songfile,200,fp2);
        sprintf(buffer,"load %s\n",songfile);
        write(fd[WRITE_END],buffer,(strlen(buffer)));
        radio=0;
        strcpy(artist,"???");
        strcpy(title,"???");
        strcpy(year,"");
      }

      // Tarkistetaan onko html käskyä listasa
      if (access("/var/www/html/soita_lista.txt", F_OK)==0) {
        fp2=fopen("/var/www/html/soita_lista.txt","r");
        fgets(buffer,10,fp2);
        lista=atoi(buffer);
        aloitus=1;
        uusilista=1;
        fclose(fp2);
        remove("/var/www/html/soita_lista.txt");
      }

      // Tarkistetaan onko html käskyä radioasemasta
      if (access("/var/www/html/soita_radiokanava.txt", F_OK)==0) {
        fp2=fopen("/var/www/html/soita_radiokanava.txt","r");
        fgets(buffer,10,fp2);
        radiokanava(atoi(buffer),fd[WRITE_END]);
        fclose(fp2);
        radio=1;
        remove("/var/www/html/soita_radiokanava.txt");
      }


      if ((digitalRead(7)!=magneetti) && (radio==0))
      {
        magneetti=digitalRead(7);
        delay(200);
        if (debug) printf("Magneetti:%i\n",magneetti);
        if (etsii==0) // etsintä alkaa
        {
          etsii=1;
          sprintf(buffer,"load /home/pi/music/hum.mp3\n");
          write(fd[WRITE_END],buffer,(strlen(buffer)));
          etsii_lkm=0;
        } else
        {
          etsii_lkm++;
          if (etsii_lkm==3)
          {
            uusilista=1;
            eka_raita=1;
            etsii=0;
          }
        }
      }
    }
  }
}

