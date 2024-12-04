
# Projeto Smart Plug
# Departamento de Projetos Eletromidia / Hardware

## Visão Geral

O Projeto Smart Plug é uma solução inovadora baseada em IoT que utiliza o microcontrolador TYWE2S (ESP8266) como núcleo do sistema. O objetivo principal do projeto é fornecer uma maneira eficiente e segura de monitorar e controlar remotamente plugs inteligentes em ambientes remotos externos.  

Com a integração de tecnologia moderna do protocolo MQTT, o Smart Plug possibilita funcionalidades como o rearmamento remoto de disjuntores e diagnósticos avançados. Essa solução é projetada para otimizar o uso de energia, aumentar a segurança elétrica e proporcionar maior conveniência ao usuário, especialmente em locais de difícil acesso ou que exijam automação confiável.  

O projeto permite integração com outros sistemas IoT e suporte a atualizações OTA (Over-The-Air), garantindo a evolução contínua das funcionalidades e a adaptação a diferentes cenários de uso.

## Objetivo e Funcionalidades
- **Objetivo:** Automação em um dispositivo IoT que permite rearmar disjuntores remotamente, garantindo controle e segurança à distância. 
- **Funcionalidades:**
  - Controle remoto via plataformas IoT.
  - Integração em nuvem.
  - Atualizações de firmware e diagnósticos.

## Componentes Utilizados
- **Microcontrolador:** TYWE2S (baseado no ESP8266).
- **Plataforma de Integração:** Brokers que suportem o protocolo MQTT.
- **Firmware:** Firmware customizado com suporte a atualizações OTA.

## Estrutura do Repositório
```
SmartPlug_GitHub_Repo/
├── docs/         # Documentação (PDFs, diagramas, especificações técnicas)
├── src/          # Código-fonte
│   ├── include/  # Arquivos de cabeçalho
│   ├── lib/      # Bibliotecas externas
│   ├── test/     # Testes unitários
├── config/       # Arquivos de configuração (ex.: platformio.ini)
├── ci/           # Scripts de Integração Contínua (ex.: GitHub Actions)
├── examples/     # Exemplos de uso
├── tools/        # Scripts utilitários (ex.: configuração de Wi-Fi, geração de tokens)
├── LICENSE       # Licença do projeto
├── README.md     # Descrição do projeto e instruções
├── CHANGELOG.md  # Histórico de versões e alterações
└── CONTRIBUTING.md  # Diretrizes para contribuições
```

## Começando
1. Clone este repositório: `git clone https://github.com/alexabreup/SMART-PLUG`
2. Navegue até o diretório do projeto: `cd SMART-PLUG`
3. Consulte a pasta **docs/** e o Wiki para instruções de configuração e uso.

## Documentação
- Consulte o **Wiki** para guias detalhados e perguntas frequentes (FAQ).
- Recursos adicionais disponíveis na pasta **docs/**.

## Roadmap
- Integração com protocolos IoT avançados.
- Diagnósticos e análises aprimorados.

## Contribuindo
Por favor, consulte `CONTRIBUTING.md` para as diretrizes.

## Licença
Este projeto está licenciado sob a Licença MIT. Consulte o arquivo LICENSE para mais detalhes.

## Códigos
Os códigos até 04/12/2024 foram produzidos pelo técnico Thiago Ferreira e validados pelo engenheiro Henrique Rosa.

## Agradecimentos
Agradecimentos especiais à equipe pelo trabalho realizado e à Comunidade de Desenvolvedores pelo suporte.
