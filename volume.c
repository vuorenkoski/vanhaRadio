#include <stdio.h>
#include <string.h>
#include <wiringSerial.h>
#include <wiringPi.h>
#include <unistd.h>
#include <stdlib.h>


const char *PATH_TO_VOLUME_DIRECTION = "/var/www/html/aseta_volume.txt";
const char *PATH_TO_VOLUME_DATA = "/var/www/html/volume.txt";

void aseta(int vol)
{
  FILE *in;
  extern FILE *popen();
  char buff[512];

  sprintf(buff,"/usr/bin/amixer sset PCM,0 %i%c",vol,'%');
  in=popen(buff, "r");
  pclose(in);
  in=fopen(PATH_TO_VOLUME_DATA,"w");
  fprintf(in,"%i",vol);
  fclose(in);
}

int tulkitse (int lukema) {
   if (lukema<35) return 0;
   if (lukema<50) return 55;
   if (lukema<70) return 60;
   if (lukema<100) return 65;
   if (lukema<200) return 70;
   if (lukema<300) return 75;
   if (lukema<400) return 80;
   if (lukema<500) return 85;
   if (lukema<750) return 90;
   if (lukema<950) return 95;
   return 100;
}

void tarkistaHtmlSaato() {
  FILE *fp;
  int volume;
  char buffer[500];
  if (access(PATH_TO_VOLUME_DIRECTION, F_OK)==0) {
    fp=fopen(PATH_TO_VOLUME_DIRECTION, "r");
    fgets(buffer,10,fp);
    aseta(atoi(buffer));
    fclose(fp);
    remove(PATH_TO_VOLUME_DIRECTION);
  }
}

void main()
{
  char c;
  int lukema,volume;
  int serial;

  serial=serialOpen("/dev/ttyAMA0", 9600);

  if (serial==-1)
  {
   printf("Serial open error, quitting\n");
   return;
  }

  while (1==1)
  {
    do
    {
      while (serialDataAvail(serial)==0)
      {
        delay (5);
        if (volume==0) {
          tarkistaHtmlSaato();
        }
      }
      c=serialGetchar(serial);
    } while (c!='a');
    lukema=0;
    c=serialGetchar(serial);
    lukema=(c-'0')*100;
    c=serialGetchar(serial);
    lukema=lukema+((c-'0')*10);
    c=serialGetchar(serial);
    lukema=lukema+(c-'0');

    volume=tulkitse(lukema);
    aseta(volume);
//    printf("asetettu %i\%, kun viesti oli %i.\n",volume,lukema);
  }
}
