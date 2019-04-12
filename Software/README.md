# Software Funktionsgenerator

In diesem Verzeichnis ist die Firmware für den Funktionsgenerator zu finden. Es handelt sich 
um ein Projekt für die IDE [Eclipse](https://www.eclipse.org/ide/) mit dem Zusatzplugin 
[System Workbench for STM32](http://www.openstm32.org/System%2BWorkbench%2Bfor%2BSTM32). 


## Dokumentation

Für die Dokumentation des Codes bitte in der Projektdokumentation im Verzeichnis `Dokumentation` nachlesen.


## How to build

Nach dem Import in Eclipse sind zwei Konfigurationen vorhanden: `Debug` und `Release`. 

Ein `Debug` Build ermöglicht das Debuggen mittels GDB, wobei keinerlei Code-Optimierungen erfolgen.

Ein Build in der `Release` Konfiguration optimiert den Code mit `-Os`. Damit wird der Code um einiges
schneller und die Binary-Size deutlich kleiner, es kann jedoch nicht mehr gedebugged werden. 
