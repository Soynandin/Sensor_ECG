#include <SPI.h>
#include <SX128XLT.h>
#include "Settings.h"

SX128XLT LoRa;

#define TIMEOUT 1000    // Tempo máximo de espera pelo ACK (1s)
#define PACKET_SIZE 90  // Tamanho máximo permitido por pacote
#define BUFFER_SIZE 700 // Tamanho do buffer para capturar 700 amostras

const int SENSOR_PIN = A0;  // Pino do sensor AD8232

void setup() {
    Serial.begin(9600);
    SPI.begin(); 
    
    if (!LoRa.begin(NSS, NRESET, RFBUSY, DIO1, DIO2, DIO3, RX_EN, TX_EN, LORA_DEVICE)) {
        Serial.println("Erro ao iniciar LoRa!");
        while (1);
    }
    
    LoRa.setupLoRa(Frequency, Offset, SpreadingFactor, Bandwidth, CodeRate);
    Serial.println("LoRa Tx iniciado!");
}

void loop() {
    int sensorValues[BUFFER_SIZE];  // Agora o vetor é LOCAL
    int index = 0;
    bool capturing = false;
    int threshold = 500;
    int lastPeakIndex = -1;
    int firstPeakIndex = -1;
    String mensagemCompleta = "";

    while (!capturing) {
        int value = analogRead(SENSOR_PIN);

        if (value > threshold) {
            if (lastPeakIndex == -1) {
                lastPeakIndex = index;
            } else if (firstPeakIndex == -1) {
                firstPeakIndex = index;
                capturing = true;
                index = (lastPeakIndex + firstPeakIndex) / 2;
            }
        }
    }

    // Captura os 700 valores e os armazena na string
    for (int i = 0; i < BUFFER_SIZE; i++) {
        sensorValues[i] = analogRead(SENSOR_PIN);
        
        char valorStr[6];  // Buffer para armazenar a string do número
        itoa(sensorValues[i], valorStr, 10);  // Converte para string

        // Adiciona os valores convertidos à mensagem, mantendo a ordem de leitura
        mensagemCompleta += String(valorStr) + ";";
        delay(1);
    }

    Serial.println("Fim da captura! Enviando os dados...");
    int tamanhoMensagem = mensagemCompleta.length();
    int pacotesTotais = (tamanhoMensagem + PACKET_SIZE - 1) / PACKET_SIZE;

    for (int i = 0; i < pacotesTotais; i++) {
        char pacote[PACKET_SIZE + 10];
        sprintf(pacote, "%03d/%03d:", i + 1, pacotesTotais);
        int cabecalhoTamanho = strlen(pacote);
        strncpy(pacote + cabecalhoTamanho, mensagemCompleta.c_str() + (i * PACKET_SIZE), PACKET_SIZE);
        pacote[cabecalhoTamanho + PACKET_SIZE] = '\0';

        bool statusEnvio = LoRa.transmit((uint8_t *)pacote, strlen(pacote), 0, TXpower, WAIT_TX);

        if (statusEnvio) {
            Serial.print("Pacote ");
            Serial.print(i + 1);
            Serial.print(" de ");
            Serial.print(pacotesTotais);
            Serial.println(" enviado com sucesso.");
        } else {
            Serial.print("Falha ao enviar o pacote ");
            Serial.println(i + 1);
        }

        delay(250);
    }

    Serial.println("Mensagem completa enviada!");
}
