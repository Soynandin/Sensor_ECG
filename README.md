# Projeto Sensor ECG

Este repositório contém o código-fonte de um sistema completo para **monitoramento de Eletrocardiograma (ECG)**. O sistema utiliza o sensor **AD8232** para aquisição de sinais biológicos e a tecnologia **LoRa 2.4 GHz** (transceptor SX128XLT) para a transmissão sem fio dos ciclos cardíacos.

O projeto é dividido em três partes lógicas: Aquisição, Transmissão e Recepção, visando o monitoramento remoto de sinais vitais.

---

## 🎯 Objetivo e Aplicações Científicas

O principal objetivo do projeto é **capturar um ciclo cardíaco completo** de forma confiável e enviá-lo por rádio, centralizando a janela de amostragem em torno do **Complexo QRS (Pico R)**.

Esta arquitetura é relevante para diversas **aplicações científicas** e de telemedicina:

* **Telemetria de Sinais Biológicos:** Estudar a viabilidade e a latência da transmissão de dados de ECG em ambientes remotos, aproveitando o longo alcance do LoRa.
* **Análise de Variação de Frequência Cardíaca (VFC):** A detecção precisa dos picos R é crucial para calcular o intervalo R-R, base para a análise da VFC.
* **Monitoramento em Condições Extremas:** Possibilidade de monitorar o ECG em indivíduos em movimento ou em locais com pouca infraestrutura de comunicação.

---

## 🔬 Batimento Cardíaco e ECG

O **Eletrocardiograma (ECG)** é um registro gráfico da **atividade elétrica do coração**, sendo um exame fundamental para o diagnóstico de diversas cardiopatias. O traçado representa a sequência de **despolarização** (contração) e **repolarização** (relaxamento) das células cardíacas.

### Componentes Chave do Ciclo Cardíaco (PQRST)

Cada batimento normal (em **Ritmo Sinusal**) é composto por um ciclo de ondas que seu projeto busca isolar e transmitir, de acordo com a eletrofisiologia básica:

| Componente | Evento Fisiológico | Relevância Clínica |
| :--- | :--- | :--- |
| **Onda P** | Despolarização Atrial | Representa a contração dos átrios. |
| **Complexo QRS** | Despolarização Ventricular | Representa a poderosa contração dos ventrículos. É a parte mais alta e estreita do traçado. |
| **Pico R** | Máxima Despolarização Ventricular | O **ponto de maior amplitude** (deflexão positiva) dentro do QRS. |
| **Onda T** | Repolarização Ventricular | Representa o relaxamento e recuperação dos ventrículos. |
| **Intervalo R-R** | Duração do Batimento | O tempo entre dois Picos R. Essencial para calcular a **Frequência Cardíaca (FC)** e a **Variação da Frequência Cardíaca (VFC)**. |

### Metodologia de Captura (Centralização do QRS)

O método de aquisição implementado no seu código é cientificamente orientado para garantir a qualidade dos dados:

1.  **Detecção do Pico R por Limiar (`threshold = 500`):**
    * O **Pico R** é a onda de **maior voltagem** e de maior energia no traçado. Utilizar um limiar é uma técnica de **baixo custo computacional** e altamente eficaz para isolar este evento (princípio simplificado de algoritmos como o Pan-Tompkins).
2.  **Centralização pelo Intervalo R-R:**
    * O código espera detectar **dois Picos R** para calcular o **ponto médio** entre eles. Esse `midPoint` é usado como ponto de partida da coleta.
    * Isso garante que a **janela de dados (o buffer de 700 amostras)** contenha toda a informação P-QRS-T de um ciclo cardíaco completo, crucial para análises de morfologia e diagnóstico.

O módulo **AD8232** é um amplificador de instrumentação biomédica de baixo consumo, fundamental para o projeto. Ele é usado para **extrair, amplificar e filtrar** os biopotenciais de ECG, fornecendo um sinal analógico mais limpo para processamento e transmissão.

---

## 📂 Estrutura do Repositório

| Arquivo/Pasta | Descrição |
| :--- | :--- |
| `Tx_Lora/` | Contém o código do **Transmissor (Tx)**: Adquire o ECG, detecta o ciclo, e o empacota para envio via LoRa. |
| `Rx_Lora/` | Contém o código do **Receptor (Rx)**: Recebe os pacotes LoRa, imprime-os e envia um *ACK* de volta. |
| `Settings.h` | Arquivo de cabeçalho compartilhado com as configurações de hardware (pinos) e de modulação LoRa (Frequência, SF, BW). |

---

## ⚡ Lógica de Aquisição do ECG (Batimento e Centralização)

O sistema é projetado para isolar uma **janela de 700 amostras** (*buffer*), garantindo que o ciclo cardíaco completo seja capturado e centralizado.

### 1. Variáveis Globais e Configuração

| Variável/Constante | Valor | Função |
| :--- | :--- | :--- |
| **`SENSOR_PIN`** | `A0` | Pino analógico de leitura do sensor AD8232. |
| **`BUFFER_SIZE`** | `700` | Tamanho do vetor que armazena as amostras (700 milissegundos a 1000Hz). |
| **`threshold`** | `500` | Valor limite para detecção do **Pico R**. |
| **`capturing`** | `false` | *Flag* booleana que indica o modo de captura ativa. |
| **`lastPeakIndex`, `firstPeakIndex`** | `-1` | Índices usados para rastrear a posição do primeiro e segundo Pico R. |

A função `setup()` é usada para inicializar a comunicação serial (`Serial.begin(9600)`) e o módulo LoRa.

### 2. Loop Principal (`loop`)

#### A. Detecção de Picos R

1.  O código espera pacientemente que **dois Picos R consecutivos** sejam detectados (leituras acima do `threshold`).
2.  Ao detectar o segundo pico, ele calcula o **ponto médio (`midPoint`)** entre os dois picos R.
3.  O contador **`index` é resetado para `midPoint`**, e a *flag* `capturing` é definida como `true`. Este é o ponto inicial da coleta de dados, garantindo a centralização do QRS no *buffer* final.

#### B. Captura de Dados (Buffer Deslizante)

No código de aquisição pura (referência `batimento_sensor.ino`), o sistema usa um **buffer deslizante (FIFO)**:

* Os dados existentes são **deslocados** para a direita.
* A leitura mais recente é inserida na posição `sensorValues[0]`.
* Quando o `index` atinge **700**, a captura é finalizada, o vetor está preenchido com as 700 amostras imediatamente subsequentes ao ponto de partida `midPoint`.

---

## 📡 Detalhe da Transmissão LoRa (`Tx_Lora`)

O código do transmissor implementa a captura e a transmissão multienvio (fragmentação):

### 1. Captura e Empacotamento

* Após a detecção dos picos R, o código entra em um loop **bloqueante** para capturar as 700 amostras e as converte em uma única *string* (`mensagemCompleta`), separada por ponto e vírgula (`;`).

### 2. Fragmentação dos Dados

* A *string* completa é muito grande para um único pacote LoRa (limite de **90 bytes** definido por `PACKET_SIZE`).
* O código divide a `mensagemCompleta` em múltiplos pacotes.
* Cada pacote recebe um **cabeçalho** no formato **`XXX/YYY:`** (ex: `001/005:`) para indicar o número do pacote atual e o total de pacotes na sequência.

### 3. Envio e Atraso

* Cada fragmento é enviado sequencialmente via `LoRa.transmit()`.
* Um pequeno **`delay(250)`** é inserido entre os envios dos pacotes para dar tempo ao receptor de processar a informação.

---

## 🛰️ Detalhe da Recepção LoRa (`Rx_Lora`)

O código do receptor está configurado para o modo de escuta:

1.  **Recepção:** O `LoRa.receive()` aguarda por um pacote.
2.  **Impressão:** O conteúdo do pacote é impresso na Serial (`Serial.println(buffer)`).
3.  **Confirmação (ACK):** Após receber um pacote, o receptor envia uma mensagem de confirmação **"ACK"** de volta ao transmissor, indicando que a transmissão foi bem-sucedida para aquele fragmento.
