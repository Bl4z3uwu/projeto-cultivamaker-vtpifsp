/* ----- Import das bibliotecas ----- */
#include <Arduino.h>
#include <DHT.h>
#include <WiFi.h>
#include <WebServer.h>

/* ----- Definição de constantes e variáveis ----- */
const int umdSoloPin = 1;
const int umdArPin = 2;
const int DHTPIN = 3;
const int DHT_TYPE = DHT11;
const int chuvaPin = 4;
const int luzPin = 5;

// Valores calibrados do sensor de umidade do solo
const int valorSeco = 4095;   // Valor no ar / solo totalmente seco
const int valorMolhado = 1500; // Valor dentro da água / solo encharcado


/* ----- Criação de objetos ----- */
DHT dht(DHTPIN, DHT_TYPE);


/* ----- Protótipos das funções ----- */
int lerUmidadeSolo();
int lerUmidadeAr();
int lerTemperatura();
boolean lerChuva();
int lerLuz();
void enviarParaServidor(int umidadeSolo, int umidadeAr, int temperatura, boolean estaChovendo, int luz);


/* ----- Setup ----- */
void setup() {
    // Definindo os modos dos pinos
    pinMode(umdSoloPin, INPUT);
    pinMode(umdArPin, INPUT);
    pinMode(DHTPIN, INPUT);
    pinMode(chuvaPin, INPUT);
    pinMode(luzPin, INPUT);

    // Inicializando a comunicação serial e o sensor DHT
    Serial.begin(115200);
    dht.begin();
}


/* ----- Loop ----- */
void loop() {
    int umidadeSolo = lerUmidadeSolo();
    int umidadeAr = lerUmidadeAr();
    int temperatura = lerTemperatura();
    boolean estaChovendo = lerChuva();
    int luz = lerLuz();

    enviarParaServidor(umidadeSolo, umidadeAr, temperatura, estaChovendo, luz);
}