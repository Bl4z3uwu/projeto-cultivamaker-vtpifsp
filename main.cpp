/* ----- Import das bibliotecas ----- */
#include <Arduino.h>
#include <DHT.h>
#include <WiFi.h>
#include <WebServer.h>

/* ----- Definição de constantes e variáveis ----- */
const int umdSoloPin = 34;
const int bombaPin = 27;
const int DHTPIN = 4;
const int DHT_TYPE = DHT11;
const int chuvaPin = 33;
const int luzPin = 35;

// Valores calibrados do sensor de umidade do solo
const int valorSeco = 4095;   
const int valorMolhado = 1500;

// Variáveis de Controle da Bomba
int umidadeMinima = 30; // Liga a bomba se cair abaixo disso
int umidadeMaxima = 40; // Só desliga quando atingir isso
bool regandoAutomatico = false;

// Controle Manual Anti-Spam
bool regaManualAtiva = false;
unsigned long tempoFimRegaManual = 0;
unsigned long tempoUltimoAcionamento = 0;
const unsigned long COOLDOWN = 15000;     // 15 segundos de bloqueio

/* ----- Criação de objetos ----- */
DHT dht(DHTPIN, DHT_TYPE);
WebServer server(80); // Servidor web na porta padrão HTTP (80)

/* ----- Protótipos das funções ----- */
int lerUmidadeSolo();
int lerUmidadeAr();
int lerTemperatura();
boolean lerChuva();
int lerLuz();
void handleRoot();
void handleDados();
void handleConfig();
void handleRegar();

/* ----- Setup ----- */
void setup() {
    Serial.begin(115200);
    
    // Configuração dos Pinos
    pinMode(umdSoloPin, INPUT);
    pinMode(DHTPIN, INPUT);
    pinMode(chuvaPin, INPUT);
    pinMode(luzPin, INPUT);

    // Configuração da Bomba
    pinMode(bombaPin, OUTPUT);
    digitalWrite(bombaPin, HIGH); 

    // Inicializando o sensor DHT
    dht.begin();

    // Criando a Própria Rede Wi-Fi (Access Point)
    Serial.println("Iniciando rede Wi-Fi própria...");
    WiFi.softAP("Horta_Inteligente", "12345678"); // Nome da rede e Senha (mínimo 8 caracteres)
    
    IPAddress IP = WiFi.softAPIP();
    Serial.print("Rede criada! IP para o QR Code: ");
    Serial.println(IP); // Por padrão, será 192.168.4.1

    // Rotas do Servidor Web
    server.on("/", handleRoot);         // Quando acessar a página principal
    server.on("/dados", handleDados);   // Rota invisível que envia os dados dos sensores
    server.on("/config", handleConfig); // Rota para mudar o limite de umidade
    server.on("/regar", handleRegar);   // Rota para o botão de rega manual

    // Ligando o Servidor
    server.begin();
    Serial.println("Servidor HTTP iniciado!");
}

/* ----- Loop ----- */
void loop() {
    server.handleClient(); 

    int umidadeSolo = lerUmidadeSolo();
    boolean estaChovendo = lerChuva();
    unsigned long tempoAtual = millis();

    // Controle da Bomba com Modo Manual e Automático
    if (regaManualAtiva) {
        // Modo Manual: só desliga quando o tempo acabar
        if (tempoAtual < tempoFimRegaManual) {
            digitalWrite(bombaPin, LOW); 
        } else {
            regaManualAtiva = false;      
            digitalWrite(bombaPin, HIGH); 
        }
    } else {
        // Modo Automático: só liga a bomba se cair abaixo do mínimo e só desliga quando atingir o máximo
        if (umidadeSolo <= umidadeMinima && estaChovendo == false) {
            regandoAutomatico = true;
        } 
        else if (umidadeSolo >= umidadeMaxima || estaChovendo == true) {
            regandoAutomatico = false;
        }

        // Executa a ação baseada na memória
        if (regandoAutomatico) {
            digitalWrite(bombaPin, LOW); // Liga
        } else {
            digitalWrite(bombaPin, HIGH); // Desliga
        }
    }

    delay(2);
}

/* ----- Funções ----- */
int lerUmidadeSolo() {
    int valorAnalogico = analogRead(umdSoloPin);
    // Limita o valor entre 1500 e 4095 antes de mapear para evitar porcentagens negativas
    valorAnalogico = constrain(valorAnalogico, valorMolhado, valorSeco); 
    return map(valorAnalogico, valorSeco, valorMolhado, 0, 100);
}

int lerUmidadeAr() {
    return dht.readHumidity();
}

int lerTemperatura() {
    return dht.readTemperature();
}

boolean lerChuva() {
    // O sensor envia LOW (0) quando a água fecha o curto na placa
    return (digitalRead(chuvaPin) == LOW); 
}

int lerLuz() {
    int valorAnalogico = analogRead(luzPin);
    return map(valorAnalogico, 0, 4095, 0, 100); 
}

// Envia a página HTML para o navegador (rota /)
void handleRoot() {
    String html = R"=====(
    <!DOCTYPE html>
    <html lang="pt-BR">
    <head>
        <meta charset="UTF-8">
        <meta name="viewport" content="width=device-width, initial-scale=1.0">
        <title>CultivaMaker - Dashboard</title>
        <style>
            :root { --bg-color: #f4f7f6; --card-bg: #ffffff; --primary-color: #2e7d32; --text-color: #333333; --shadow: 0 4px 6px rgba(0,0,0,0.1); }
            body { font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; background-color: var(--bg-color); color: var(--text-color); margin: 0; padding: 20px; display: flex; flex-direction: column; align-items: center; }
            header { text-align: center; margin-bottom: 30px; }
            h1 { color: var(--primary-color); margin-bottom: 5px; }
            .dashboard-container { display: grid; grid-template-columns: repeat(auto-fit, minmax(180px, 1fr)); gap: 20px; width: 100%; max-width: 1000px; margin-bottom: 20px; }
            .card { background-color: var(--card-bg); border-radius: 12px; padding: 20px; text-align: center; box-shadow: var(--shadow); transition: transform 0.2s ease-in-out; }
            .card:hover { transform: translateY(-3px); }
            .icon { font-size: 2.5rem; margin-bottom: 10px; }
            .card h2 { font-size: 1.1rem; margin: 10px 0; color: #555; }
            .value { font-size: 1.8rem; font-weight: bold; color: var(--primary-color); }
            
            /* Estilos do Painel de Controle */
            .control-panel { background-color: #e8f5e9; grid-column: 1 / -1; }
            .slider { width: 80%; margin: 15px 0; }
            .btn { background-color: #2e7d32; color: white; border: none; padding: 12px 24px; border-radius: 8px; font-size: 1.1rem; cursor: pointer; transition: background 0.3s; font-weight: bold; box-shadow: 0 2px 4px rgba(0,0,0,0.2); }
            .btn:hover { background-color: #1b5e20; }
            .btn:active { transform: scale(0.98); }
        </style>
    </head>
    <body>
        <header>
            <h1>Dashboard CultivaMaker 🌿</h1>
            <p>Monitoramento e Controle</p>
        </header>

        <div class="dashboard-container">
            <!-- Os 5 Cards de Monitoramento -->
            <div class="card"><div class="icon">🌱</div><h2>Umidade do Solo</h2><div class="value" id="val-umidade-solo">--%</div></div>
            <div class="card"><div class="icon">💧</div><h2>Umidade do Ar</h2><div class="value" id="val-umidade-ar">--%</div></div>
            <div class="card"><div class="icon">🌡️</div><h2>Temperatura</h2><div class="value" id="val-temperatura">--°C</div></div>
            <div class="card"><div class="icon" id="icon-luz">☀️</div><h2>Luminosidade</h2><div class="value" id="val-luz" style="font-size: 1.4rem;">--</div></div>
            <div class="card"><div class="icon" id="icon-chuva">🌤️</div><h2>Status de Chuva</h2><div class="value" id="val-chuva" style="font-size: 1.4rem;">Sem Chuva</div></div>
            
            <!-- Painel de Controle -->
            <div class="card control-panel">
                <h2>⚙️ Sistema de Irrigação</h2>
                
                <p style="font-size: 1.1rem; margin-top: 15px;">
                    Nível Mínimo: <strong id="texto-min" style="color: #d32f2f;">30%</strong>
                </p>
                <!-- Slider travado com 'disabled' -->
                <input type="range" min="0" max="100" value="30" class="slider" id="slider-min" onchange="enviarLimiares()" disabled>
                
                <p style="font-size: 1.1rem; margin-top: 15px;">
                    Nível Máximo: <strong id="texto-max" style="color: var(--primary-color);">40%</strong>
                </p>
                <!-- Slider travado com 'disabled' -->
                <input type="range" min="0" max="100" value="40" class="slider" id="slider-max" onchange="enviarLimiares()" disabled>
                
                <br><br>
                <button class="btn" id="btn-regar" onclick="acionarRega()">💧 Regar Manualmente (1s)</button>
            </div>
        </div>
        <script>
            function atualizarDashboard(umidadeSolo, umidadeAr, temperatura, estaChovendo, luz) {
                // Atualizações simples
                document.getElementById('val-umidade-solo').innerText = umidadeSolo + '%';
                document.getElementById('val-umidade-ar').innerText = umidadeAr + '%';
                document.getElementById('val-temperatura').innerText = temperatura + '°C';
                
                // Lógica de Status da Chuva
                if (estaChovendo) {
                    document.getElementById('val-chuva').innerText = "Chovendo";
                    document.getElementById('val-chuva').style.color = "#0277bd";
                    document.getElementById('icon-chuva').innerText = "🌧️";
                } else {
                    document.getElementById('val-chuva').innerText = "Sem Chuva";
                    document.getElementById('val-chuva').style.color = "#f57c00";
                    document.getElementById('icon-chuva').innerText = "🌤️";
                }

                // Lógica Dinâmica da Luminosidade (LDR)
                let luzTexto = "";
                let luzIcon = "";
                if (luz < 20) {
                    luzTexto = "Escuro";
                    luzIcon = "🌙";
                } else if (luz < 60) {
                    luzTexto = "Luz Natural";
                    luzIcon = "🔅";
                } else {
                    luzTexto = "Sol Direto";
                    luzIcon = "☀️";
                }
                document.getElementById('val-luz').innerText = luzTexto + ' (' + luz + '%)';
                document.getElementById('icon-luz').innerText = luzIcon;
            }

            // Função do Slider
            function enviarLimiar(valor) {
                document.getElementById('texto-limiar').innerText = valor + '%';
                fetch('/config?limiar=' + valor);
            }

            // Função do Botão de Rega
            function acionarRega() {
            let btn = document.getElementById('btn-regar');
            
            // Se o botão já estiver desativado, o clique não faz nada
            if (btn.disabled) return;

            // Dispara a requisição pro ESP32
            fetch('/regar');
            
            // Bloqueia o botão instantaneamente e muda o visual pra água
            btn.disabled = true;
            btn.innerText = "💦 Regando...";
            btn.style.backgroundColor = "#0277bd";
            
            // Depois de 1 segundo (tempo da rega), bloqueia o botão no frontend
            setTimeout(() => {
                btn.innerText = "🚫 Regue apenas uma vez por vez.";
                btn.style.backgroundColor = "#9e9e9e"; // Fica cinza
                btn.style.cursor = "not-allowed";
            }, 1000);
        }

            // Busca os dados do ESP32 a cada 2 segundos
            setInterval(() => {
                fetch('/dados')
                .then(response => response.json())
                .then(data => {
                    atualizarDashboard(data.umidadeSolo, data.umidadeAr, data.temperatura, data.estaChovendo, data.luz);
                })
                .catch(error => console.error("Erro:", error));
            }, 2000);
        </script>
    </body>
    </html>
    )=====";
    
    server.send(200, "text/html", html);
}

// Envia o JSON com as leituras dos sensores (rota /dados)
void handleDados() {
    String json = "{";
    json += "\"umidadeSolo\":" + String(lerUmidadeSolo()) + ",";
    json += "\"umidadeAr\":" + String(lerUmidadeAr()) + ",";
    json += "\"temperatura\":" + String(lerTemperatura()) + ",";
    json += "\"estaChovendo\":" + String(lerChuva() ? "true" : "false") + ",";
    json += "\"luz\":" + String(lerLuz());
    json += "}";

    server.send(200, "application/json", json);
}

// Recebe os novos limites do site
void handleConfig() {
    if (server.hasArg("min") && server.hasArg("max")) {
        int novoMin = server.arg("min").toInt();
        int novoMax = server.arg("max").toInt();

        // Trava de segurança: O mínimo nunca pode ser maior ou igual ao máximo
        if (novoMin < novoMax) {
            umidadeMinima = novoMin;
            umidadeMaxima = novoMax;
            Serial.println("Novos limites -> Liga: " + String(umidadeMinima) + "% | Desliga: " + String(umidadeMaxima) + "%");
        } else {
            Serial.println("Erro: Usuário tentou colocar o mínimo maior que o máximo!");
        }
    }
    server.send(200, "text/plain", "OK");
}

// Ativa a rega manual por 1 segundo
void handleRegar() {
    unsigned long tempoAtual = millis();

    // Proteção também no Backend: Se apertar antes dos 15s de cooldown, o ESP32 recusa
    if (tempoAtual - tempoUltimoAcionamento < COOLDOWN && tempoUltimoAcionamento != 0) {
        server.send(429, "text/plain", "Pedimos encarecidamente que não regue mais de uma vez na apresentação. Obrigado(a) pela compreensão!");
        return;
    }

    // Se passou pela defesa, libera a água por 1 segundo
    regaManualAtiva = true;
    tempoUltimoAcionamento = tempoAtual; 
    tempoFimRegaManual = tempoAtual + 1000; 
    
    Serial.println("Rega manual de 1s ativada!");
    server.send(200, "text/plain", "OK");
}
