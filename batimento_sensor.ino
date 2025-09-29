const int SENSOR_PIN = A0;  // Pino do sensor AD8232
const int BUFFER_SIZE = 700; // Tamanho do buffer para capturar 700 amostras
int sensorValues[BUFFER_SIZE];  // Vetor para armazenar os dados do ciclo
int index = 0;  // Índice do vetor
bool capturing = false;  // Indica se está capturando
int threshold = 500;  // Limiar para detectar pico R
int lastPeakIndex = -1;  // Índice do último pico detectado
int firstPeakIndex = -1; // Índice do primeiro pico detectado
int midpointIndex = BUFFER_SIZE / 2;  // Meio do vetor de captura

void setup() {
    Serial.begin(9600);
}

void loop() {
    int value = analogRead(SENSOR_PIN); // Lê o valor do sensor

    // Detecta os picos R
    if (value > threshold) {
        if (lastPeakIndex == -1) {
            // Primeiro pico detectado
            lastPeakIndex = index;
        } else if (firstPeakIndex == -1) {
            // Segundo pico detectado
            firstPeakIndex = index;

            // Calcula o ponto médio entre os picos
            int midPoint = (lastPeakIndex + firstPeakIndex) / 2;

            // Inicia a captura com o pico no meio do vetor
            capturing = true;
            index = midPoint;  // Começa a captura a partir do meio do vetor
            Serial.print("Picos detectados! Iniciando captura no ponto médio: ");
            Serial.println(midPoint);
        }
    }

    // Inicia a captura de dados a partir do ponto médio e vai preenchendo o vetor
    if (capturing) {
        // Desloca os dados para a direita, mantendo os anteriores
        for (int i = BUFFER_SIZE - 1; i > 0; i--) {
            sensorValues[i] = sensorValues[i - 1];
        }

        // Armazena o valor lido no índice 0 do buffer
        sensorValues[0] = value;

        index++;  // Aumenta o índice

        // Preenche o vetor a partir do meio, de forma contínua
        if (index >= BUFFER_SIZE) {
            capturing = false;  // Para de capturar quando o buffer for preenchido
            Serial.println("Fim da captura! Exibindo os dados:");

            // Exibe os valores capturados
            for (int i = 0; i < BUFFER_SIZE; i++) {
                Serial.println(sensorValues[i]);
            }
            Serial.println("------ FIM DA CAPTURA ------");

            // Reseta as variáveis para o próximo ciclo de captura
            lastPeakIndex = -1;
            firstPeakIndex = -1;
        }
    }

    delay(1);
}
