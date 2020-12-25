import os
import json
from tinytag import TinyTag

root = '/home/pi/music/'
jsondatafile = '/var/www/html/contents.json'
deamondatafile = '/home/pi/music/files.txt'
albumdatafile = '/home/pi/music/albums.txt'

os.remove('/home/pi/radio.dat')

data = {'albums':[]}
id = 0
aid = 1
dfile = open(deamondatafile, 'w')
adfile = open(albumdatafile, 'w')
adfile.write('radio\n')

entries = os.listdir(root)
for entry in entries:
    if '.' not in entry:
        files=os.listdir(root+entry+'/')
        album={'name': entry, 'songs': []}
        if entry=='radio':
            album['id']=0
        else:
            album['id']=aid
            adfile.write(entry+'\n')
            aid+=1
        for file in files:
            if '.mp3' in file:
                filename=root+entry+'/'+file
                tag = TinyTag.get(filename)
                track={'id': id, 'filename':file, 'full filename': filename, 'artist':tag.album, 'title':tag.title, 'year':tag.year}
                id+=1
                dfile.write(filename+'\n')
                album['songs'].append(track)
        data['albums'].append(album)

        alistfile = open(root+entry+".txt", 'w')
        alistfile.write(str(len(album['songs']))+'\n');
        for track in album['songs']:
            alistfile.write(track['filename']+'\n')
        alistfile.close()


with open(jsondatafile, 'w') as write_file:
    json.dump(data, write_file)
