/*
Controle do forno LAFAC
Gustavo Voltani von Atzingen - 08/09/2016
*/

#ifndef ControleForno_h
#define ControleForno_h

#include "Arduino.h"

class ControleForno
{
    public:
        ControleForno();
        void teste_resistencias();
        void eventoEsteira();
        void loopTimer();
        void leituraSerial(char chr);
        void PWMsalvaEEPROM(long valor);
        long PWMleituraEEPROM();
        long getPeriodoPwd();
        void setPeriodoPwm(int v);
        void ADCsalvaEEPROM(int delayMs, int nLeituras);
        int DelayADCleituraEEPROM();
        int nLeituraADCleituraEEPROM();
        void controlaEsteira(int v);
        String leituraAnalogica();
        String tradaDado(String dado);
        boolean verificaNumerico(char chr);
        boolean verificaNumerico(String str);
        int getIndiceResitencia(char chr);
        int getPinSensor(int i);
        int getPinResistencia(int i);
        String retornaInfo();
        void setPinResistencia(int pinR1=3, int pinR2=5,int pinR3=2,int pinR4=6,int pinR5=4,int pinR6=7);
        void setPinEsteira(int pinEstEnable=12, int pinEstPwm=11, int pinEstSentido=10);
        void setPinSensores(int pinS1=4, int pinS2=3, int pinS3=5, int pinS4=0, int pinS5=2, int pinS6=1);
        void setLeituraAnalog(int delayAnalog = 5, int nLeituras = 10);
        void setProtocoloSerial(
            String STR_pedidoTemperaturas = "ST",
            String STR_emergencia = "SE",
            String STR_inicioDado = "0001",
            char CHR_inicioDado = 'S',
            char CHR_fimDado = '\n',
            char CHR_ligaForno = '1',
            char CHR_desligaForno = '2',
            char CHR_setPotenciaPWM = 'P',
            char CHR_esteiraFrente = 'H',
            char CHR_esteiraTras = 'A',
            char CHR_esteiraParada = 'D',
            char CHR_tempoPWM = 'U',
            char CHR_check = 'K',
            char CHR_setADC = 'L'
         );
        int velocidadeEsteira = 0;
        struct resistencias{
            boolean pwmLigado;
            int potencia;
            boolean ligado;
        };
        resistencias estadoResistencias[6] = {
            {false, 0, false},
            {false, 0, false},
            {false, 0, false},
            {false, 0, false},
            {false, 0, false},
            {false, 0, false}};
        boolean DEBUG = false;

    private:
        unsigned long _tempoAnterior = 0;

        int _delayAnalog;
        int _nLeituras;

        int _pinSensor[6];
        int _pinResistencia[6];

        int _pinEstEnable;
        int _pinEstPwm;
        int _pinEstSentido;
        long _PeriodoPwd;
        int _contadorPwm;

        String _STR_pedidoTemperaturas;
        String _STR_inicioDado;
        String _STR_emergencia;
        char _CHR_inicioDado;
        char _CHR_fimDado;
        char _CHR_ligaForno;
        char _CHR_desligaForno;
        char _CHR_setPotenciaPWM;
        char _CHR_esteiraFrente;
        char _CHR_esteiraTras;
        char _CHR_esteiraParada;
        char _CHR_tempoPWM;
        char _CHR_check;
        char _CHR_setADC;

        String _dadosBuffer = "";
};

#endif
