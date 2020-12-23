# vanhaRadio

Projektissa vanhaan 50-luvun radioon (Salora Milano) on asennettu raspberry pi, jolla voi soittaa mp3 tiedostoja (esim. YLE arkston vanoja radio-ohjelmia tai muuta mp3 musiikkia). Sillä voi myös striimata interenradiokanavia. 

Radion "kanavina" toimivat albummit. Kanavaa voi vaihtaa vanhalla kanavalitsemlla, johon on liitetty magneetit+magneettianturit jotka rekisteröivät kanavavalitsemen pyörityksen. Äänenvoimakkuus nappi on totettu ATMEGA328 mikrokontrollelilla, joka ottaa analogista signaalia sisään. Atamega sylkee ulos äänenvoimakkuusdataa sarjaportin kautta raspikselle.

Rasperry toimii myös www-serverinä, jonka kautta soitettavia tiedostoja/albumeja voi valita sekä säätää äänenvoimakkuutta.

[<img src="radio.jpg"]
