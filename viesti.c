#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void main()
{
  char buffer[100],buffer2[200];
  FILE *fp;
  FILE *in;
  extern FILE *popen();

  while (1)
  {
    if (access("/home/pi/control/radioViesti.txt", F_OK)!=-1)
    {
      fp=fopen("/home/pi/control/radioViesti.txt", "r");
      fgets(buffer,90,fp);
      fclose(fp);
      remove("/home/pi/control/radioViesti.txt");

      sprintf(buffer2,"wget -O - -q \"http://192.168.10.22/radio.php?toiminto=radio&teksti=%s\"\0",buffer);
      in=popen(buffer2, "r");
      pclose(in);

      sprintf(buffer2,"wget -O - -q \"http://www.vuorenkoski.fi/control/kasky.php?toiminto=radio&teksti=%s\"\0",buffer);
      in=popen(buffer2, "r");
      pclose(in);
    }
    sleep(1);
  }
}
