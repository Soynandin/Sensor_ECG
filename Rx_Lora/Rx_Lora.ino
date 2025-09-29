#include <SPI.h>
#include <SX128XLT.h>
#include "Settings.h"

SX128XLT LoRa;

void setup() {
  Serial.begin(9600);
  SPI.begin();

  if (!LoRa.begin(NSS, NRESET, RFBUSY, DIO1, DIO2, DIO3, RX_EN, TX_EN, LORA_DEVICE)) {
      Serial.println("Erro ao iniciar LoRa!");
      while (1);
  }
    
  // Configuração LoRa (deve ser igual à do transmissor)
  LoRa.setupLoRa(Frequency, Offset, SpreadingFactor, Bandwidth, CodeRate);

  Serial.println("LoRa Rx iniciado!");
}

void loop() {
  char buffer[100] = {0};  // Buffer para armazenar a mensagem recebida
  uint8_t tamanhoRecebido = LoRa.receive((uint8_t *)buffer, sizeof(buffer), 0, WAIT_RX);

  if (tamanhoRecebido > 0) {
      Serial.print("Mensagem recebida: ");
      Serial.println(buffer);

      // Envia o ACK de volta ao transmissor
      const char *ack = "ACK";
      bool ackStatus = LoRa.transmit((uint8_t *)ack, strlen(ack), 0, TXpower, WAIT_TX);
     
      if (ackStatus) {
          Serial.println("ACK enviado!");
      } else {
          Serial.println("Falha ao enviar ACK!");
      }
  } else {
      Serial.println("Aguardando mensagem...");
  }

  delay(125);  // Pequena pausa antes de escutar novamente
}
