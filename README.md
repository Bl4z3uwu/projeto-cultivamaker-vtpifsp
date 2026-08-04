# 🌿 Projeto CultivaMaker: Horta Inteligente no IFSP Votuporanga

![Status](https://img.shields.io/badge/Status-Em%20Desenvolvimento-green)
![Hardware](https://img.shields.io/badge/Hardware-Arduino%20%7C%20ESP32-blue)

## Sobre o Projeto

Este repositório contém o código-fonte do projeto da criação de uma horta inteligente no IFSP. O projeto base foi elaborado após uma Oficina de Automação com a ajuda dos mesmos. O projeto possui uma alta capacidade de expansão, sendo disponibilizado em modelo de código aberto para futuras colaborações.

## Funcionalidades do Sistema

- **Controle Físico (Hardware):** Leitura contínua de sensores ambientais (luminosidade, umidade do solo, temperatura) e acionamento mecânico de atuadores (módulos relé e bombas d'água/válvulas solenoides) para uma irrigação de alta precisão.
- **Conectividade (IoT):** Transferência de dados do microcontrolador para servidores web via requisições HTTP e APIs (JSON).
- **Dashboard Web:** Interface gráfica para visualização dinâmica e em tempo real do estado da horta (níveis hídricos, histórico de temperatura e status das bombas).
- **Inteligência Artificial:** Integração avançada do sistema com Modelos Base (LLMs) via um protótipo da aplicação desenvolvida por um dos integrantes (Negotium AI). A IA processa os dados coletados e atua como um "agente botânico", não apenas monitorando, mas enviando comandos lógicos de volta ao sistema para acionar a irrigação baseada em análises complexas.

## Tecnologias e Componentes

- **Microcontroladores:** ESP32
- **Linguagens e Protocolos:** C/C++, JSON, REST APIs.
- **Sensores:** Sensor de Umidade do Solo, DHT11/DHT22 (Temperatura e Umidade do ar), LDR (Luz) e Sensor de Chuva.
- **Atuadores:** Relés de potência e válvulas solenoides.
