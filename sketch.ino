// Projeto 1 - MicroControladores
// Autor: Gustavo Oliveira da Silva
/*
- No estado S0, o primeiro semáforo está em vermelho e o segundo em verde.
- No estado S1, o primeiro semáforo está em vermelho e o segundo em amarelo.
- No estado S2, o primeiro semáforo está em verde e o segundo em vermelho.
- No estado S3, o primeiro semáforo está em amarelo e o segundo em vermelho.
- No estado S4 em que os dois semáforos ficam com o LED amarelo piscando.
Este estado somente é acionado pelo envio da palavra “amarelo” pela
comunicação Serial. Ao enviar a palavra “semáforo”, o Arduino volta executar o
semáforo.
- Ao pressionar algum botão, será liberado após o tempo do sinal amarelo, a
travessia do pedestre daquela rua.
*/

// Inclusão da biblioteca para uso do Display de Sete Segmentos
#include <SevenSeg.h>

// Define os Temporizadores do Semáforo
#define RED 15
#define GREEN 12
#define YELLOW RED-GREEN

// Setando variaveis para o Display 1
SevenSeg disp1(22, 23, 24, 25, 26, 27, 28);
const int numOfDigits1 = 2;
int digitPins1 [numOfDigits1] = {4, 5};
int countD1 = 0;

// Setando variaveis para o Display 2
SevenSeg disp2(42, 43, 44, 45, 46, 47, 48);
const int numOfDigits2 = 2;
int digitPins2 [numOfDigits2] = {6, 7};
int countD2 = 0;

// Setando variaveis de controle de tempo
static unsigned long timer = millis();
int count = 0;

// Setando variaveis para os botões
int Bottom1 = 2;
int Bottom2 = 3;

// Ponteiro de função para alternancia dos estados do semáforo
void (*fp)();

// S0 - Semáforo 1 vermelho e Semáforo 2 verde
void S0()
{  
  PORTC = 0x14;
  PORTB = 0x09;

  disp1.write(countD1);
  disp2.write(countD2);

  if (millis() - timer >= 1000)
  {
    timer += 1000;

    countD1--;
    countD2--;
    count++;
  }
  
  if(count == GREEN)
  {
    fp = S1;
    count = 0;
    countD1 = YELLOW;
    countD2 = 0;
    timer = millis();
  }
}

// S1 - Semáforo 1 vermelho e Semáforo 2 amarelo
void S1()
{
  PORTC = 0x12;
  PORTB = 0x09;

  disp1.write(countD1);
  
  if (millis() - timer >= 1000)
  {
    timer += 1000;

    countD1--;
    count++;
  }
  
  if(count == YELLOW)
  {
    fp = S2;
    count = 0;
    countD1 = GREEN;
    countD2 = RED;
    timer = millis();
  }
}

// S2 - Semáforo 1 verde e Semáforo 2 vermelho
void S2()
{
  PORTC = 0x41;
  PORTB = 0x06;
  
  disp1.write(countD1);
  disp2.write(countD2);
  
  if (millis() - timer >= 1000)
  {
    timer += 1000;

    countD1--;
    countD2--;
    count++;
  }
  
  if(count == GREEN)
  {
    fp = S3;
    count = 0;
    countD1 = 0;
    countD2 = YELLOW;
    timer = millis();
  }
}

// S3 - Semáforo 1 amarelo e Semáforo 2 vermelho
void S3()
{
  PORTC = 0x21;
  PORTB = 0x06;

  disp2.write(countD2);
  
  if (millis() - timer >= 1000)
  {
    timer += 1000;

    countD2--;
    count++;
  }

  if(count == YELLOW)
  {
    fp = S0;
    count = 0;
    countD1 = RED;
    countD2 = GREEN;
    timer = millis();
  }
}

// S4 - Semáforo 1 e 2 piscando amarelo
void S4()
{
  PORTC = 0x22;
  PORTB = 0x05;
  delay(700);
  PORTC = 0x00;
  PORTB = 0x00;
  delay(700);
}

// Leitura serial para incio e fim do estado S4
void amarelo()
{
  if (Serial.available() > 0)
  {
    String entrada;
    entrada = Serial.readString();

    if(entrada == "amarelo\n")
    {
      Serial.print(entrada);
      fp = S4;
    }
    if(entrada == "semáforo\n")
    {
      Serial.print(entrada);
      fp = S0;
      count = 0;
      countD1 = RED;
      countD2 = GREEN;
      timer = millis();
    }
  }
}

// Interrupção 1 - Liberação para o pedestre do cruzamento 1
void interrupt_1()
{
  if((fp != S0) && (fp != S1) && (fp != S3))
  {
    fp = S3;
    count = 0;
    countD1 = 0;
    countD2 = YELLOW;
    timer = millis();
  }
}

// Interrupção 2 - Liberação para o pedestre do cruzamento 2
void interrupt_2()
{
  if((fp != S1) && (fp != S2) && (fp != S3))
  {
    fp = S1;
    count = 0;
    countD1 = YELLOW;
    countD2 = 0;
    timer = millis();
  }
}

void setup()
{
  // Inicia porta serial
  Serial.begin(9600);
  Serial.setTimeout(50);

  // Config do Display1
  disp1.setDigitPins(numOfDigits1, digitPins1);
  disp1.setRefreshRate(1);

  // Config do Display2
  disp2.setDigitPins(numOfDigits2, digitPins2);
  disp2.setRefreshRate(1);

  // Inicia port registers presentes
  DDRB = 0x0F;
  DDRC = 0x77;

  // Pinmode dos botões como INPUT
  pinMode(Bottom1, INPUT);
  pinMode(Bottom2, INPUT);

  // Declaração da Interrupção 1 e 2
  attachInterrupt(digitalPinToInterrupt(2), interrupt_1, RISING);
  attachInterrupt(digitalPinToInterrupt(3), interrupt_2, RISING);

  // Preparação das variaveis para o modo S0
  fp = S0;
  countD1 = RED;
  countD2 = GREEN;
  timer = millis();
}

void loop()
{
  // Roda o estado atual do semáforo
  (*fp)();

  // Roda a função amarelo
  amarelo();
}