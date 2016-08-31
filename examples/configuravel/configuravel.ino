/*
	Controle do forno LAFAC
	Gustavo Voltani von Atzingen - 08/09/2016
*/
// Bibliotecas
#include <EEPROM.h>
#include "ControleForno.h"

ControleForno forno;

void setup()
{
    Serial.begin(9600);
    forno.setPeriodoPwm(50);
    forno.setPinResistencia(3,4,5,6,7,8);
    forno.setPinEsteira(9,10,11);
    forno.setPinSensores(0,1,2,3,4,5);
    forno.setLeituraAnalog(2,8);
    forno.setProtocoloSerial("ST","666",'I','\n','1','2','P','H','A','D','U');
}

void loop()
{
  forno.loopTimer();
  if ( Serial.available() > 0 )
  {
    forno.leituraSerial(Serial.read());
  }
}
