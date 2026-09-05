// ============================================================
// TACÔMETRO HONDA FAN 125 2018
// Arduino Nano -> Painel Titan 2023 Blackout
//
// D2 = entrada do pulso da moto
// D9 = saída do sinal para o painel
//
// Cada posição da tabela representa uma faixa de 250 RPM.
// Os valores de frequência são PROVISÓRIOS e devem ser
// substituídos pelos valores descobertos nos testes.
// ============================================================


// ============================================================
// CONFIGURAÇÃO
// ============================================================

const byte PINO_ENTRADA = 2;
const byte PINO_SAIDA   = 9;

// Quantidade de eventos detectados pelo sensor por volta.
// Valor já calibrado nos testes anteriores.
const float PULSOS_POR_VOLTA = 9.2;

// Ignora pulsos separados por menos que 500 microssegundos.
const unsigned long FILTRO_US = 500;


// ============================================================
// TABELA DE FREQUÊNCIAS
// ============================================================
//
// Cada posição representa:
//
// [0]  = 250  até  499 RPM
// [1]  = 500  até  749 RPM
// [2]  = 750  até  999 RPM
// [3]  = 1000 até 1249 RPM
// ...
// [43] = 11000 até 11249 RPM
//
// ============================================================

const float frequencias[44] = {

  8.800,   // [00] 250 - 499
  8.828,   // [01] 500 - 749
  8.856,   // [02] 750 - 999
  8.884,   // [03] 1000 - 1249
  8.912,   // [04] 1250 - 1499
  8.940,   // [05] 1500 - 1749
  8.967,   // [06] 1750 - 1999
  8.995,   // [07] 2000 - 2249
  9.023,   // [08] 2250 - 2499
  9.051,   // [09] 2500 - 2749
  9.079,   // [10] 2750 - 2999
  9.107,   // [11] 3000 - 3249
  9.135,   // [12] 3250 - 3499
  9.163,   // [13] 3500 - 3749
  9.191,   // [14] 3750 - 3999
  9.219,   // [15] 4000 - 4249
  9.247,   // [16] 4250 - 4499
  9.274,   // [17] 4500 - 4749
  9.302,   // [18] 4750 - 4999
  9.330,   // [19] 5000 - 5249
  9.358,   // [20] 5250 - 5499
  9.386,   // [21] 5500 - 5749
  9.414,   // [22] 5750 - 5999
  9.442,   // [23] 6000 - 6249
  9.470,   // [24] 6250 - 6499
  9.498,   // [25] 6500 - 6749
  9.526,   // [26] 6750 - 6999
  9.553,   // [27] 7000 - 7249
  9.581,   // [28] 7250 - 7499
  9.609,   // [29] 7500 - 7749
  9.637,   // [30] 7750 - 7999
  9.665,   // [31] 8000 - 8249
  9.693,   // [32] 8250 - 8499
  9.721,   // [33] 8500 - 8749
  9.749,   // [34] 8750 - 8999
  9.777,   // [35] 9000 - 9249
  9.805,   // [36] 9250 - 9499
  9.832,   // [37] 9500 - 9749
  9.860,   // [38] 9750 - 9999
  9.888,   // [39] 10000 - 10249
  9.916,   // [40] 10250 - 10499
  9.944,   // [41] 10500 - 10749
  9.972,   // [42] 10750 - 10999
  10.000   // [43] 11000+
};


// ============================================================
// VARIÁVEIS
// ============================================================

volatile unsigned long pulsos = 0;
volatile unsigned long ultimoPulso = 0;

float rpmFiltrado = 0.0;
float frequenciaAtual = 0.0;


// ============================================================
// INTERRUPÇÃO DO SENSOR
// ============================================================

void contarPulso() {

  unsigned long agora = micros();

  if (agora - ultimoPulso >= FILTRO_US) {

    pulsos++;

    ultimoPulso = agora;
  }
}


// ============================================================
// ESCOLHE O QUADRADO DO PAINEL
// ============================================================
//
// 250-499   -> posição 0
// 500-749   -> posição 1
// 750-999   -> posição 2
// etc.
//
// Não existem 44 IFs.
// A própria matemática encontra a posição da tabela.
// ============================================================

float converterRPM(float rpm) {

  // Abaixo de 250 RPM = painel parado
  if (rpm < 250.0) {
    return 0.0;
  }

  int indice = (int)((rpm - 250.0) / 250.0);

  // Proteção contra ultrapassar a tabela
  if (indice < 0) {
    indice = 0;
  }

  if (indice >= 44) {
    indice = 43;
  }

  return frequencias[indice];
}


// ============================================================
// CONFIGURA TIMER1
// ============================================================
//
// D9 = OC1A
//
// Timer1 gera diretamente uma onda quadrada em hardware.
// Isso permite frequências decimais sem delayMicroseconds()
// e sem bloquear a leitura do sensor.
//
// Frequência:
//
// F_CPU / (2 * prescaler * (OCR1A + 1))
//
// Prescaler = 64
// ============================================================

void configurarFrequencia(float frequencia) {

  if (frequencia <= 0.0) {

    // Desliga saída do Timer1
    TCCR1A &= ~(1 << COM1A0);

    digitalWrite(PINO_SAIDA, LOW);

    frequenciaAtual = 0.0;

    return;
  }


  // Calcula OCR1A para a frequência desejada
  unsigned long valor =
    (unsigned long)((16000000.0 /
    (2.0 * 64.0 * frequencia)) - 1.0);


  // Limites de segurança
  if (valor > 65535) {
    valor = 65535;
  }

  if (valor < 1) {
    valor = 1;
  }


  // Timer1 em CTC
  TCCR1A = 0;
  TCCR1B = 0;

  // CTC
  TCCR1B |= (1 << WGM12);

  // Toggle OC1A = D9
  TCCR1A |= (1 << COM1A0);

  // Valor de comparação
  OCR1A = (uint16_t)valor;

  // Prescaler 64
  TCCR1B |= (1 << CS11);
  TCCR1B |= (1 << CS10);

  frequenciaAtual = frequencia;
}


// ============================================================
// SETUP
// ============================================================

void setup() {

  Serial.begin(115200);

  pinMode(PINO_ENTRADA, INPUT);

  pinMode(PINO_SAIDA, OUTPUT);

  digitalWrite(PINO_SAIDA, LOW);


  // Interrupção do sensor
  attachInterrupt(
    digitalPinToInterrupt(PINO_ENTRADA),
    contarPulso,
    RISING
  );


  // Começa com saída desligada
  configurarFrequencia(0.0);
}


// ============================================================
// LOOP
// ============================================================

void loop() {

  static unsigned long ultimoCalculo = 0;


  // Atualiza RPM a cada 100 ms
  if (millis() - ultimoCalculo >= 100) {


    // --------------------------------------------------------
    // Copia quantidade de pulsos
    // --------------------------------------------------------

    noInterrupts();

    unsigned long quantidade = pulsos;

    pulsos = 0;

    interrupts();


    // --------------------------------------------------------
    // Calcula pulsos por segundo
    // --------------------------------------------------------

    float pulsosPorSegundo =
      quantidade * 10.0;


    // --------------------------------------------------------
    // Calcula RPM
    // --------------------------------------------------------

    float rpmInstantaneo =
      (pulsosPorSegundo * 60.0)
      / PULSOS_POR_VOLTA;


    // --------------------------------------------------------
    // Filtro
    // --------------------------------------------------------

    rpmFiltrado =
      (rpmFiltrado * 0.40) +
      (rpmInstantaneo * 0.60);


    // --------------------------------------------------------
    // Procura o quadradinho correspondente
    // --------------------------------------------------------

    frequenciaAtual =
      converterRPM(rpmFiltrado);


    // --------------------------------------------------------
    // Manda a frequência correspondente para o painel
    // --------------------------------------------------------

    configurarFrequencia(frequenciaAtual);


    // --------------------------------------------------------
    // Monitor Serial
    // --------------------------------------------------------

    Serial.print("RPM REAL: ");
    Serial.print(rpmFiltrado, 0);

    Serial.print(" | QUADRADO: ");

    if (rpmFiltrado < 250) {

      Serial.print("-");

    } else {

      int indice =
        (int)((rpmFiltrado - 250.0) / 250.0);

      if (indice < 0)
        indice = 0;

      if (indice >= 44)
        indice = 43;

      Serial.print(indice);
    }

    Serial.print(" | Hz ENVIADO: ");
    Serial.println(frequenciaAtual, 3);


    ultimoCalculo = millis();
  }
}

// by qubitdot