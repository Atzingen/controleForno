# controleForno

Biblioteca do Arduino para o controle Automatizado de um Forno típo Túnel. 
O sistema de controle é baseado em duas partes, a unidade mestre que tem a interface de controle do usuário e realiza os calculos numéricos para simulação e controle automático.
A parte embarcada no forno (escravo) é controlada por um atmega328p-pu programado via arduino com esta biblioteca e controla as resistências, esteira e sensores. As unidades se comunicam via bluetooth.

Parte de trabalho realizado para conclusão do Doutorado em Engenharia de Alimento. FZEA USP

Gustavo Voltani von Atzingen
gustavo.von.atzingen@gmail.com

This project is licensed under the terms of the MIT license.
