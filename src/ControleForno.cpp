#include "Arduino.h"
#include "ControleForno.h"
#include <EEPROM.h>

ControleForno::ControleForno()
{
    setPinResistencia();
    setPinEsteira();
    setPinSensores();
    setLeituraAnalog();
    int t = leituraEEPROM();
    setPeriodoPwm(t);
    Serial.println(t);
    setProtocoloSerial();
}

void ControleForno::teste_resistencias()
{
    for (int i=0;i<6;i++)
    {
        digitalWrite(_pinResistencia[i],HIGH);
        delay(300);
        digitalWrite(_pinResistencia[i],LOW);
        delay(300);
    }
}

void ControleForno::loopTimer()
{
    if ( millis() - _tempoAnterior > 10*_PeriodoPwd )
    {
        _tempoAnterior = millis();
        eventoEsteira();
    }
}

void ControleForno::leituraSerial(char chr)
{
    /*
        Função que é chamada toda vez que um caracter chega pela serial e
        o caracter que chega é passado como parâmetro para esta função.
        Caso a comunicação esteja terminada (com caracter de fim de dado),
        é feito um tratamento no conjunto de dados recebido (que fica salvo
        na String _dadosBuffer).
        Caso o pedido da unidade mestre não esteja terminada (nao seja o caracter
        final), a String _dadosBuffer é incrementada.
    */
    if ( chr == _CHR_fimDado)   // Caso chegue um caracter '\n' (fim de linha)
    {                           // Descobrir que tipo de pedido chegou:
        ///////////////////////////////////////////////////////////////////////////////////
		// Caso ST, devolver os valores de temperatura dos 6 sensores
		if ( _dadosBuffer == _STR_pedidoTemperaturas )
        {
			_dadosBuffer = "";	// Apaga o valor da variavel dados para usa-la para devolver os valores de temperatura pela serial
            _dadosBuffer += _CHR_inicioDado;
            _dadosBuffer += _STR_inicioDado;
            // Inicia com S0001 (padrão para manter consistência com a comunicação ubee - 0001 = primeiro módulo)
            for (int i=0; i<6; i++)     // Loop para passar por todos os sensores
            {
				_dadosBuffer += leituraAnalogica(_pinSensor[i]);
        	}
			Serial.println(_dadosBuffer);	// Envia pela serial os dados da temperatura
		}
        ///////////////////////////////////////////////////////////////////////////////////
		// Caso S'x''y', com x de 2 a 7 (as 6 resistências) e y 1 ou 2 (ligar ou desligar)
		else if ( _dadosBuffer.charAt(0) == _CHR_inicioDado &&
        getIndiceResitencia(_dadosBuffer.charAt(1)) != -1  &&
        (_dadosBuffer.charAt(2) == _CHR_ligaForno ||
        _dadosBuffer.charAt(2) == _CHR_desligaForno) )
        {
            // Liga ou desliga o respectivo pino digital da resitência do forno
			digitalWrite(int(_dadosBuffer.charAt(1)) - 48, _dadosBuffer.charAt(2) == _CHR_ligaForno);
            int i = getIndiceResitencia(_dadosBuffer.charAt(1));
            ResistenciaPWM[i].ligado   = false;
			Serial.println(_dadosBuffer);// Devolve o pedido recebido para verificação
		}
        ///////////////////////////////////////////////////////////////////////////////////
		// Caso SP'x''yy', com x de 2 a 7 (as 6 resistências) e y a potência de 1 a 99 (1% a 99%)
		else if ( _dadosBuffer.charAt(0) == _CHR_inicioDado &&
        _dadosBuffer.charAt(1) == _CHR_setPotenciaPWM &&
        getIndiceResitencia(_dadosBuffer.charAt(2)) != -1 &&
        verificaNumerico(_dadosBuffer.charAt(3)) )
        {
            String dados_new = _dadosBuffer;
            dados_new.remove(0,3);
            int a = dados_new.toInt();
            if ( a >= 0  && a < 101 && verificaNumerico(dados_new) )
            {
                int i = getIndiceResitencia(_dadosBuffer.charAt(2));
                ResistenciaPWM[i].ligado   = true;
                ResistenciaPWM[i].potencia = a;
                Serial.println(_dadosBuffer); // Devolve o pedido recebido para verificação
            }
            else
                Serial.println("Erro - Comando recebido mas nao identificado");
		}
		/////////////////////////////////////////////////////////////////////////////////
		// Caso SH'xx', SA'xx' ou SD - Esteira, para frente, tras ou parada
		else if ( _dadosBuffer.charAt(0) == _CHR_inicioDado &&
        (_dadosBuffer.charAt(1) == _CHR_esteiraFrente || _dadosBuffer.charAt(1) == _CHR_esteiraTras || _dadosBuffer.charAt(1) == _CHR_esteiraParada))
        {
			String dados_new = _dadosBuffer;
            // Quando Comando SD
			if (_dadosBuffer.charAt(1) == _CHR_esteiraParada && _dadosBuffer.length() == 2 )
            {
				digitalWrite(_pinEstSentido, LOW);  // Comando para parar a esteira
				digitalWrite(_pinEstPwm, LOW);
				digitalWrite(_pinEstEnable, LOW);   // Esteira Disable
			}
            // Quando Comando SH'xx', com x = numero inteiro 1-100
			else if (_dadosBuffer.charAt(1) == _CHR_esteiraFrente )
            {
				dados_new.remove(0,2);
				if ( dados_new.toInt() >= 0 &&
                dados_new.toInt() < 101  &&
                verificaNumerico(dados_new) )
                {
                	digitalWrite(_pinEstSentido, LOW);// Sentido da esteira
					digitalWrite(_pinEstEnable, HIGH);// Esteira Enable
					analogWrite(_pinEstPwm, map(dados_new.toInt(),100,0,0,255));// Potência da eteira (pwm)
				}
				else
					Serial.println("Erro - SH_H_D'xx xx nao numerico");
			}
            // Quando Comando SA'xx', com x = numero inteiro 1-100
			else if (_dadosBuffer.charAt(1) == _CHR_esteiraTras )
            {
				dados_new.remove(0,2);
                if ( dados_new.toInt() >= 0 &&
                dados_new.toInt() < 101 &&
                verificaNumerico(dados_new) )
                {
					digitalWrite(_pinEstSentido, HIGH);	   // Sentido da Esteira
					digitalWrite(_pinEstEnable, HIGH);     // Esteira Enable
					analogWrite(_pinEstPwm, map(dados_new.toInt(),100,0,0,255));// Potência da esteira (pwm)
				}
				else
					Serial.println("Erro - SH_A_D'xx xx nao numerico");
			}
			else
				Serial.println("Erro - SH_A_D'xx");
			Serial.println(_dadosBuffer);	// Devolve o pedido recebido para verificação
		}
        ///////////////////////////////////////////////////////////////////////////////////
		// Caso SU'xxx' - Alterar o periodo do pwm
		else if ( _dadosBuffer.charAt(0) == _CHR_inicioDado &&
        _dadosBuffer.charAt(1) == _CHR_tempoPWM)
        {
            String dados_new = _dadosBuffer;
            dados_new.remove(0,2);
			if ( dados_new.toInt() > 10 )
            {
				long tempo_pwd = dados_new.toInt();
				salvaEEPROM(tempo_pwd);
                _PeriodoPwd = tempo_pwd;
			}
            Serial.println(_dadosBuffer);	// Devolve o pedido recebido para verificação
		}
		else // Erro
			Serial.println("Erro - Comando recebido mas nao identificado");
		_dadosBuffer = "";	// Apaga os valores da String dados
	}
	else   // Caso não chegue um caracter '\n' (fim de linha)
		_dadosBuffer += String(chr);  // Concatena o caracter lido pela serial a String dados
}

void ControleForno::eventoEsteira()
{
    /*
    Funcao que é controla o pwm manual da esteira.
    Esta função deve ser chamada t/100 segundos, com t o período do pwm manual
    */
    for (int i=0;i<6;i++)
    {
        if ( ResistenciaPWM[i].ligado ) // Checa se a resistência esta marcada
        {                               // para atuar como pwm
            digitalWrite(_pinResistencia[i], (ResistenciaPWM[i].potencia >= _contadorPwm));
            // Liga ou desliga a resitência 1 de acordo com o resultado
            // da comparação entre a potência da resistência 1 e o contador,
        }
    }
    if ( _contadorPwm > 99 ) // Contador do pwm manual
        _contadorPwm = 0;
    else
        _contadorPwm++;
}

void ControleForno::salvaEEPROM(long v)
{
    /*
    Recebe o valor v referente ao tempo do periodo pwm
    e salva na eeprom para persistência desta informação.
    O valor é decomposto e 4 partes de 8 bits e salvo nos
    endereços 1, 2, 3 e 4 da memória.
    */
    byte quatro = (v & 0xFF);
	byte tres = ((v >> 8) & 0xFF);
	byte dois = ((v >> 16) & 0xFF);
	byte um = ((v >> 24) & 0xFF);
	Serial.print("EEPROM_salva: ");
	Serial.println(v);
	EEPROM.write(1, quatro);
	EEPROM.write(2, tres);
	EEPROM.write(3, dois);
	EEPROM.write(4, um);
}

long ControleForno::leituraEEPROM()
{
    /*
    Função que recupera o valor (tempo) do período de pwm que
    está salvo na eeprom.
    */
    long quatro = EEPROM.read(1);
	long tres = EEPROM.read(2);
	long dois = EEPROM.read(3);
	long um = EEPROM.read(4);
	Serial.print("EEPROM_leitura: ");
	Serial.println(((quatro << 0) & 0xFF) + ((tres << 8) & 0xFFFF) + ((dois << 16) & 0xFFFFFF) + ((um << 24) & 0xFFFFFFFF));
	return ((quatro << 0) & 0xFF) + ((tres << 8) & 0xFFFF) + ((dois << 16) & 0xFFFFFF) + ((um << 24) & 0xFFFFFFFF);
}

boolean ControleForno::verificaNumerico(char chr)
{
    /*  OVERLOAD com String str
        verifica se o caracter passado é numérico 0..9
    */
    return (chr > 47 && chr < 58);
}

boolean ControleForno::verificaNumerico(String str)
{
    /*  OVERLOAD com char chr
        Verifica se a String(objeto) passado é numérica
        ou se possui algum caracter não numérico (de 0..9)
    */
    for (int i=0; i<str.length(); i++)
    {
        char c = str.charAt(i);
        if ( !verificaNumerico(c) )
            return false;
    }
    return true;
}

int ControleForno::getIndiceResitencia(char chr)
{
    /*
        Retorna o indice do vetor de pinos da resistencia
        ( _pinResistencia ) pertence o numero do pino que é passado
        como parâmetro da função em forma de caracter.
        Caso o núero do pino não esteja conectado a uma resistência,
        é retornado o valor -1.
    */
    for (int i=0;i<6;i++)
    {
        if ( _pinResistencia[i] == int(chr) - 48 )
            return i;
    }
    return -1;
}

String ControleForno::leituraAnalogica(int pin)
{
	/* 	Essa função lê o valor no ACD especificado (variável analog) _nLeituras vezes retorna a média.
	   	O Resultado é devolvido com 4 digitos, sendo que zeros são adicionados caso
        o valor seja menor que mil. ADC de 10 bits -> resultado de 0 a 1023
	   	O resultado é concatenado na variável dado e retornado.
	*/
	int leitura = 0;														// Variável para receber o valor da leitura
	for (int i=0;i<_nLeituras;i++)                                          // Repete 10 vezes o procedimento para efetuar a média
    {
		leitura += analogRead(pin);										    // Leitura do valor do ADC
		delay(_delayAnalog);												// delay ?
	}
	leitura = leitura/10;													// Divisão inteira por 10 (foram somadas 10 leituras)
	String a = "";															// String que receberá o valor da leitura com 4 digitos
	if ( leitura < 10 ){													// Completa com digitos zeros para completar 4 digitos
		a += "000";															//  de acordo com o numero inteiro 'leitura'
	}																		// 3 zeros quando 'leitura' tem 1 digito
	else if ( leitura < 100 ){
		a += "00"; 															// 2 zeros quando 'leitura' tem 2 digito
	}
	else if ( leitura < 1000 ){
		a += "0"; 															// 1 zero  quando 'leitura' tem 3 digito
	}
	a += String(leitura);
	return a;															   // Retorna o valor
}

int ControleForno::getPinSensor(int i)
{
    return _pinSensor[i];
}

int ControleForno::getPinResistencia(int i)
{
    return _pinResistencia[i];
}

long ControleForno::getPeriodoPwd()
{
    return _PeriodoPwd;
}

void ControleForno::setProtocoloSerial(String STR_pedidoTemperaturas, String STR_inicioDado,
char CHR_inicioDado,char CHR_fimDado,char CHR_ligaForno, char CHR_desligaForno, char CHR_setPotenciaPWM, char CHR_esteiraFrente,
char CHR_esteiraTras, char CHR_esteiraParada,
char CHR_tempoPWM)
{
    _STR_pedidoTemperaturas = STR_pedidoTemperaturas;
    _STR_inicioDado = STR_inicioDado;
    _CHR_inicioDado = CHR_inicioDado;
    _CHR_fimDado = CHR_fimDado;
    _CHR_ligaForno = CHR_ligaForno;
    _CHR_desligaForno = CHR_desligaForno;
    _CHR_setPotenciaPWM = CHR_setPotenciaPWM;
    _CHR_esteiraFrente = CHR_esteiraFrente;
    _CHR_esteiraTras = CHR_esteiraTras;
    _CHR_esteiraParada = CHR_esteiraParada;
    _CHR_tempoPWM = CHR_tempoPWM;
}

void ControleForno::setLeituraAnalog(int delayAnalog, int nLeituras)
{
    _delayAnalog = delayAnalog;
    _nLeituras = nLeituras;
}

void ControleForno::setPeriodoPwm(int t)
{
    if ( t < 10 )//salvaEEPROM(100);
        t = 100;
    _PeriodoPwd = t;
}

void ControleForno::setPinSensores(int pinS1, int pinS2,int pinS3,int pinS4,int pinS5,int pinS6)
{
    int pinSensor[6] = {pinS1,pinS2,pinS3,pinS4,pinS5,pinS6};
    for (int i=0;i<6;i++)
    {
        _pinSensor[i] = pinSensor[i];
    }
}

void ControleForno::setPinResistencia(int pinR1, int pinR2,int pinR3,int pinR4,int pinR5,int pinR6)
{
    int pinResistencia[6] = {pinR1,pinR2,pinR3,pinR4,pinR5,pinR6};
    for (int i=0;i<6;i++)
    {
        _pinResistencia[i] = pinResistencia[i];
        pinMode(_pinResistencia[i], OUTPUT);
    }
}

void ControleForno::setPinEsteira(int pinEstEnable, int pinEstPwm, int pinEstSentido)
{
    _pinEstEnable = pinEstEnable;
    _pinEstPwm = pinEstPwm;
    _pinEstSentido = pinEstSentido;
    pinMode(_pinEstEnable, OUTPUT);
    pinMode(_pinEstPwm, OUTPUT);
    pinMode(_pinEstSentido, OUTPUT);
}
