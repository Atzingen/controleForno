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
  forno.teste_resistencias();
  Serial.println(forno.retornaInfo());
}

void loop()
{
  forno.loopTimer();
  if ( Serial.available() > 0 )
  {
    forno.leituraSerial(Serial.read());
  }
}
