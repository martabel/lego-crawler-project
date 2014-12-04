lego*crawler*project
====================

The Lego 4x4 Crawler car with some cool extensions

_English_

TODO

_German_

##Betrieb:
  * Via 9V Lego Batterie (6*1,5V), An/Aus am Batterie Block
    * Wechsel des Blocks durch Aufmachen des Autos
    * Lösen von zwei grauen Legosteckern hinter der Tür
    * Trennen der Frontbeleuchtung, Stecker ist vorne vorhanden
  * Optional, Microcontroller und LED Versorgung über USB/5V Hohlstecker
    * Stromversorgung über 9v Block muss getrennt sein bevor über USB/5V betrieben wird
##Beleuchtung:
  * 4*Abblendlicht(vorne+hinten)
  * 2*Aufblendlicht
  * 4*Blinker
  * 2*Bremse
  * 1*Rückfahrlicht
  * 4*Innenraumlicht
  * 2*Dachstrahler
##Helligkeitsensor:
  * 1*Photodiode auf dem Dach
##Motorsensorik:
  * 4*Spannungsteiler von 9V auf 5V für digitalen input
  * Vor/Zurück/Links/Rechts

##Controller:
  * Arduino Mega 2560
  * CAN*Bus Shield by Seeed Studio v1.0 03/06/2013 http://www.seeedstudio.com/wiki/CAN*BUS_Shield
  * LED, Sensor Platine, siehe Schaltplan
##Software:
  * Implementierung einer ECU nach einem CAN Katalog
  * Siehe CAN Katalog für eine Beschreibung der CAN Nachrichten TODO