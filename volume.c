#include <stdio.h>
#include <string.h>
#include <wiringSerial.h>
#include <wiringPi.h>
#include <unistd.h>
#include <stdlib.h>

const char *PATH_TO_VOLUME_DIRECTION = "/var/www/html/aseta_volume.txt";
const int pt[10] = {35,50,70,100,200,300,400,500,750,950}; // vanhan potentiometrin skaala ei ole ihan lineaarinen

void aseta(int vol)
{
  FILE *in;
  extern FILE *popen();
  char buff[512];

  sprintf(buff,"/usr/bin/amixer sset Headphone,0 %i%c",vol,'%');
  in=popen(buff, "r");
  pclose(in);
}

int tulkitse (int lukema) {
  int i;
  if (lukema<pt[0]) return 0;
  for (i=0;i<9;i++) {
    if (lukema<pt[i+1]) return 50+5*i+(5*(lukema-pt[i]))/(pt[i+1]-pt[i]);
  }
  return 95;
}

void tarkistaHtmlSaato() {
  FILE *fp;
  char buffer[10];
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
  int lukema,volume=0;
  int serial;

  serial=serialOpen("/dev/ttyAMA0", 9600);

  if (serial==-1) {
   printf("Serial open error, quitting\n");
   return;
  }

  while (1) {
    do {
      while (serialDataAvail(serial)==0) {
        delay(5);
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
