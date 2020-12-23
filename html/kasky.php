<?php

if ($_POST["toiminto"]=="lista") {
  $handle=fopen("/var/www/html/soita_lista.txt", "w");
  fwrite($handle,$_POST["id"]);
  fclose($handle);
}

if ($_POST["toiminto"]=="kappale") {
  $handle=fopen("/var/www/html/soita_kappale.txt", "w");
  fwrite($handle,$_POST["id"]);
  fclose($handle);
}

if ($_POST["toiminto"]=="radiokanava") {
  $handle=fopen("/var/www/html/soita_radiokanava.txt", "w");
  fwrite($handle,$_POST["id"]);
  fclose($handle);
}

if ($_POST["toiminto"]=="volume") {
  $handle=fopen("/var/www/html/aseta_volume.txt", "w");
  fwrite($handle,$_POST["id"]);
  fclose($handle);
}


?>
