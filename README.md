# SK2 - Projekt
Aplikacja typu klient-serwer pozwalająca na pobieranie i zapis plików.

## Kompilacja 
Serwer: 
gcc -Wall SeverHTTP.c -o ServerHTTP.out -lpthread  

Klient: 
C:\Qt\5.12.6\mingw73_64\bin\qmake.exe -o Makefile ..\ClientHTTP\ClientHTTP.pro -spec win32-g++ "CONFIG+=debug" "CONFIG+=qml_debug"
