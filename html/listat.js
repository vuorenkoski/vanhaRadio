var albumit = [];
var kappale = "";
var radiokanavat = "";

var soitaKappale = function (id) {
    return (function() {
        console.log('soita kappale:'+id);
        var xhttp = new XMLHttpRequest();
        xhttp.open('POST', 'kasky.php', true);
        xhttp.setRequestHeader('Content-type', 'application/x-www-form-urlencoded');
        xhttp.send('toiminto=kappale&id='+id);
    })
}

var soitaLista = function (id) {
    return (function() {
        console.log('soita lista:'+id);
        var xhttp = new XMLHttpRequest();
        xhttp.open('POST', 'kasky.php', true);
        xhttp.setRequestHeader('Content-type', 'application/x-www-form-urlencoded');
        xhttp.send('toiminto=lista&id='+id);
    })
}

var soitaRadiokanava = function (id) {
    return (function() {
        console.log('soita radiokanava:'+id);
        var xhttp = new XMLHttpRequest();
        xhttp.open('POST', 'kasky.php', true);
        xhttp.setRequestHeader('Content-type', 'application/x-www-form-urlencoded');
        xhttp.send('toiminto=radiokanava&id='+id);
    })
}


// Albumien haku
var drawAlbumit = function() {
  var ul = document.createElement('ul');

  albumit.forEach(function (lista) {
    var li = document.createElement('li');
    ul.appendChild(li);
    var node = document.createElement('button');
    node.setAttribute('class', 'listabtn');
    node.innerHTML=lista.name;
    node.addEventListener('click', drawAlbumi(lista));
    li.appendChild(node);
  })
  document.getElementById('albumit').appendChild(ul);
  var aloitus=drawAlbumi(albumit[0]);
  aloitus();
}

var xhttp_json = new XMLHttpRequest();
xhttp_json.onreadystatechange = function () {
  if (this.readyState == 4 && this.status == 200) {
    albumit = JSON.parse(this.responseText).albums;
    drawAlbumit();
  }
}
xhttp_json.open('GET', 'contents.json', true);
xhttp_json.send();


// Soitettavan kappaleen haku
var redrawKappale = function() {
  var kappaleElement = document.getElementById('kappale')
  if (kappaleElement.hasChildNodes()) {
    kappaleElement.removeChild(kappaleElement.childNodes[0]);
  }
  var root = document.createElement('div');
  kappale.split('\n').forEach(function (rivi) {
      root.appendChild(document.createTextNode(rivi));
      root.appendChild(document.createElement('br'));
  })
  kappaleElement.appendChild(root);
}

var xhttp_kappale = new XMLHttpRequest()
xhttp_kappale.onreadystatechange = function () {
  if (this.readyState == 4 && this.status == 200) {
    kappale = this.responseText;
    redrawKappale();
  }
}

var kappaleNaytto = function () {
  xhttp_kappale.open('GET', 'kappale.txt', true);
  xhttp_kappale.send();
}

kappaleNaytto();
setInterval(kappaleNaytto, 10000);


// Listan kappaleiden haku
var drawAlbumi = function(lista) {
  return (function () {
    var ul = document.createElement('ul');
    lista.songs.forEach(function (song) {
      var li = document.createElement('li');
      ul.appendChild(li);
      var node = document.createElement('button');
      node.setAttribute('class', 'kappalebtn');
      if (lista.name==="radio") {
        node.innerHTML=song.title+' - '+song.year;
      } else {
        node.innerHTML=song.title;
      }
      node.addEventListener('click', soitaKappale(song.id));
      li.appendChild(node);
    })

    var albumiElement = document.getElementById('albumi');
    if (albumiElement.hasChildNodes()) {
      albumiElement.removeChild(albumiElement.childNodes[0]);
    }
    var root = document.createElement('div');
    var node = document.createElement('button');
    node.setAttribute('class', 'listabtn');
    node.innerHTML=lista.name;
    node.addEventListener('click', soitaLista(lista.id));
    root.appendChild(node);
    root.appendChild(ul);
    albumiElement.appendChild(root);
  })
}


// Radiokanavan valinta
var drawRadiokanava = function() {
  var kappaleElement = document.getElementById('radiokanavat');
  if (kappaleElement.hasChildNodes()) {
    kappaleElement.removeChild(kappaleElement.childNodes[0]);
  }
  var ul = document.createElement('ul');
  var id = 0;
  radiokanavat.split('\n').forEach(function (rivi) {
    if (!rivi.includes('http') && !rivi=='') {
      var li = document.createElement('li');
      ul.appendChild(li);
      var node = document.createElement('button');
      node.setAttribute('class', 'listabtn');
      node.innerHTML=rivi;
      node.addEventListener('click', soitaRadiokanava(id));
      id = id+1;
      li.appendChild(node);
      li.appendChild(document.createElement('br'));
    }
  })
  kappaleElement.appendChild(ul);
}

var xhttp_radio = new XMLHttpRequest()
xhttp_radio.onreadystatechange = function () {
  if (this.readyState == 4 && this.status == 200) {
    radiokanavat = this.responseText;
    drawRadiokanava();
  }
}

xhttp_radio.open('GET', 'radiokanavat.txt', true);
xhttp_radio.send();


// Volume
var volumeSlider = document.getElementById('vslaideri');
volumeSlider.oninput = function() {
  var xhttp = new XMLHttpRequest();
  xhttp.open('POST', 'kasky.php', true);
  xhttp.setRequestHeader('Content-type', 'application/x-www-form-urlencoded');
  xhttp.send('toiminto=volume&id='+this.value);
}
