# SK2 - Projekt
Aplikacja typu klient-serwer pozwalająca na pobieranie i zapis plików.

## Kompilacja 
Serwer: 
gcc -Wall SeverHTTP.c -o ServerHTTP.out -lpthread  

## Użycie
Serwer działa na porcie 12345. 
W przypadku lokalnego uruchomienia skorzystać można z adresu loopback tj. 127.0.0.1. 
Wynikowy URL: http://127.0.0.1:12345
W repoztyrium załączone są dwa pliki: sample i index. 
Dostęp do nich za pomocą przeglądarki / Postmana: http://127.0.0.1:12345/[sample||index]. 


