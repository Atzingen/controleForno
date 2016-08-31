/*
	Controle do forno LAFAC
	Gustavo Voltani von Atzingen - 13/12/2015 - 30/07/2016
*/
// Bibliotecas
#include <EEPROM.h>
// Declaração das variáveis globais
String dados = "";						// Variável para aquisição de dados via serial
unsigned long tic = 0;					// Variável do tempo de execução do programa para o pwm manual
int contador = 0;						// Contador para uso no pwm manual
long tempo_pwd;
struct pwm_manual{						// Estrutura para informações do pwm manual nas 6 resitências
	boolean ligado;
	int potencia;
};
pwm_manual resitencia1 = {false, 0};	// Inicia a struct das resistências como desligadas e sem potência
pwm_manual resitencia2 = {false, 0};	// Idem resistências 2 à 6
pwm_manual resitencia3 = {false, 0};	// Idem
pwm_manual resitencia4 = {false, 0};	// Idem
pwm_manual resitencia5 = {false, 0};	// Idem
pwm_manual resitencia6 = {false, 0};	// Idem

void setup(){							// Setup - Executa uma vez ao iniciar o microcontrolador
	Serial.begin(9600);					// Inicia a Comunicação Serial
	for (int i=2; i<8; i++){
		pinMode(i, OUTPUT);				// Configura o pino i (de 2 a 7) como saida digital (Resistências)
		digitalWrite(i, LOW);			// Inicia a tensão de saída do pino como 0
	}
	pinMode(10, OUTPUT);				// Pino de controle de sentido do forno (0 e 1 frente e tras ?)
	pinMode(11, OUTPUT);				// Pino de controle da potência da esteira (pwm)
	pinMode(12, OUTPUT);				// Pino de Enable
	digitalWrite(10, LOW);				// Inicia com a esteira parada
	digitalWrite(11, LOW);				// Inicialmente a esteira esta com potencia 0 (parada)
	digitalWrite(12, HIGH);				// Idem
	tempo_pwd = EEPROM_leitura();
	if (tempo_pwd < 10 ){
		tempo_pwd = 100;
		EEPROM_salva(tempo_pwd);
	}
}

void loop(){							// Loop do programa principal
	// Condição que será verdadeira a cada "tempo_pwd", em segundos
	if ( (millis() - tic) > 10*tempo_pwd ){  // 10*tempo_pwd : tempo_pwd em segundos % por 100
		//Serial.print(contador);								// Debug - Retirar depois de OK
		//Serial.println("tic");
		if ( resitencia1.ligado ){								// Checa se a resistência 1 esta marcada para atuar como pwm
			digitalWrite(2, (resitencia1.potencia >= contador));// Liga ou desliga a resitência 1 de acordo com o resultado
		}														// da comparação entre a potência da resistência 1 e o contador,
		if ( resitencia2.ligado ){								// ambos de 0 a 10.
			digitalWrite(3, (resitencia2.potencia >= contador));// Idem 2
		}
		if ( resitencia3.ligado ){
			digitalWrite(4, (resitencia3.potencia >= contador));// Idem  3
		}
		if ( resitencia4.ligado ){
			digitalWrite(5, (resitencia4.potencia >= contador));// Idem 4
		}
		if ( resitencia5.ligado ){
			digitalWrite(6, (resitencia5.potencia >= contador));// Idem 5
		}
		if ( resitencia6.ligado ){
			digitalWrite(7, (resitencia6.potencia >= contador));// Idem 6
		}
		if ( contador > 99 ){									// Caso o contador seja maior que 99 (100)
			contador = 0;										// Volta para zero e reinicia o processo 1, 2, ..., 99
		}
		else{
			contador += 1;										// Incrementa o contador quando <= 9
		}
		tic = millis();											// guarda o tempo para usar no if da condição de tempo novamente
	}

	// Parte que recebe e trada dados recebidos pela serial
	if ( Serial.available() > 0 ){								// Caso chegue algum dado pela serial:
		char chr = Serial.read();								// Lê o dado e salva na variavel chr
		if ( chr == '\n'){										// Caso chegue um caracter '\n' (fim de linha) - Descobrir que tipo de pedido chegou
			/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			// Caso ST, devolver os valores de temperatura dos 6 sensores
			if ( dados == "ST" ){
				dados = "";										// Apaga o valor da variavel dados para usa-la para devolver os valores de temperatura pela serial
	      dados += "S0001";								// Inicia com S0001 (padrão para manter consistência com ca comunicação ubee - 0001 = primeiro módulo)
	      for (int i=0; i<8; i++){						// Loop para passar por todos os sensores
					if (i < 4 || i > 5){
						dados = add_string(dados,i);			// Chama a função add_string que lerá um sensor retornará o valor na String dados
					}
	        	}
				Serial.println(dados);							// Envia pela serial os dados da temperatura
			}
			////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			// Caso S'x''y', com x de 2 a 7 (as 6 resistências) e y 1 ou 2 (ligar ou desligar)
			else if ( dados.charAt(0) == 'S' && dados.charAt(1) > 49 && dados.charAt(1) < 56  && (dados.charAt(2) == '1' || dados.charAt(2) == '2') ){
				int v;											// Variável auxiliar v
				if ( dados.charAt(2) == '1' )					// Final do dado Sx1 ou Sx2: 1 liga, 2 desliga
					v = 1;
				else
					v = 0;
				digitalWrite(int(dados.charAt(1)) - 48, v);		// Liga ou desliga o respectivo pino digital da resitência do forno
				switch (dados.charAt(1)){ 						// Switch para alterar a estrutura de controle 'resitencia' para ligada ou desligada
				    case '2':									// Altera para Liga ou desliga a estrutura 'resistencia' ( de acordo com o valor de 'v')
						resitencia1.ligado   = false;
						break;
					case '3':									// Idem para resitencia2
						resitencia2.ligado   = false;
						break;
					case '4':									// Idem para resitencia3
						resitencia3.ligado   = false;
						break;
					case '5':									// Idem para resitencia4
						resitencia4.ligado   = false;
						break;
					case '6':									// Idem para resitencia5
						resitencia5.ligado   = false;
						break;
					case '7':									// Idem para resitencia6
						resitencia6.ligado   = false;
						break;
				    default:
				      	Serial.println("Erro -  S'x''y'");
				}
				Serial.println(dados);							// Devolve o pedido recebido para verificação
			}
			////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			// Caso SP'x''yy', com x de 2 a 7 (as 6 resistências) e y a potência de 1 a 99 (1% a 99%)
			else if ( dados.charAt(0) == 'S' && dados.charAt(1) == 'P' &&  (dados.charAt(2) > 49 && dados.charAt(2) < 56) && (dados.charAt(3) > 47 && dados.charAt(3) < 58) ){
				int a = int(dados.charAt(3) - 48);
				if ((dados.length() == 5) && int(dados.charAt(3) - 48)){								// Caso o valor que da potência tenha dois digitos
					int b = int(dados.charAt(4) - 48);													// Converte o ultimo digito em inteiro
					a = b + 10*a;																		// 10 x dezena + 1 x unidade
				}
				switch (dados.charAt(2)){																// Switch para ver para qual resitência foi o comando
					case '2':
						resitencia1.ligado   = true;													// Marca a resitência um como ligada (pwm ligado)
						resitencia1.potencia = a;	// Marca a potência da resitência um
						break;
					case '3':																			// Idem para a resitência 2
						resitencia2.ligado = true;
						resitencia2.potencia = a;
						break;
					case '4':																			// Idem para a resitência 3
						resitencia3.ligado = true;
						resitencia3.potencia = a;
						break;
					case '5':																			// Idem para a resitência 4
						resitencia4.ligado = true;
						resitencia4.potencia = a;
						break;
					case '6':																			// Idem para a resitência 5
						resitencia5.ligado = true;
						resitencia5.potencia = a;
						break;
					case '7':																			// Idem para a resitência 6
						resitencia6.ligado = true;
						resitencia6.potencia = a;
						break;
					default:
				      	Serial.println("Erro - SP'x''y'");
				}
				Serial.println(dados);										// Devolve o pedido recebido para verificação
			}
			//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			// Caso SH'xx', SA'xx' ou SD - Esteira, para frente, tras ou parada
			else if ( dados.charAt(0) == 'S' && (dados.charAt(1) == 'H' || dados.charAt(1) == 'A' || dados.charAt(1) == 'D') ){
				String dados_new = dados;
				if (dados.charAt(1) == 'D' && dados.length() == 2 ){ 		// Quando Comando SD (tamanho da String = 2)
					digitalWrite(10, LOW);  								// Comando para parar a esteira
					digitalWrite(11, LOW);
					digitalWrite(12, HIGH);									// Esteira Disable
					//Serial.println("Comando para SD");					// Para a esteira
				}
				else if (dados.charAt(1) == 'H'){							// Quando Comando SH'xx', com x = numero inteiro 1-99
					dados_new.remove(0,2);									// Remove os dois primeiros caracteres (SH) para retirar o numero
					if ( testa_numero(dados_new) && dados_new.length()<4){	// Checa se o restante do comando são epenas números e até dois digitos
						digitalWrite(10, LOW);								// Sentido da esteira
						digitalWrite(12, LOW);								// Esteira Enable
						analogWrite(11, map(dados_new.toInt(),0,100,0,244));// Potência da eteira (pwm)
						//Serial.println(dados_new.toInt());				// Debug - remover
					}
					else{
						Serial.println("Erro - SH_H_D'xx xx nao numerico");
					}
				}
				else if (dados.charAt(1) == 'A'){							// Quando Comando SA'xx', com x = numero inteiro 1-99
					dados_new.remove(0,2);									// Remove os dois primeiros caracteres (SA) para retirar o numero
					if ( testa_numero(dados_new) && dados_new.length()<4){
						digitalWrite(10, HIGH);								// Sentido da Esteira
						digitalWrite(12, LOW);								// Esteira Enable
						analogWrite(11, map(dados_new.toInt(),0,100,0,244));// Potência da esteira (pwm)
						//Serial.println(dados_new.toInt());				// Debug - remover
					}
					else{
						Serial.println("Erro - SH_A_D'xx xx nao numerico");
					}
				}
				else{
					Serial.println("Erro - SH_A_D'xx");
				}
				Serial.println(dados);										// Devolve o pedido recebido para verificação
			}
			else if ( dados.charAt(0) == 'S' && dados.charAt(1) == 'U'){
				if ( (dados.charAt(2) > 47 && dados.charAt(2) < 58) && (dados.charAt(3) > 47 && dados.charAt(3) < 58) && (dados.charAt(4) > 47 && dados.charAt(4) < 58) ){
					int a = int(dados.charAt(2) - 48);
					int b = int(dados.charAt(3) - 48);
					int c = int(dados.charAt(4) - 48);
					tempo_pwd = 10*((10*a)  + b) + c;
					EEPROM_salva(tempo_pwd);
					Serial.print("ST");
					Serial.println(tempo_pwd);
				}
			}
			else{
				// Erro
				Serial.println("Erro - Comando recebido mas nao identificado");	// Debug - remover
			}
			dados = "";														// Apaga os valores da String dados
		}
		else{																// Caso não chegue um caracter '\n' (fim de linha)
			dados += String(chr);											// Concatena o caracter lido pela serial a String dados
		}
	}
}

long EEPROM_leitura(){
	long quatro = EEPROM.read(1);
	long tres = EEPROM.read(2);
	long dois = EEPROM.read(3);
	long um = EEPROM.read(4);
	Serial.print("EEPROM_leitura: ");
	Serial.println(((quatro << 0) & 0xFF) + ((tres << 8) & 0xFFFF) + ((dois << 16) & 0xFFFFFF) + ((um << 24) & 0xFFFFFFFF));
	return ((quatro << 0) & 0xFF) + ((tres << 8) & 0xFFFF) + ((dois << 16) & 0xFFFFFF) + ((um << 24) & 0xFFFFFFFF);
}

void EEPROM_salva(long v){
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

String add_string(String dado, int analog){
	/* 	Essa função lê o valor no ACD especificado (variável analog) 10 vezes retorna a média.
	   	O Resultado é devolvido com 4 digitos, sendo que zeros são adicionados caso o valor seja
	   	menor que mil. ADC de 10 bits -> resultado de 0 a 1023
	   	O resultado é concatenado na variável dado e retornado.
	*/
	int leitura = 0;														// Variável para receber o valor da leitura
	for (int i=0;i<10;i++){													// Repete 10 vezes o procedimento para efetuar a média
		leitura += analogRead(analog);										// Leitura do valor do ADC
																			// delay ?
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
	dado += a;																// Concatena o valor da leitura aos zeros adicionados
	return dado;															// Retorna o valor
}

boolean testa_numero(String texto) {										// Função que chega se a String é numérica (apenas digitos de 0 a 9)
    for(char i = 0; i < texto.length(); i++) {								// For para passar por todos os caracteres da String
        if (!isDigit(texto.charAt(i))) {									// Testa se o caracter na posição i da string é ou não um digito 0 a 9
            return false;													// Retorna Falso caso a condição da linha acima (if) seja falsa
        }
    }
    return true;															// Retorna Verdadeiro caso tenha passado por todos os elementos da String sem
}																			//   entrar na condição do if
