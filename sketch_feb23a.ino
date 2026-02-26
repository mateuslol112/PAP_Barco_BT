#include <Servo.h>

// ----- PINOS -----
#define ENA 5     // Enable Motor A (PWM)
#define IN1 2     // Motor A
#define IN2 3

#define ENB 6     // Enable Motor B (PWM)
#define IN3 4     // Motor B
#define IN4 7

#define SERVO_PIN 9  // Leme
#define BATTERY_PIN A0 // Para monitor de bateria

// ----- OBJETOS -----
Servo leme;

// ----- VARIÁVEIS -----
int velocidadeMax = 255;  // 0-255
char comando;

unsigned long tempoMotorFrente = 0; // milissegundos
unsigned long inicioFrente = 0;
bool motorFrenteLigado = false;

float velocidadeBarco = 0.5; // metros por segundo (ajusta conforme barco)
float tensaoBateria = 0;

// Para leme suave
int lemeAlvo = 90;
int lemeAtual = 90;

void setup() {
  // Motores
  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);

  pinMode(ENB, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  // Servo
  leme.attach(SERVO_PIN);
  leme.write(lemeAtual);

  // Serial
  Serial.begin(9600);
  Serial.println("Barco RC pronto! Aguarda comandos...");
}

void loop() {
  // Leitura de comando Bluetooth
  if (Serial.available()) {
    comando = Serial.read();
    controleBarco(comando);
  }

  // Atualiza monitor de bateria e odometro a cada segundo
  static unsigned long ultimaLeitura = 0;
  if (millis() - ultimaLeitura > 1000) {
    ultimaLeitura = millis();
    medeBateria();
    mostraDistancia();
    mostraEstado();
  }

  // Leme suave (volta ao centro)
  if (lemeAtual != lemeAlvo) {
    if (lemeAtual < lemeAlvo) lemeAtual++;
    else if (lemeAtual > lemeAlvo) lemeAtual--;
    leme.write(lemeAtual);
    delay(10); // suaviza movimento
  }
}

// ----- FUNÇÃO DE CONTROLE -----
void controleBarco(char cmd) {
  switch (cmd) {
    case 'F': frente(); break;
    case 'B': tras(); break;
    case 'L': lemeAlvo = 45; break;  // leme esquerda
    case 'R': lemeAlvo = 135; break; // leme direita
    case 'S': 
      parar(); 
      lemeAlvo = 90; // volta ao centro
      break;
    case 'C': lemeAlvo = 90; break;  // centraliza leme
  }
}

// ----- MOVIMENTOS -----
void frente() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  analogWrite(ENA, velocidadeMax);

  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  analogWrite(ENB, velocidadeMax);

  inicioFrente = millis();
  motorFrenteLigado = true;
}

void tras() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  analogWrite(ENA, velocidadeMax);

  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  analogWrite(ENB, velocidadeMax);

  // Conta apenas frente para odometro
  if (motorFrenteLigado) {
    tempoMotorFrente += millis() - inicioFrente;
    motorFrenteLigado = false;
  }
}

void parar() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  analogWrite(ENA, 0);

  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
  analogWrite(ENB, 0);

  if (motorFrenteLigado) {
    tempoMotorFrente += millis() - inicioFrente;
    motorFrenteLigado = false;
  }
}

// ----- BATERIA -----
void medeBateria() {
  int leitura = analogRead(BATTERY_PIN);
  tensaoBateria = leitura * (5.0 / 1023.0) * 2; // ajustar divisor
  Serial.print("Bateria: ");
  Serial.print(tensaoBateria);
  Serial.print(" V");
  if (tensaoBateria < 6.0) Serial.print(" ⚠ Bateria baixa!");
  Serial.println();
}

// ----- DISTÂNCIA -----
void mostraDistancia() {
  float distancia = (tempoMotorFrente / 1000.0) * velocidadeBarco; // metros
  Serial.print("Distancia percorrida: ");
  Serial.print(distancia);
  Serial.println(" m");

  float velocidadeAtual = velocidadeBarco; // aqui é constante, mas dá pra estimar com encoders depois
  Serial.print("Velocidade estimada: ");
  Serial.print(velocidadeAtual);
  Serial.println(" m/s");
}

// ----- ESTADO -----
void mostraEstado() {
  Serial.print("Leme: ");
  Serial.println(lemeAtual);
  Serial.println("---------------------");
}