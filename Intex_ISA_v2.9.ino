/*     INFORMAÇÕES SOBRE O SISTEMA:
        Firmware desenvolvido por Fábio Colella para aplicação no MDP ISA versão 2.9 da INTEX AGRO
        O HARDWARE FOI COMPOSTO PELOS SEGUINTES COMPONENTES E ATUADORES APLICADOS:
        - Bateria lítio li-ion 3S 12V 4400mAH
        - PCB MDP ISA v2.9
        - Controlador AVR Atmega328p integrado em plataforma (Arduino Nano)
        - Regulador de Tensão Dc/Dc Ajustável Step Down - LM2596
        - Sensor de Corrente Elétrica - MAX-GY471
        - Sensor de Voltagem por Divisores de Tensão com Resistores
        - Sensor de Ambiente (Umidade/Temperatura/Pressão) - BME280
        - Módulo Bluetooth 2.0 UART RS232 - HC-06
        - Módulo Comunicação Rádio 2.4Ghz - NRF24L01
        - Driver Ponte-H - BTS7960
        - Módulo Encoder Óptico (Função Controle do Motor) - LM393
        - 02x Chaves ópticas (Função Fim de Curso) - ITR9608
        - Motor sem escova tipo mabuchi sem redutor - JC-578VA
        - Botão On/Off e Botão Reset
        - Led Bicolor (Verde/Vermelho)
        - Buzzer Passivo
        */

// --- BIBLIOTECAS AUXILIARES --- //
  #include <SoftwareSerial.h> // Habilita porta serial extra para comunicação com Bluetooth 
  #include <Wire.h>           // Habilita porta I2C para comunicação do sensor de ambiente
  #include <BME280I2C.h>      // Biblioteca de parâmetros do sensor de ambiente

// --- MAPEAMENTO DO HARDWARE --- //
  #define porta_contador 2        // define porta do contador
  #define porta_fimdecurso_A 4    // define porta digital fim de curso A
  #define porta_fimdecurso_B 3    // define porta digital fim de curso B
  #define porta_motor_esq 5       // define porta PWM para sentido CW do motor_carro
  #define porta_motor_dir 6       // define porta PWM para sentido CCW do motor_carro
  #define Bluetooth_TX 7          // define porta de transmissão de bytes (TX) do Bluetooth 
  #define Bluetooth_RX 8          // define porta de recepção de bytes (RX) do Bluetooth
  #define Wifi_CE 9               // define porta Enable NRF24L01
  #define Wifi_CE 10              // define porta Select NRF24L01 
  #define porta_buzzer 14         // define porta speaker (Equivalente A0)
  #define porta_led_verde 15      // define porta Led verde (Equivalente A1)
  #define porta_led_vermelho 16   // define porta Led Vermelho (Equivalente A2)
  #define porta_tensao A3         // define porta Analógica do Sensor de Tensão (Divisor de Tensão)
  #define porta_corrente A6       // define porta Analógica do Sensor de Corrente MAX-GY471
  // Sensor de Temperatura e Umidade BME280 comunicando através do protocolo I2C em A4=SDA / A5=SCL
  // Modulo NRF24L01 comunicando através do protocolo SPI - D11=MOSI / D12=MISO / D13=SCK

// --- VARIAVEIS DE CONTROLE DO TEMPO --- //
  unsigned long currentMillis;
  unsigned long previousMillis = 0;
  unsigned long tempo_inicio;
  unsigned long tempo_fim;
  const long intervalo0 = 500;
  const long intervalo1 = 1000;
  const long intervalo5 = 5000;
  const long intervalo10 = 10000;
  const long intervalo50 = 50000;

// --- VARIAVEIS DO SENSOR DE AMBIENTE --- //
  BME280I2C Bme;              // Cria Estância do Sensor de Ambiente
  int temperatura;
  int umidade;

// --- VARIAVEIS DO MOTOR --- //  
  int slowmotor = 150;
  int fastmotor = 230;

// --- VARIAVEIS DO CONTADOR --- //
  bool estado = false;
  bool estadoAnt = false;
  unsigned long count;
  unsigned long count_1;
  unsigned long count_2;
  int distancia = 17;
  int consumo = 1;

// --- VARIAVEIS DO BLUETOOTH --- //
  SoftwareSerial Bluetooth(Bluetooth_RX,Bluetooth_TX);
  char Bluetooth_dados;
  char Bluetooth_info;

// --- VARIAVEIS DOS LEDS --- //
  unsigned long atualMillis;
  unsigned long anteriorMillis = 0;
  const long tempo_pisca_fast = 150;
  int statusLED = LOW;
  
// --- VARIAVEIS DOS SENSORES DE CORRENTE E DE TENSÃO --- //
  #define vmax 15.0
  int medicoes;
  int totalMedicoes;
  int consumo_mA_medio;
  int consumo_mAH;
  float valorMedicoes;
  float tensao_inicio;
  float carga_inicio;
  float carga_bat;
  float corrente;
  float corrente_media;
  float tensao;
  float volt;
  float carga;
  unsigned long MillisAtual;
  unsigned long MillisAnterior = 0;

// --- VARIAVEIS DAS FUNCOES --- //
  void ligaLED_verde();
  void desligaLED_verde();
  void piscaLED_verde();
  void ligaLED_vermelho();
  void desligaLED_vermelho();
  void piscaLED_vermelho();
  void piscaLED_vermelho_fast();
  void motorOFF();
  void motorESQ();
  void motorDIR();
  void motorESQ_lento();
  void motorDIR_lento();
  void ambiente();
  void contador();
  void teste_FC();
  void buzzer_beep();
  void buzzer_sinal();
  void buzzer_sucess();
  void tensao_monitor();
  void bateria_monitor();
  void corrente_monitor();
  void CORRENTE_CONSTANTE();
  void CORRENTE_CONSTANTE_MONITOR();

// --- SETUP --- //
  void setup() {
  pinMode(porta_led_verde, OUTPUT);
  pinMode(porta_led_vermelho, OUTPUT);
  pinMode(porta_buzzer, OUTPUT);
  pinMode(porta_motor_dir, OUTPUT);
  pinMode(porta_motor_esq, OUTPUT);
  pinMode(porta_fimdecurso_A, INPUT);
  pinMode(porta_fimdecurso_B, INPUT);  
  pinMode(porta_contador, INPUT);
  pinMode(porta_corrente, INPUT);
  pinMode(porta_tensao, INPUT);
  Bluetooth.begin(38400);           // Inicia Bluetooth
  Wire.begin();                     // Inicia porta de Comunicação I2C
  Bme.begin();                      // Inicia Sensor de Temperatura e Umidade
  }

// [ SETUP / VOID LOOP ] //
// Inicializando Sistema
    void loop() {
    buzzer_sinal();
    ligaLED_verde();
    desligaLED_vermelho();
    Bluetooth.println(); Bluetooth.println(F("POWER ON"));
    Bluetooth.println(F("Checando Bluetooth")); Bluetooth.println();
    delay(2000);

// Checando Bluetooth
    Bluetooth.println(F("Conexão com Bluetooth - OK"));
    Bluetooth.println(F("Clique 'STOP' para Continuar")); Bluetooth.println();
    while (true) {
    piscaLED_verde();
    currentMillis = millis(); if (currentMillis - previousMillis >= intervalo5) { previousMillis = currentMillis;
    Bluetooth.println(F("Conexão com Bluetooth - OK"));
    Bluetooth.println(F("Clique 'STOP' para Continuar")); Bluetooth.println();  }
    Bluetooth_info=Bluetooth.read(); if (Bluetooth_info=='s') {break;}}

// Saudação do Sistema
    ligaLED_verde();
    buzzer_beep();
    Bluetooth.println(); Bluetooth.println(F("_______________________")); Bluetooth.println();
    Bluetooth.println(F("INTEX AGRO"));
    Bluetooth.println(F("Sistema de monitoramento"));
    Bluetooth.println(F("e controle do Protótipo ISA-2"));
    Bluetooth.println(F("(MDP versão 2.90)"));
    Bluetooth.println(); Bluetooth.println(F("_______________________")); Bluetooth.println(); delay(500);
    Bluetooth.println(F("Iniciando Sistema, aguarde!"));
    Bluetooth.println(F("_______________________"));
    delay(1500);
    
// Checando Bateria
    Bluetooth.println(); Bluetooth.println(F("- Checando BATERIA...")); Bluetooth.println();
    tensao = 0; medicoes=0; volt=0; carga_bat=0;
    for(int index = 0; index < 100; index++) {
    volt = (analogRead(porta_tensao));  volt = (volt * vmax)/1023.0;
    medicoes = medicoes + 1; tensao = tensao + volt; delay(1); }
    carga = map((tensao * 100), 105000, 120000, 0, 100); if (carga <= 0.1) carga = 0;
    tensao = tensao / medicoes; if (tensao <= 0.1) tensao = 0;
    while (tensao <= 10.5) {
    // SEM CARGA SUFICIENTE //
    desligaLED_verde();
    piscaLED_vermelho_fast();
    currentMillis = millis();
    if (currentMillis - previousMillis >= intervalo5) {
    Bluetooth.println(F("CARGA INSUFICIENTE"));
    Bluetooth.println(F("RECARREGUE P/ CONTINUAR")); Bluetooth.println();
    bateria_monitor(); tensao_monitor();
    Bluetooth.println(); Bluetooth.println();
    previousMillis = currentMillis; } } delay(900);
    // --- BATERIA ENTRE 0% E 20% --- //
    if (carga <= 20) {Bluetooth.println(F("ATENÇÃO !!!")); delay(1000); Bluetooth.print(F("BATERIA COM ")); Bluetooth.print(carga);
    Bluetooth.println(F(" % DE CARGA")); Bluetooth.println(); delay(1000);
    Bluetooth.println(F("RECARREGÁ-LA")); Bluetooth.println(F("ANTES DE INICIAR")); Bluetooth.println(F("UMA NOVA SANGRIA")); delay(2000);
    float temp(NAN), hum(NAN), pres(NAN); BME280::TempUnit tempUnit(BME280::TempUnit_Celsius); BME280::PresUnit presUnit(BME280::PresUnit_Pa);
    Bme.read(pres, temp, hum, tempUnit, presUnit); temperatura = temp; umidade = hum;
    goto MODO_MANUTENCAO_BATERIA;}
    // --- BATERIA ACIMA DE 20% --- //
    else {tensao_monitor(); bateria_monitor(); Bluetooth.println(); Bluetooth.println(F("BATERIA - OK"));
    Bluetooth.println(); Bluetooth.println(F("_______________________"));}
    desligaLED_vermelho(); ligaLED_verde();
    delay(900);

// Checando Sensor de ambiente
    Bluetooth.println(); Bluetooth.println(F("- Checando AMBIENTE...")); Bluetooth.println();
    float temp(NAN), hum(NAN), pres(NAN);
    BME280::TempUnit tempUnit(BME280::TempUnit_Celsius);
    BME280::PresUnit presUnit(BME280::PresUnit_Pa);
    Bme.read(pres, temp, hum, tempUnit, presUnit);
    temperatura = temp;
    umidade = hum;
    delay(1000);
    Bluetooth.print(F("Temperatura Ambiente: ")); Bluetooth.print(temp); Bluetooth.println(F("°"));
    Bluetooth.print(F("Umidade Relativa do Ar: ")); Bluetooth.print(hum); Bluetooth.println(F("%")); delay(500);
    Bluetooth.println(); Bluetooth.print(F("SENSOR DO AMBIENTE - OK"));
    Bluetooth.println(); Bluetooth.println(); Bluetooth.println(F("_______________________")); delay(500);
    delay(1000);

// Checando posição do atuador
    Bluetooth.println(); Bluetooth.println(F("- Checando ATUADOR...")); Bluetooth.println();
    delay(1000);
    if (digitalRead(porta_fimdecurso_A) == HIGH) { 
    goto MODO_MANUTENCAO_ATUADOR; }
    Bluetooth.println(F("ATUADOR posicionado em"));
    Bluetooth.println(F("Geratriz 'A' - Pronto"));
    Bluetooth.println(F("para SANGRIA AUTOMÁTICA"));
    Bluetooth.println(); Bluetooth.println(F("_______________________")); delay(500);
    
// Aguardando comando via Bluetooth    
    desligaLED_vermelho(); piscaLED_verde();
    buzzer_sinal();
    Bluetooth.println(); Bluetooth.println(); Bluetooth.println();
    Bluetooth.println(F("Selecione uma Operação"));
    while (true) {
    currentMillis = millis();
    if (currentMillis - previousMillis >= intervalo50) {
    previousMillis = currentMillis;
    Bluetooth.println(); Bluetooth.println(); Bluetooth.println();
    Bluetooth.println(F("Selecione uma Operação"));    }
    if (Bluetooth.available()>0) { Bluetooth_dados = Bluetooth.read(); switch (Bluetooth_dados) {
    case 't':  tensao_monitor();              break;
    case 'c':  corrente_monitor();            break;
    case 'C':  CORRENTE_CONSTANTE_MONITOR();  break;
    case 'b':  bateria_monitor();             break;
    case 'a':  ambiente();                    break;
    case 'A':  if (digitalRead(porta_fimdecurso_A) == HIGH) {goto MODO_MANUTENCAO_ATUADOR; break;} else {consumo=1; goto MODO_AUTO; break;}
    case 'B':  if (digitalRead(porta_fimdecurso_A) == HIGH) {goto MODO_MANUTENCAO_ATUADOR; break;} else {consumo=2; goto MODO_AUTO; break;}
    case 'm':  goto MODO_MANUTENCAO_BOTAO;    break;
    default:   break;   } } }

// [ MODO AUTOMÁTICO]
    MODO_AUTO:
    desligaLED_vermelho();  ligaLED_verde();
    Bluetooth.println(); Bluetooth.println(); Bluetooth.println(F("[ * MODO AUTOMÁTICO * ]")); Bluetooth.println(); Bluetooth.println();
    Bluetooth_info=Bluetooth.read(); if (Bluetooth_info=='s') {goto MODO_MANUTENCAO_BOTAO;}
    count=0;                                                                        // ZERA ODÔMETRO
    consumo_mA_medio = 0; consumo_mAH = 0; valorMedicoes = 0; totalMedicoes = 0;    // ZERA CONTADORES DE MEDIÇÃO DE CORRENTE
    CORRENTE_CONSTANTE();                                                           // INICIA CONTAGEM DO CONSUMO DE CORRENTE
    tensao_inicio = tensao; carga_inicio = carga;                                   // REGISTRA TENSÃO E CARGA INICIAL DA BATERIA
    tempo_inicio = millis();                                                        // INICIA CRONÔMETRO
    
// Processo 1/3 - Geratriz-A => Geratriz-B
    Bluetooth.println(F("Movendo para geratriz 'B'"));
    Bluetooth.print(F("(Processo 1/3)"));
    while (digitalRead(porta_fimdecurso_B) == HIGH) {
    Bluetooth_info=Bluetooth.read(); if (Bluetooth_info=='s') {goto MODO_MANUTENCAO_BOTAO;}
    contador(); CORRENTE_CONSTANTE(); motorDIR();   } 
    motorOFF(); CORRENTE_CONSTANTE(); count_1=count;                                // REGISTRA PERCURSO DA 1ª ETAPA
    Bluetooth_info=Bluetooth.read(); if (Bluetooth_info=='s') {goto MODO_MANUTENCAO_BOTAO;}
    // Fim Processo 1/3
    Bluetooth.println(F(" - CONCLUÍDO"));
    delay(1000);

// Processo 2/3 - Posiciona Atuador VERTICALMENTE na Linha da Casca Consumida
    Bluetooth.println(); Bluetooth.println(); Bluetooth.println(F("Posicionando atuador na casca"));
    Bluetooth.print(F("(Processo 2/3)"));
    count=0; while (count < distancia) {
    Bluetooth_info=Bluetooth.read(); if (Bluetooth_info=='s') {goto MODO_MANUTENCAO_BOTAO;}
    contador(); CORRENTE_CONSTANTE(); motorDIR_lento(); }  
    motorOFF(); CORRENTE_CONSTANTE(); 
    Bluetooth_info=Bluetooth.read(); if (Bluetooth_info=='s') {goto MODO_MANUTENCAO_BOTAO;}
    Bluetooth.println(F(" - CONCLUÍDO"));

// Continua Processo 2/3 - Posiciona Atuador VERTICALMENTE na linha de corte selecionada
    delay(100); Bluetooth.println(); Bluetooth.println(); 
    Bluetooth.println(F("Posicionando para consumo"));
    Bluetooth.print(F("de casca pré-definido: (")); Bluetooth.print(consumo); Bluetooth.println(F(" mm)")); delay(500);
    count=0; while (count < consumo) {
    Bluetooth_info=Bluetooth.read(); if (Bluetooth_info=='s') {goto MODO_MANUTENCAO_BOTAO;}
    contador(); CORRENTE_CONSTANTE(); motorDIR_lento(); } 
    motorOFF(); CORRENTE_CONSTANTE();
    Bluetooth_info=Bluetooth.read(); if (Bluetooth_info=='s') {goto MODO_MANUTENCAO_BOTAO;}
    // Fim do Processo 2/3
    Bluetooth.println(F("CONCLUÍDO"));
    delay(1000);

// Inicio Processo 3/3 - Geratriz-B => Geratriz-A - Efetua Sangria
    Bluetooth.println(); Bluetooth.println(); 
    Bluetooth.println(F("Movendo para geratriz 'A'"));
    Bluetooth.println(F("(Efetuando Sangria)"));
    Bluetooth.println(F("(Processo 3/3)"));
    count=0; while ((digitalRead(porta_fimdecurso_A)) == HIGH) {
    Bluetooth_info=Bluetooth.read(); if (Bluetooth_info=='s') {goto MODO_MANUTENCAO_BOTAO;}
    contador(); CORRENTE_CONSTANTE(); motorESQ();     }
    motorOFF(); CORRENTE_CONSTANTE(); count_2=count;
    Bluetooth_info=Bluetooth.read(); if (Bluetooth_info=='s') {goto MODO_MANUTENCAO_BOTAO;}
    Bluetooth.print(F("CONCLUÍDO")); delay(1000);
    // RECOLHE ATUADOR DA LINHA DA CASCA //
    Bluetooth.println(); Bluetooth.println();
    Bluetooth.print(F("Recolhendo atuador"));
    count=0; while (count < distancia) {
    Bluetooth_info=Bluetooth.read(); if (Bluetooth_info=='s') {goto MODO_MANUTENCAO_BOTAO;}
    contador(); CORRENTE_CONSTANTE(); motorESQ_lento(); }
    motorOFF(); CORRENTE_CONSTANTE();
    Bluetooth_info=Bluetooth.read(); if (Bluetooth_info=='s') {goto MODO_MANUTENCAO_BOTAO;}
    Bluetooth.println(F(" - OK"));
    delay(1000);

// Fim do Processo 3/3
    tempo_fim = (millis() - tempo_inicio) / 1000;
    Bluetooth.println(); Bluetooth.println(); Bluetooth.println(F("SANGRIA CONCLUÍDA")); Bluetooth.println(); Bluetooth.println(); Bluetooth.println(); 
    buzzer_sucess();
    piscaLED_verde();
    delay(2000);

// [ RESUMO DO PROCESSO ]
    int consumo_mA_medio = valorMedicoes/totalMedicoes;
    int consumo_mAH = ((valorMedicoes / 3600) * totalMedicoes) * 1;
    tensao = 0; medicoes=0; volt=0; carga_bat=0; for(int index = 0; index < 100; index++) { 
    volt = (analogRead(porta_tensao)); volt = (volt * vmax)/1023.0;medicoes=medicoes + 1; tensao=tensao + volt; delay(1); }
    carga_bat=tensao*100; carga=map((carga_bat), 105000, 120000, 0, 100); if (carga <= 0.1) carga = 0; if (carga >= 100) carga = 100;
    tensao = tensao / medicoes;
    float consumo_bateria = carga - carga_inicio;
    float consumo_tensao = tensao - tensao_inicio;
    Bme.read(pres, temp, hum, tempUnit, presUnit); 
    Bluetooth.println(F("_______________________")); Bluetooth.println();
    Bluetooth.println(F("** RESUMO DO PROCESSO: **")); delay(1000); Bluetooth.println();
    Bluetooth.print(F("Tempo de Duração: ")); Bluetooth.print(tempo_fim); Bluetooth.println(F(" segundos")); delay(200);
    Bluetooth.print(F("Extensão da 1ª etapa: ")); Bluetooth.print(count_1); Bluetooth.println(F(" mm")); delay(200);
    Bluetooth.print(F("Extensão da 2ª etapa: ")); Bluetooth.print(count_2); Bluetooth.println(F(" mm")); delay(200);
    Bluetooth.print(F("Extensão Total: ")); Bluetooth.print(count_1 + count_2); Bluetooth.println(F(" mm")); delay(200);
    Bluetooth.print(F("DAP aproximado: ")); Bluetooth.print(F("(Calculando)")); Bluetooth.println(); delay(1000);

    Bluetooth.print(F("Consumo de Casca: ")); Bluetooth.print(consumo); Bluetooth.println(F(" mm)")); delay(200);
    Bluetooth.print(F("Umidade Relativa do ar: ")); Bluetooth.print(umidade); Bluetooth.println(F("%")); delay(200);
    Bluetooth.print(F("Temperatura do Ambiente: ")); Bluetooth.print(temperatura); Bluetooth.println(F("°")); delay(200);
    Bluetooth.print(F("Temperatura do ISA: ")); Bluetooth.print(temp); Bluetooth.println(F("°")); Bluetooth.println(); delay(1000);

    Bluetooth.print(F("Carga inicial Bateria: ")); Bluetooth.print(carga_inicio); Bluetooth.println(F(" %")); delay(200);
    Bluetooth.print(F("Carga atual Bateria: ")); Bluetooth.print(carga); Bluetooth.println(F(" %")); delay(200);
    Bluetooth.print(F("Carga Consumida Bateria: ")); Bluetooth.print(consumo_bateria); Bluetooth.println(F(" %")); delay(200);
    Bluetooth.print(F("Tensão inicial Bateria: ")); Bluetooth.print(tensao_inicio); Bluetooth.println(F(" v")); delay(200);
    Bluetooth.print(F("Tensão atual Bateria: ")); Bluetooth.print(tensao); Bluetooth.println(F(" v")); delay(200);
    Bluetooth.print(F("Tensão Consumida: ")); Bluetooth.print(consumo_tensao); Bluetooth.println(F(" v")); Bluetooth.println(); delay(1000);

    Bluetooth.print(F("Consumo de Corrente: ")); Bluetooth.print(consumo_mA_medio); Bluetooth.println(F(" mA")); delay(200);
    Bluetooth.print(F("Consumo de Potência: ")); Bluetooth.print((tensao * consumo_mA_medio) /1000); Bluetooth.println(F(" W")); delay(200);
    Bluetooth.print(F("Corrente/Hora: ")); Bluetooth.print(consumo_mAH); Bluetooth.println(F(" mAH")); delay(200);
    Bluetooth.print(F("Watts/Hora: ")); Bluetooth.print((tensao * consumo_mAH) /1000); Bluetooth.println(F(" Wh")); delay(1000);
    Bluetooth.println(); Bluetooth.println(F("** FIM DO RESUMO **"));
    Bluetooth.println(F("_______________________")); delay(2000);

// Encerramento do Modo Automático
    Bluetooth.println(); Bluetooth.println(); Bluetooth.println();
    Bluetooth.println(F("[ENCERRAMENTO]"));  Bluetooth.println(); delay(1000);
    Bluetooth.println(F("Compartilhar Relatório!")); delay(500);
    if (carga <= 20) {Bluetooth.println(); Bluetooth.println(F("ATENÇÃO !!!")); delay(500); 
    Bluetooth.print(F("BATERIA COM")); Bluetooth.print(carga); Bluetooth.println(F(" % DE CARGA")); Bluetooth.println(); delay(1000);
    Bluetooth.println(F("RECARREGUE ANTES DA")); Bluetooth.println(F("PRÓXIMA SANGRIA")); Bluetooth.println(); delay(1500);}
    else {Bluetooth.println(F("Conferir a Carga da Bateria")); bateria_monitor(); delay(500);}
    Bluetooth.println(F("Pode desligar o ISA")); delay(500);
    Bluetooth.println(); Bluetooth.println(); Bluetooth.println(); Bluetooth.println(F("Até a próxima Sangria!")); Bluetooth.println();
    consumo_mA_medio = 0; consumo_mAH = 0; valorMedicoes = 0; totalMedicoes = 0;
    desligaLED_verde(); piscaLED_vermelho();
    while (true) {
    if (Bluetooth.available()>0) { Bluetooth_dados = Bluetooth.read(); switch (Bluetooth_dados) {
    case 't': tensao_monitor();   break;
    case 'c': corrente_monitor(); break;
    case 'b': bateria_monitor();  break;
    case 'a': ambiente();         break;
    case 'm': goto MODO_MANUTENCAO_BOTAO;  break;
    case '7': buzzer_beep(); break;
    case '8': buzzer_beep(); delay(200); buzzer_beep(); break;
    case '9': buzzer_sinal(); break;
    case '0': buzzer_sucess(); break;
    default:  break;
    } } }

// Entrada Modo Manutencao
    MODO_MANUTENCAO_ATUADOR:
    desligaLED_verde(); piscaLED_vermelho();
    Bluetooth.println(F("ATUADOR fora de posição"));
    Bluetooth.println(F("para SANGRIA AUTOMÁTICA"));
    Bluetooth.println(F("(Geratriz 'A')"));
    Bluetooth.println(); Bluetooth.println(F("_______________________")); delay(500);
    Bluetooth.println(); Bluetooth.println(F("Iniciando MODO MANUTENÇÃO"));    
    buzzer_beep(); delay(200); buzzer_beep();
    delay(1000);
    goto MODO_MANUTENCAO;

    MODO_MANUTENCAO_BOTAO:
    desligaLED_verde(); piscaLED_vermelho();
    Bluetooth.println(); Bluetooth.println(F("Iniciando MODO MANUTENÇÃO"));
    Bluetooth.println(F("(Requisitado pelo Operador)"));
    Bluetooth.println(); Bluetooth.println(F("_______________________")); delay(500);
    buzzer_beep(); delay(200); buzzer_beep();
    delay(1000);
    goto MODO_MANUTENCAO;

    MODO_MANUTENCAO_BATERIA:
    desligaLED_verde(); piscaLED_vermelho();
    Bluetooth.println(); Bluetooth.println(F("Iniciando MODO MANUTENÇÃO"));
    Bluetooth.println(F("devido a baixa carga"));
    Bluetooth.println(F("da Bateria")); Bluetooth.println(); delay(1000);
    Bluetooth.println(F("Iniciando MODO MANUTENÇÃO"));
    Bluetooth.println(); Bluetooth.println(F("_______________________")); delay(500);
    buzzer_beep(); delay(200); buzzer_beep();
    delay(1000);

// [ MODO MANUTENÇÃO ] //
  MODO_MANUTENCAO:
  Bluetooth.println(); Bluetooth.println(); Bluetooth.println(); Bluetooth.println(F("[ * MODO MANUTENÇÃO * ]"));
  Bluetooth.println(); Bluetooth.println(); Bluetooth.println(); Bluetooth.println(F("Selecione uma Operação")); Bluetooth.println();
  while (true) {
  motorOFF(); piscaLED_vermelho();  
  currentMillis = millis(); if (currentMillis - previousMillis >= intervalo50) { previousMillis = currentMillis;
  Bluetooth.println(); Bluetooth.println(F("Selecione uma Operação")); Bluetooth.println();}
  if (Bluetooth.available()>0) { Bluetooth_dados = Bluetooth.read(); switch (Bluetooth_dados) {
  case 'A': if (digitalRead(porta_fimdecurso_A) == HIGH) {goto MODO_MANUTENCAO_ATUADOR; break;} else {consumo=1; goto MODO_AUTO; break;}  // AUTOMATICO-1
  case 'B': if (digitalRead(porta_fimdecurso_A) == HIGH) {goto MODO_MANUTENCAO_ATUADOR; break;} else {consumo=2; goto MODO_AUTO; break;}  // AUTOMATICO-2
  case 'b': bateria_monitor(); Bluetooth.println();   break;  // BATERIA
  case 't': tensao_monitor(); Bluetooth.println();    break;  // TENSÃO
  case 'c': corrente_monitor(); Bluetooth.println();  break;  // CORRENTE
  case 'a': ambiente(); Bluetooth.println();          break;  // AMBIENTE
  case 'm': Bluetooth.println(); Bluetooth.println(F("[ * EM MODO MANUTENÇÃO * ]")); Bluetooth.println(); break;  // MODO MANUTENÇÃO
  case 'T': Bluetooth.println(F("Teste Fim de Curso:")); while (true) {Bluetooth_info=Bluetooth.read(); if (Bluetooth_info=='s') {goto fim_teste;}
  teste_FC(); } fim_teste: Bluetooth.println(); Bluetooth.println(F("Selecione uma Operação")); Bluetooth.println(); break;  // TESTE FIM DE CURSO
  case 'e': Bluetooth.println(F("Movendo Carro para ESQUERDA")); Bluetooth.println(F("até Geratriz 'A' ou 'STOP'"));
  while (digitalRead(porta_fimdecurso_A)==HIGH) {ligaLED_vermelho(); CORRENTE_CONSTANTE_MONITOR(); motorESQ();
  Bluetooth_info=Bluetooth.read(); if (Bluetooth_info=='s') {goto STOP;}}
  motorOFF(); Bluetooth.println(F("Posicionado em Geratriz 'A'")); break; // ESQUERDA
  case 'd': Bluetooth.println(F("Movendo Carro para DIREITA")); Bluetooth.println(F("até Geratriz 'B' ou 'STOP'"));
  while (digitalRead(porta_fimdecurso_B)==HIGH) {ligaLED_vermelho(); CORRENTE_CONSTANTE_MONITOR(); motorDIR();
    Bluetooth_info=Bluetooth.read(); if (Bluetooth_info=='s') {goto STOP;}}
    motorOFF(); Bluetooth.println(F("Posicionado em Geratriz 'B'")); break; // DIREITA
  case 's': STOP: motorOFF(); Bluetooth.println(); Bluetooth.println(F("STOP")); Bluetooth.println(); break; // STOP
  case '1': count=0; while (count<1) {ligaLED_vermelho(); contador(); motorESQ_lento();} Bluetooth.println(F("1mm ESQUERDA")); motorOFF(); break;
  case '2': count=0; while (count<3) {ligaLED_vermelho(); contador(); motorESQ_lento();} Bluetooth.println(F("3mm ESQUERDA")); motorOFF(); break;
  case '3': count=0; while (count<5) {ligaLED_vermelho(); contador(); motorESQ_lento();} Bluetooth.println(F("5mm ESQUERDA")); motorOFF(); break;
  case '4': count=0; while (count<1) {ligaLED_vermelho(); contador(); motorDIR_lento();} Bluetooth.println(F("1mm DIREITA")); motorOFF(); break;
  case '5': count=0; while (count<3) {ligaLED_vermelho(); contador(); motorDIR_lento();} Bluetooth.println(F("3mm DIREITA")); motorOFF(); break;
  case '6': count=0; while (count<5) {ligaLED_vermelho(); contador(); motorDIR_lento();} Bluetooth.println(F("5mm DIREITA")); motorOFF(); break;
  case '7': buzzer_beep(); break;
  case '8': buzzer_beep(); delay(200); buzzer_beep(); break;
  case '9': buzzer_sinal(); break;
  case '0': buzzer_sucess(); break;
  default: break; continue;  }  }  }  }

// [ FIM LOOP ] //
// [ COMANDOS E FUNÇÕES DO SISTEMA ] //
// --- MONITOR TENSAO E CORRENTE --- //
  void tensao_monitor() {
  tensao = 0; medicoes=0; volt=0;
  for(int index = 0; index < 100; index++) {
  volt = (analogRead(porta_tensao));  volt = (volt * vmax)/1023.0;
  medicoes = medicoes + 1; tensao = tensao + volt; delay(1); }
  tensao = tensao / medicoes; if (tensao <= 0.1) tensao = 0;
  Bluetooth.print(F("Tensão da Bateria: ")); Bluetooth.print(tensao); Bluetooth.println(F(" Volts"));
  }
  void bateria_monitor() {
  tensao = 0; medicoes=0;	volt=0; carga_bat=0;
  for(int index = 0; index < 100; index++) {
  volt = (analogRead(porta_tensao));  volt = (volt * vmax)/1023.0;
  medicoes = medicoes + 1;  tensao = tensao + volt; delay(1); }
  carga_bat=tensao*100; carga=map((carga_bat), 105000, 120000, 0, 100); if (carga <= 0.1) carga = 0; if (carga >= 100) carga = 100; tensao = tensao / medicoes;
  Bluetooth.print(F("Carga da Bateria: ")); Bluetooth.print(carga); Bluetooth.println(F(" %"));
  }
  void corrente_monitor() {
  for(int index = 0; index < 900; index++) {
  corrente_media = (analogRead(porta_corrente) * 6.75);
  corrente = corrente + corrente_media; } corrente = corrente / 900;  
  Bluetooth.println(F("Consumo de Corrente: ")); Bluetooth.print(corrente); Bluetooth.println(" mAh");
  }
  void CORRENTE_CONSTANTE() {
  MillisAtual = millis();
  if (MillisAtual - MillisAnterior >= intervalo1) {
  for(int index = 0; index < 900; index++) {
  corrente_media = (analogRead(porta_corrente) * 6.75);
  corrente = corrente + corrente_media; } corrente = corrente / 900;
  MillisAnterior = MillisAtual;
  valorMedicoes = valorMedicoes + corrente;
  totalMedicoes = totalMedicoes + 1;
  } }
  void CORRENTE_CONSTANTE_MONITOR() {
  MillisAtual = millis();
  if (MillisAtual - MillisAnterior >= intervalo0) { MillisAnterior = MillisAtual;
  for(int index = 0; index < 900; index++) {
  corrente_media = (analogRead(porta_corrente) * 6.75);
  corrente = corrente + corrente_media; } corrente = corrente / 900;
  Bluetooth.print(F("Corrente: ")); Bluetooth.print(corrente); Bluetooth.println(" mAh");
  } }

// --- MOTOR --- //
  void motorESQ() {
  digitalWrite(porta_motor_dir, LOW);
  analogWrite(porta_motor_esq, fastmotor);   
  }
  void motorDIR() {
  digitalWrite(porta_motor_esq, LOW);
  analogWrite(porta_motor_dir, fastmotor);
  }
  void motorESQ_lento()  {
  digitalWrite(porta_motor_dir, LOW);
  analogWrite(porta_motor_esq, slowmotor);
  }
  void motorDIR_lento()  {
  digitalWrite(porta_motor_esq, LOW);
  analogWrite(porta_motor_dir, slowmotor);
  }
  void motorOFF() {
  digitalWrite(porta_motor_esq, HIGH);
  digitalWrite(porta_motor_esq, LOW);
  digitalWrite(porta_motor_dir, LOW);
  }

// --- CONTADOR --- //
  void contador() {
  estado = digitalRead(porta_contador);
  if (!estado && estadoAnt) { count++; delay(50); }
  estadoAnt = estado;
  }

// --- TESTE FIM DE CURSO --- //
  void teste_FC() {
  int fcA = digitalRead(porta_fimdecurso_A); int fcB = digitalRead(porta_fimdecurso_B);
  MillisAtual = millis(); if (MillisAtual - MillisAnterior >= intervalo0) {MillisAnterior = MillisAtual;
  Bluetooth.print(F("FC 'A' = ")); Bluetooth.print(fcA); Bluetooth.print(F("   |   FC 'B' = ")); Bluetooth.println(fcB);
  } }

// --- AMBIENTE --- //
  void ambiente()  {
  float temp(NAN), hum(NAN), pres(NAN);
  BME280::TempUnit tempUnit(BME280::TempUnit_Celsius);
  BME280::PresUnit presUnit(BME280::PresUnit_Pa);
  Bme.read(pres, temp, hum, tempUnit, presUnit);
  Bluetooth.print(F("Temperatura: ")); Bluetooth.print(temp); Bluetooth.println(F("°"));
  Bluetooth.print(F("Umidade: ")); Bluetooth.print(hum); Bluetooth.println(F("%"));
  }

// --- BUZZER --- //
  void buzzer_beep() {
  tone(porta_buzzer, 784, 100);
  delay(100);
  }
  void buzzer_sinal() {
  tone(porta_buzzer, 391, 150);
  delay(150);
  tone(porta_buzzer, 784, 150);
  }
  void buzzer_sucess() {
  tone(porta_buzzer, 391, 150); delay(150);
  tone(porta_buzzer, 489, 150); delay(150);   
  tone(porta_buzzer, 587, 150); delay(150);
  tone(porta_buzzer, 784, 150); delay(100);
  }

// --- LED VERDE--- //
  void ligaLED_verde() {
  digitalWrite(porta_led_verde, HIGH); 
  }
  void desligaLED_verde() {
  digitalWrite(porta_led_verde, LOW);
  }
  void piscaLED_verde() {
  static unsigned long delayPisca1;
  if ((millis() - delayPisca1) < 200) {
  digitalWrite(porta_led_verde, HIGH);     }
  if (((millis() - delayPisca1) >= 200) && ((millis() - delayPisca1) < 400)) {
  digitalWrite(porta_led_verde, LOW);     }
  if (((millis() - delayPisca1) >= 400) && ((millis() - delayPisca1) < 600)) {
  digitalWrite(porta_led_verde, HIGH);     }
  if (((millis() - delayPisca1) >= 600) && ((millis() - delayPisca1) < 800)) {
  digitalWrite(porta_led_verde, LOW);     }
  if ((millis() - delayPisca1) > 1800) {
  delayPisca1 = millis();     }
  }

// --- LED VERMELHO--- // 
  void ligaLED_vermelho() {
  digitalWrite(porta_led_vermelho, HIGH); 
  }
  void desligaLED_vermelho() {
  digitalWrite(porta_led_vermelho, LOW);
  }
  void piscaLED_vermelho() {
  static unsigned long delayPisca2;
  if ((millis() - delayPisca2) < 200) {
  digitalWrite(porta_led_vermelho, HIGH);     }
  if (((millis() - delayPisca2) >= 200) && ((millis() - delayPisca2) < 400)) {
  digitalWrite(porta_led_vermelho, LOW);     }
  if (((millis() - delayPisca2) >= 400) && ((millis() - delayPisca2) < 600)) {
  digitalWrite(porta_led_vermelho, HIGH);     }
  if (((millis() - delayPisca2) >= 600) && ((millis() - delayPisca2) < 800)) {
  digitalWrite(porta_led_vermelho, LOW);     }
  if ((millis() - delayPisca2) > 1800) {
  delayPisca2 = millis();     }
  }
  void piscaLED_vermelho_fast() {
  atualMillis = millis();
  if (atualMillis - anteriorMillis >= tempo_pisca_fast) {
  anteriorMillis = atualMillis;
  if (statusLED == LOW) {statusLED = HIGH;} else {statusLED = LOW;}
  digitalWrite(porta_led_vermelho, statusLED);  }
  }