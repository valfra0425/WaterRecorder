// biblioteca de acesso wifi
#include <ESP8266WiFi.h>
// biblioteca de servidor http | aceita apenas um cliente simultâneo 
#include <ESP8266WebServer.h>
// pinos de entrada do sensor ultrasonico
const int trigPin = 2;
const int echoPin = 0;
// pino do led, que aciona a bomba para encher a caixa d'agua
const int ledPin = 5;
// variáveis para o acesso da rede wi-fi
const char* ssid = "iPhone (2)";
const char* password = "12345678";
// variáveis para o calculo do sonar
long duration;
int distance;
19
// variável de capacidade da caixa de água
// essa variável deve ser mudada de acordo com a caixa d'água utilizada
int capacidade = 22;
// variável que corresponde a porcentagem atual da caixa d'agua
float porc;
// criação de uma instância do servidor na porta 80
ESP8266WebServer server(80);

// função de controle da bomba de água
void ligar() {
    if (digitalRead(ledPin) == HIGH){
        digitalWrite(ledPin,LOW);
    }
    else{
        digitalWrite(ledPin,HIGH);
    }
}

// função de ligar é adicionada ao servidor
void handleLigar() {
    ligar();
    server.send(200, "text/plain", "OK");
}

// essa função é um manipulador de solicitações para a raiz do servidor web
void handleRoot() {

    // variavel de controle da bomba de água
    String bomba;
    if (digitalRead(ledPin) == HIGH){
        bomba = "ligada";
    }
    else{
        bomba = "desligada";
    }
    
    // html do server
    String message = "<html>";
    message += "<meta http-equiv=\'content-type\' content=\'text/html; charset=utf-8\'>";
    message += "<meta http-equiv='refresh' content='5'>";
    message += "<h1>Central de controle da Caixa de água</h1>";
    message += "<p> O nivel da caixa é: "+ String(porc) + "%</p>";
    message += "<p> A bomba está: "+ bomba +"</p>";
    message += "<button type='button' onclick=\"window.location.href='/ligar'\"> ativar </button>";
    message += "</html>";

    server.send(200, "text/html", message);
}

// função setup executada quando o programa é iniciado
void setup() {
    // porta serial de comunicação utilizada
    Serial.begin(9600);

    // definição das variáveis de entrade e saida do ESP com tigger saída e echo entrada
    pinMode(trigPin, OUTPUT);
    pinMode(echoPin, INPUT);
    // variável que representará o led/bomba de água
    pinMode(ledPin, OUTPUT);

    // aqui a conexão wi-fi tenta ser iniciada com nome da rede e a senha fornecida acima
    WiFi.begin(ssid, password);

    // laço de repetição que só e encerrado depois do status da conexão for conectada
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Conectando ao Wi-Fi...");
    }

    // informações da conexão de rede
    Serial.println("Medidor de distancia com RCW-0001");
    Serial.println("Conectado ao Wi-Fi!");
    Serial.print("Endereço IP: ");
    Serial.println(WiFi.localIP());

    // o servidor registra a função handleRoot para acessar a raiz do servidor(mesmo endereço ip do informado anteriormente)
    // exemplo: http://192.168.1.100/
    server.on("/", handleRoot);
    server.on("/ligar", handleLigar);

    // servidor iniciado para aceitar requisições 
    server.begin();
    Serial.println("Servidor iniciado!");
}

void loop() {
    // aqui permite que o servidor aceite requisições dos usuário
    server.handleClient();

    // deixando o trigger em sinal baixo para manter 
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);

    // define o trigger em sinal alto por 10 micro segundos
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);

    // aqui é cauculada a duração  da volta da onda para o echo, através da mudança de valor para alto
    // o valor esta em microsegundos
    duration = pulseIn(echoPin, HIGH);

    // este é o calculo da distância em cm
    // o calculo é a duração vezes a velocidade do som em cm/microsegundos
    // depois e dividido por 2 porque queremos só a médida da volta do sinal e não ida e volta
    distance= duration*0.034/2;

    // Cálculo da porcentagem atual da caixa d'agua 
    porc =  ((100 *(capacidade - distance))/capacidade);

    // exibição da medida do medidor serial
    Serial.print("Nivel: "+ String(porc) +"%");
    if(distance < capacidade*0.15){
        digitalWrite(ledPin,LOW);
        Serial.println(" mais de 85% a bomba será desligada");
    }
    else if(distance < capacidade*0.25){
        Serial.println(" mais de 75%");
    }
    else if(distance < capacidade*0.5 ){
        Serial.println(" mais de 50%");
    }
    else if(distance < capacidade*0.75 ){
        Serial.println(" mais de 25%");
    }
    else if(distance < capacidade*0.9){
        Serial.println(" mais de 10%");
    }
    else{
        digitalWrite(ledPin,HIGH);
        Serial.println(" abaixo de 10%, a bomba será ligada");
    }
    // intervalo entre cada medição
    delay(3000);
}