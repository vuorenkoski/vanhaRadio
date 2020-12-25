#!/bin/bash
cp /var/www/html/* /home/pi/vanhaRadio/html/.
cp /home/pi/musiikki.c /home/pi/vanhaRadio/.
cp /home/pi/volume.c /home/pi/vanhaRadio/.
cp /home/pi/volume.ino /home/pi/vanhaRadio/.
cp /home/pi/viesti.c /home/pi/vanhaRadio/.
cp /home/pi/asennus.txt /home/pi/vanhaRadio/.
cp /home/pi/repoon.sh /home/pi/vanhaRadio/.
cp /home/pi/albums_to_json.py /home/pi/vanhaRadio/.
cd /home/pi/vanhaRadio
git add .
git commit -m "$1"
git push
