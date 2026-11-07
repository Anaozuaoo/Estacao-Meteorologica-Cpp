# IoT Weather Station / Estação Meteorológica IoT
### Multi-Target Embedded System in Modern C++ (ESP32 & STM32)
*Projeto Final de Programação em C++ — Universidade Federal de Santa Catarina (UFSC)*  
**Autores / Authors:** Roberto O. de Andrade Junior & Gabriel Raul Marino

---

## Language / Idioma
- [English Documentation](#english)
- [Documentação em Português](#português)

---

<a name="english"></a>
# English

## 1. Overview
This repository contains a modular, cross-platform embedded IoT Weather Station developed in **Modern C++ (C++17)**. The system samples environmental metrics (temperature, relative humidity, and atmospheric pressure) using the **Bosch BME280** sensor over I2C, applies 64-bit Bosch compensation algorithms, and serves telemetry to end-users.

The codebase implements **Clean Architecture** principles to separate core domain logic from hardware-dependent SDKs. As a result, the exact same core sensor driver runs unchanged across multiple microcontrollers:
- **`targets/esp32`**: ESP-IDF firmware with FreeRTOS multi-tasking, SPIFFS flash storage, and an embedded HTTP Web Server dashboard.
- **`targets/stm32`**: STM32 firmware using STM32 HAL, UART telemetry output, and extensible RTOS support.

---

## 2. Architecture & Design Patterns

The architecture enforces strict decoupling between application, domain logic, and hardware peripherals:

```
+-------------------------------------------------------------+
|                     Application Layer                       |
|   (ESP32 DeviceController / STM32 DeviceController)         |
+-------------------------------------------------------------+
                              |
       +----------------------+----------------------+
       |                                             |
       v                                             v
+-----------------------------+        +-----------------------------+
|    Hardware-Agnostic Core   |        |   Concurrency & Buffering   |
|  - SensorBase (Interface)   |        |  - DataQueue<T> (Template)  |
|  - BME280Sensor (Driver)    |        |  - Mutex (FreeRTOS / std)   |
|  - II2CDriver (HAL Interface)        +-----------------------------+
+-----------------------------+
               ^
               | (Implements II2CDriver)
       +-------+-------+
       |               |
       v               v
+--------------+ +---------------+
| ESP32 Driver | | STM32 Driver  |
| (ESP-IDF I2C)| | (STM32 HAL I2C)|
+--------------+ +---------------+
```

### Key Object-Oriented & C++ Principles:
1. **Hardware Abstraction Layer (HAL Interface):**
   `II2CDriver` defines a pure virtual contract (`writeReg`, `readReg`, `readReg16`, `readBytes`). The core sensor logic has zero compile-time dependencies on ESP-IDF or STM32 HAL.
2. **Open-Closed Principle & Polymorphism:**
   `SensorBase` defines virtual methods (`init()`, `readData()`, `getReading()`, `printData()`). The controllers manage sensors through `std::vector<SensorBase*>`, allowing new sensors (e.g., BMP280, SHT31, DHT22) to be added without modifying the controller.
3. **Dependency Injection:**
   The concrete `II2CDriver` instance and a platform delay callback (e.g. FreeRTOS `vTaskDelay` or STM32 `HAL_Delay`) are injected into `BME280Sensor` during initialization.
4. **Template Programming & Thread Safety:**
   `DataQueue<T>` is a generic, thread-safe FIFO buffer protected by mutexes, enabling safe data transfer between the sampling task and communication tasks.
5. **Separation of Presentation & Storage:**
   On the ESP32 target, static Web assets (`index.html`) are isolated in the SPIFFS flash partition (`site_data`), saving RAM and allowing web front-end updates without recompiling C++ source code.

---

## 3. ASCII Class Diagram

```
 +-------------------------------------------------------------------------+
 |                               <<Struct>>                                |
 |                               SensorData                                |
 +-------------------------------------------------------------------------+
 | + temperature : float                                                   |
 | + humidity    : float                                                   |
 | + pressure    : float                                                   |
 +-------------------------------------------------------------------------+
                                      ^
                                      | uses
 +------------------------------------+------------------------------------+
 |                                                                         |
 |  +-----------------------------------+   +---------------------------+  |
 |  |            <<Interface>>          |   |        <<Template>>       |  |
 |  |             II2CDriver            |   |       DataQueue<T>        |  |
 |  +-----------------------------------+   +---------------------------+  |
 |  | + init() : bool                   |   | - queue   : std::queue<T> |  |
 |  | + writeReg(addr, reg, val) : bool |   | - mutex   : Mutex/Sem     |  |
 |  | + readReg(addr, reg) : uint8_t    |   | - maxSize : size_t        |  |
 |  | + readReg16(addr, reg) : uint16_t |   +---------------------------+  |
 |  | + readBytes(addr, reg, buf, len)  |   | + push(item : T) : bool   |  |
 |  +-----------------------------------+   | + pop(out item : T) : bool|  |
 |                  ^                       | + isEmpty() : bool        |  |
 |                  |                       | + size() : size_t         |  |
 |                  |                       +---------------------------+  |
 |                  | implements                                           |
 |         +--------+--------+                                             |
 |         |                 |                                             |
 |  +---------------+ +---------------+                                    |
 |  |ESP32I2CDriver | |STM32I2CDriver |                                    |
 |  +---------------+ +---------------+                                    |
 |  |- port: i2c_port |- hi2c: I2C_H* |                                    |
 |  |- sda/scl : int  |- timeout : u32|                                    |
 |  +---------------+ +---------------+                                    |
 |                                                                         |
 |  +-----------------------------------+                                  |
 |  |            <<Abstract>>           |                                  |
 |  |             SensorBase            |                                  |
 |  +-----------------------------------+                                  |
 |  | # id : std::string                |                                  |
 |  | # lastReading : SensorData        |                                  |
 |  +-----------------------------------+                                  |
 |  | + init() : bool = 0               |                                  |
 |  | + readData() : bool = 0           |                                  |
 |  | + getReading() : SensorData = 0   |                                  |
 |  | + printData() : void = 0          |                                  |
 |  | + getId() : const std::string&    |                                  |
 |  +-----------------------------------+                                  |
 |                  ^                                                      |
 |                  | inherits                                             |
 |  +---------------+-------------------+                                  |
 |  |           BME280Sensor            |                                  |
 |  +-----------------------------------+                                  |
 |  | - i2c : II2CDriver&               | <>--- depends on II2CDriver      |
 |  | - address : uint8_t               |                                  |
 |  | - delayMs : DelayFunc             |                                  |
 |  | - t_fine : int32_t                |                                  |
 |  | - calib : CalibrationData         |                                  |
 |  +-----------------------------------+                                  |
 |  | + init() : bool override          |                                  |
 |  | + readData() : bool override      |                                  |
 |  | + getReading() : SensorData ovr.  |                                  |
 |  | + printData() : void override     |                                  |
 |  | - readCalibrationData() : void    |                                  |
 |  | - compensateTemperature(raw): flt |                                  |
 |  | - compensatePressure(raw): float  |                                  |
 |  | - compensateHumidity(raw): float  |                                  |
 |  +-----------------------------------+                                  |
 +-------------------------------------------------------------------------+
                                      ^
                                      | manages (polymorphic)
          +---------------------------+---------------------------+
          |                                                       |
+-----------------------------+                         +-----------------------------+
|      DeviceController       |                         |    STM32DeviceController    |
|       (ESP32 Target)        |                         |        (STM32 Target)       |
+-----------------------------+                         +-----------------------------+
| - i2c : ESP32I2CDriver      |                         | - i2c : STM32I2CDriver      |
| - spiffs : SpiffsManager    |                         | - huart : UART_HandleTypeDef|
| - network : NetworkManager  |                         | - sensors : vector<Sensor* >|
| - queue : DataQueue<Data>   |                         | - queue : DataQueue<Data>   |
| - sensors : vector<Sensor* >|                         +-----------------------------+
| - sensorTaskHandle : TaskH_t|                         | + start() : bool            |
+-----------------------------+                         | + process() : void          |
| + start() : void            |                         | - sendUartLog(msg) : void   |
| - sensorReadTask(pvParam)   |                         +-----------------------------+
+-----------------------------+
      |              |
      | uses         | uses
      v              v
+---------------+  +-----------------------------+
| SpiffsManager |  |       NetworkManager        |
+---------------+  +-----------------------------+
| - mounted:bool|  | - spiffs : SpiffsManager&   |
+---------------+  | - server : httpd_handle_t   |
| + readCreds() |  | - currentData : SensorData  |
| + writeCreds()|  +-----------------------------+
+---------------+  | + connect() : void          |
                   | + updateData(data) : void   |
                   | - startWebServer() : void   |
                   | - index_handler(req) : err  |
                   | - data_handler(req) : err   |
                   +-----------------------------+
```

---

## 4. Hardware Wiring & Pinout

### ESP32 Target
| BME280 Pin | ESP32 DevKit V1 (Default) | ESP32-C3 SuperMini | Description |
|------------|---------------------------|--------------------|-------------|
| **VCC**    | 3.3V                      | 3.3V               | Power Supply (3.3V) |
| **GND**    | GND                       | GND                | Ground |
| **SDA**    | GPIO 21                   | GPIO 8             | I2C Data Line (Pull-up) |
| **SCL**    | GPIO 22                   | GPIO 9             | I2C Clock Line (Pull-up) |
| **CSB**    | 3.3V                      | 3.3V               | Selects I2C protocol |
| **SDO**    | GND (Address `0x76`)      | GND (`0x76`)       | Address Select (`0x76` / `0x77`) |

### STM32 Target
| BME280 Pin | STM32 Nucleo-64 (F401RE / F411RE) | STM32 Blue Pill (F103C8T6) |
|------------|-----------------------------------|----------------------------|
| **VCC**    | 3.3V                              | 3.3V                       |
| **GND**    | GND                               | GND                        |
| **SCL**    | PB8 (or PB6)                      | PB6                        |
| **SDA**    | PB9 (or PB7)                      | PB7                        |
| **UART TX**| PA2 (ST-LINK Virtual COM)         | PA9                        |

---

## 5. Wi-Fi Configuration (Security Best Practices)
All hardcoded Wi-Fi credentials have been removed from source files. To connect the ESP32 to your network:

### Option A: Header Configuration (Recommended for Development)
1. Copy the example configuration template:
   ```bash
   cp targets/esp32/main/wifi_credentials.h.example targets/esp32/main/wifi_credentials.h
   ```
2. Edit `wifi_credentials.h` with your SSID and Password:
   ```cpp
   #define CONFIG_WIFI_SSID     "MyHomeNetwork"
   #define CONFIG_WIFI_PASSWORD "MySecretPassword"
   ```
3. `wifi_credentials.h` is excluded by `.gitignore` to prevent leaking credentials.

### Option B: SPIFFS Dynamic Storage
Write a `wifi.txt` file into the SPIFFS storage formatted as:
```
MyHomeNetwork;MySecretPassword
```

---

## 6. Build Instructions

### Running Core Unit Tests (Desktop / Host)
You can compile and run the core unit tests directly with any modern C++ compiler (GCC / Clang / MSVC):
```bash
# Using g++ directly
g++ -std=c++17 -I core/include core/src/BME280Sensor.cpp tests/test_core.cpp -o test_core
./test_core

# Or using CMake
cd tests
mkdir build && cd build
cmake ..
cmake --build .
ctest --output-on-failure
```

### Compiling ESP32 Target (ESP-IDF)
Ensure [ESP-IDF](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/) (v5.x recommended) is installed and sourced in your terminal:
```bash
cd targets/esp32

# Set chip target (esp32, esp32c3, esp32s3, etc.)
idf.py set-target esp32

# Build firmware and generate SPIFFS partition image with web dashboard
idf.py build

# Flash firmware and monitor console output
idf.py -p <PORT> flash monitor
```
*(Replace `<PORT>` with your serial device, e.g. `COM3` on Windows or `/dev/ttyUSB0` on Linux).*

### Compiling STM32 Target (CMake / ARM GCC)
Ensure `arm-none-eabi-gcc` is available:
```bash
cd targets/stm32
mkdir build && cd build
cmake ..
cmake --build .
```
Or open and import `targets/stm32` directly into **STM32CubeIDE**.

---
---

<a name="português"></a>
# Português

## 1. Visão Geral
Este repositório contém o sistema embarcado modular e multiplataforma de uma **Estação Meteorológica IoT**, desenvolvido em **C++ Moderno (C++17)**. O sistema realiza a leitura de grandezas ambientais (temperatura, umidade relativa e pressão atmosférica) através do sensor **Bosch BME280** via I2C, executa os algoritmos de calibração oficiais de 64 bits da Bosch e disponibiliza os dados em tempo real.

O projeto foi reestruturado seguindo princípios de **Clean Architecture**, separando a lógica de domínio dos SDKs de hardware. Dessa forma, o mesmo driver de sensor roda sem modificações em diferentes microcontroladores:
- **`targets/esp32`**: Firmware para ESP32 utilizando ESP-IDF, multitarefa não-bloqueante com FreeRTOS, sistema de arquivos em Flash SPIFFS e Servidor Web HTTP nativo (`esp_http_server`).
- **`targets/stm32`**: Firmware para microcontroladores STM32 (ARM Cortex-M) utilizando STM32 HAL, transmissão de telemetria serial via UART e suporte a FreeRTOS.

---

## 2. Arquitetura e Decisões de Projeto

A arquitetura em camadas garante total desacoplamento entre hardware e aplicação:

### Pilares de Programação Orientada a Objetos e C++:
1. **Camada de Abstração de Hardware (HAL):**
   A interface abstrata pura `II2CDriver` define os métodos de barramento (`writeReg`, `readReg`, `readReg16`, `readBytes`). A classe `BME280Sensor` comunica-se exclusivamente através dessa interface, eliminando chamadas diretas a APIs proprietárias da Espressif ou STMicroelectronics.
2. **Herança e Polimorfismo:**
   A classe base abstrata `SensorBase` estabelece o contrato (`init`, `readData`, `getReading`, `printData`). O controlador gerencia os sensores através de um vetor de ponteiros `std::vector<SensorBase*>`, permitindo adicionar novos sensores facilmente.
3. **Injeção de Dependência:**
   A referência do driver I2C concreto e uma função lambda de delay de plataforma (`vTaskDelay` no FreeRTOS ou `HAL_Delay` na STM32) são injetadas no construtor de `BME280Sensor`.
4. **Templates e Thread-Safety:**
   A classe genérica `DataQueue<T>` implementa uma fila segura protegida por Mutex, garantindo transferência de dados livre de condições de corrida (*race conditions*) entre a tarefa de amostragem e as tarefas de rede/telemetria.
5. **Persistência e Separação no ESP32 (SPIFFS):**
   O arquivo da interface web (`index.html`) fica gravado na partição flash dedicada `storage`. O servidor lê o arquivo sob demanda em pedaços (*chunks*), poupando memória RAM crítica (Heap) e permitindo atualizar a interface visual sem recompilar o firmware C++.

---

## 3. Diagrama de Classes em ASCII

*(Consulte a seção correspondente em inglês acima para o diagrama completo com notação UML em ASCII).*

---

## 4. Conexões e Pinagem (Hardware)

### Alocação de Pinos no ESP32
| Pino BME280 | ESP32 DevKit V1 (Padrão) | ESP32-C3 SuperMini | Descrição |
|-------------|--------------------------|--------------------|-----------|
| **VCC**     | 3.3V                     | 3.3V               | Alimentação (3.3V) |
| **GND**     | GND                      | GND                | Terra (0V) |
| **SDA**     | GPIO 21                  | GPIO 8             | Linha de Dados I2C |
| **SCL**     | GPIO 22                  | GPIO 9             | Linha de Clock I2C |
| **CSB**     | 3.3V                     | 3.3V               | Seleciona modo I2C |
| **SDO**     | GND (Endereço `0x76`)    | GND (`0x76`)       | Seleção de Endereço I2C |

### Alocação de Pinos no STM32
| Pino BME280 | STM32 Nucleo-64 (F401RE / F411RE) | STM32 Blue Pill (F103C8T6) |
|-------------|-----------------------------------|----------------------------|
| **VCC**     | 3.3V                              | 3.3V                       |
| **GND**     | GND                               | GND                        |
| **SCL**     | PB8 (ou PB6)                      | PB6                        |
| **SDA**     | PB9 (ou PB7)                      | PB7                        |
| **UART TX** | PA2 (Virtual COM via ST-LINK)     | PA9                        |

---

## 5. Configuração de Wi-Fi (Boas Práticas de Segurança)
Todas as credenciais hardcoded foram completamente removidas do repositório para evitar vazamento de senhas pessoais.

### Método 1: Arquivo de Configuração (wifi_credentials.h)
1. Crie uma cópia do arquivo modelo:
   ```bash
   cp targets/esp32/main/wifi_credentials.h.example targets/esp32/main/wifi_credentials.h
   ```
2. Abra `targets/esp32/main/wifi_credentials.h` e defina suas credenciais:
   ```cpp
   #define CONFIG_WIFI_SSID     "NomeDaSuaRede"
   #define CONFIG_WIFI_PASSWORD "SuaSenhaSegura"
   ```
3. O arquivo `wifi_credentials.h` está incluído no `.gitignore` e não será versionado no Git.

### Método 2: Gravação Dinâmica em Flash SPIFFS
O firmware busca credenciais no arquivo `/spiffs/wifi.txt` no formato:
```
NomeDaSuaRede;SuaSenhaSegura
```

---

## 6. Instruções de Compilação

### 1. Testes de Unidade do Core (Simulador Desktop)
Compile e execute o teste de unidade diretamente na sua máquina host (Windows/Linux/macOS) usando g++:
```bash
g++ -std=c++17 -I core/include core/src/BME280Sensor.cpp tests/test_core.cpp -o test_core
./test_core
```

### 2. Compilação para ESP32 (ESP-IDF)
Abra o terminal com o ambiente do ESP-IDF carregado:
```bash
cd targets/esp32

# 1. Configurar alvo
idf.py set-target esp32

# 2. Compilar projeto e gerar imagem da partição SPIFFS
idf.py build

# 3. Gravar na placa e abrir monitor serial
idf.py -p COMx flash monitor
```

### 3. Compilação para STM32
```bash
cd targets/stm32
mkdir build && cd build
cmake ..
cmake --build .
```
Ou importe a pasta `targets/stm32` diretamente no **STM32CubeIDE**.

---

## 7. Estrutura do Repositório

```
.
├── .gitignore                    # Regras de exclusão (builds, credenciais, binários)
├── README.md                     # Documentação completa bilíngue (EN / PT)
├── docs/                         # Documentação acadêmica e assets
│   ├── Relatorio Final.pdf       # Relatório acadêmico completo com diagramas UML
│   ├── Plano de Testes.pdf       # Plano de testes e validação
│   ├── BME280 Web Server...pdf   # Documentação de referência
│   └── demo_video.mp4            # Vídeo demonstrativo de funcionamento
├── core/                         # Camada de Domínio / Independente de Hardware
│   ├── CMakeLists.txt            # Definição da biblioteca estática do core
│   ├── include/
│   │   ├── SensorBase.h          # Classe base abstrata e struct SensorData
│   │   ├── II2CDriver.h          # Interface pura HAL de comunicação I2C
│   │   ├── BME280Sensor.h        # Cabeçalho do driver BME280
│   │   └── DataQueue.h           # Template de fila segura (Thread-safe)
│   └── src/
│       └── BME280Sensor.cpp      # Implementação com algoritmos de 64 bits Bosch
├── targets/                      # Implementações específicas por plataforma
│   ├── esp32/                    # Target ESP32 (ESP-IDF)
│   │   ├── CMakeLists.txt        # CMake do projeto ESP-IDF
│   │   ├── partitions.csv        # Tabela de partições customizada (SPIFFS storage)
│   │   ├── sdkconfig.defaults    # Configurações padrão de Flash e Partições
│   │   ├── README.md             # Guia específico para ESP32
│   │   └── main/
│   │       ├── CMakeLists.txt    # Registro de componentes e imagem SPIFFS
│   │       ├── main.cpp          # app_main() limpo sem credenciais
│   │       ├── DeviceController.h/cpp  # Orquestrador de tarefas FreeRTOS
│   │       ├── ESP32I2CDriver.h/cpp    # Implementação de II2CDriver via ESP-IDF
│   │       ├── NetworkManager.h/cpp    # Gerenciador Wi-Fi e Web Server HTTP
│   │       ├── SpiffsManager.h/cpp     # Gerenciador de Flash SPIFFS
│   │       ├── wifi_credentials.h.example # Template seguro para credenciais
│   │       └── site_data/
│   │           └── index.html    # Dashboard Web (HTML5/CSS3/JavaScript)
│   └── stm32/                    # Target STM32 (STM32 HAL / CMSIS)
│       ├── CMakeLists.txt        # CMake para compilação cruzada ARM GCC
│       ├── README.md             # Guia de pinagem e build STM32
│       ├── include/
│       │   ├── stm32_hal_compat.h      # Compatibilidade com STM32 HAL
│       │   ├── STM32I2CDriver.h        # Driver I2C para STM32 HAL
│       │   └── STM32DeviceController.h # Orquestrador para STM32
│       └── src/
│           ├── main.cpp                # Loop principal de execução
│           ├── STM32I2CDriver.cpp      # Implementação das operações HAL I2C
│           └── STM32DeviceController.cpp # Aquisição e telemetria UART
└── tests/                        # Testes de unidade
    ├── CMakeLists.txt            # Automação CTest
    └── test_core.cpp             # Teste com Mock I2C e validação de algoritmos
```

---

## 8. Licença e Créditos
Desenvolvido por **Roberto O. de Andrade Junior** e **Gabriel Raul Marino** como Projeto Final da disciplina de Programação em C++ (Engenharia Eletrônica / UFSC).
