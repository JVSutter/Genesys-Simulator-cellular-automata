# Guia do Repositorio GenESyS

Este guia resume o que existe neste repositorio, como executar o projeto e como
trabalhar nele como desenvolvedor. Ele foi escrito a partir da estrutura atual
do codigo e dos arquivos de build presentes no projeto.

## 1. O que e este projeto

GenESyS significa **Generic and Expansible System Simulator**. O projeto e uma
plataforma de simulacao em C++ voltada para modelos extensaveis, principalmente
por meio de um kernel de simulacao e um sistema de plugins.

Na pratica, o repositorio contem:

- um kernel de simulacao;
- infraestrutura de parser para a linguagem/formato de modelos;
- plugins de componentes e definicoes de dados;
- exemplos de simulacao em terminal;
- uma aplicacao web HTTP experimental;
- uma GUI Qt;
- testes unitarios e smoke tests;
- documentacao tecnica e material historico.

O projeto usa **C++23** e **CMake** como fluxo principal de build.

## 2. Mapa rapido da estrutura

```text
.
|-- CMakeLists.txt                  # Build principal do projeto
|-- CMakePresets.json               # Presets recomendados de configuracao/build
|-- README.md                       # Visao geral do projeto
|-- GUIA_DO_REPOSITORIO.md          # Este guia
|-- docker/                         # Execucao via Docker, GUI/noVNC e web
|-- documentation/                  # PDFs, Doxygen e notas tecnicas
|-- models/                         # Modelos .gen, .json, .xml e exemplos
|-- ova/                            # Scripts e guias para ambiente OVA
|-- source/
|   |-- applications/
|   |   |-- terminal/               # Shell e exemplos executaveis de terminal
|   |   |-- web/                    # Servidor HTTP/API experimental
|   |   `-- gui/qt/GenesysQtGUI/    # Interface grafica Qt
|   |-- kernel/
|   |   |-- simulator/              # Classes centrais do simulador
|   |   |-- statistics/             # Coletores, amostradores e estatisticas
|   |   `-- util/                   # Utilitarios comuns
|   |-- parser/                     # Parser e scanner da linguagem GenESyS
|   |-- plugins/
|   |   |-- components/             # Componentes de modelagem/simulacao
|   |   `-- data/                   # Definicoes de dados usadas por componentes
|   |-- tests/                      # Testes unitarios e smoke tests
|   `-- tools/                      # Ferramentas auxiliares, distribuicoes, solvers
`-- temp/                           # Arquivos temporarios/intermediarios
```

## 3. Como o sistema se organiza

### Kernel

O nucleo fica em `source/kernel/simulator`. As classes mais importantes para
entender o projeto sao:

- `Simulator`: coordena o ambiente geral de simulacao.
- `Model`: representa um modelo de simulacao, seus componentes, dados,
  conexoes, eventos, persistencia e execucao.
- `ModelSimulation`: controla a execucao, replications, tempo simulado,
  eventos e condicoes de parada.
- `Event` e `Entity`: representam a dinamica do modelo durante a simulacao.
- `ModelComponent`: base para componentes que processam entidades/eventos.
- `ModelDataDefinition`: base para dados do modelo, como recursos, filas,
  variaveis, schedules e definicoes auxiliares.
- `PluginManager`: registra e disponibiliza plugins.
- `ParserManager` e `Parser_if`: integram parser/expressoes ao kernel.
- `TraceManager` e `OnEventManager`: oferecem rastreamento, diagnostico e
  notificacoes de eventos internos.

O CMake separa o kernel em bibliotecas estaticas, principalmente:

- `genesys_kernel_util`
- `genesys_kernel_statistics`
- `genesys_kernel_simulator_support`
- `genesys_kernel_simulator_runtime`

### Parser

O parser fica em `source/parser`. Os arquivos C++ gerados ja estao versionados,
entao um build normal nao precisa de Bison/Flex. Se for necessario regenerar o
parser, existe a opcao CMake:

```bash
cmake --preset tests-unit -DGENESYS_PARSER_REGENERATE=ON
```

Nesse caso, o ambiente precisa ter **Bison** e **Flex** instalados.

### Plugins

Plugins sao uma parte central do desenho do GenESyS. Eles ficam em:

- `source/plugins/components`: componentes de simulacao;
- `source/plugins/data`: definicoes de dados usadas pelos modelos.

O build atual compila plugins como bibliotecas estaticas. O alvo agregado usado
pelo kernel inclui, entre outros, `genesys_plugins_data` e
`genesys_plugins_components_minimal`.

Para entender rapidamente os dominios suportados, veja subpastas como:

- `DiscreteProcessing`
- `MaterialHandling`
- `ModalModel`
- `BiochemicalSimulation`
- `Continuous`
- `ExternalIntegration`
- `Grouping`
- `InputOutput`
- `Logic`

### Aplicacoes

O repositorio possui tres frentes principais de aplicacao:

- **Terminal**: `source/applications/terminal`
- **Web**: `source/applications/web`
- **GUI Qt**: `source/applications/gui/qt/GenesysQtGUI`

A aplicacao terminal pode compilar o shell interativo `GenesysShell` ou um
exemplo C++ especifico de `source/applications/terminal/examples`.

A aplicacao web cria o executavel `genesys_web_app`. O CMake tambem define o
alias `genesys_webhook`, entao alguns documentos antigos podem mencionar esse
nome.

A GUI Qt cria `genesys_qt_gui_application` e tambem o alvo agregado
`genesys_gui`.

## 4. Pre-requisitos

Para compilar localmente, instale:

- compilador C++ com suporte a C++23, como `g++` recente ou `clang++`;
- CMake 3.24 ou superior;
- Ninja, pois os presets usam o generator `Ninja`;
- Python 3, usado pelos testes;
- Qt 5 ou Qt 6, apenas para GUI e alguns testes de GUI;
- Bison/Flex, apenas se for regenerar parser/scanner;
- Docker, opcional, para rodar pelo ambiente containerizado.

Observacao do ambiente analisado: neste workspace havia `g++` e `docker`, mas
`cmake` e `ninja` nao estavam disponiveis. Por isso os comandos abaixo nao
foram executados ate o fim aqui, mas correspondem aos presets do repositorio.

## 5. Como rodar o projeto

### 5.1 Rodar testes unitarios

Este e o melhor primeiro passo para validar o ambiente.

```bash
cmake --preset tests-unit
cmake --build --preset tests-unit
ctest --preset tests-unit
```

Para o conjunto especifico de testes de kernel documentado pelo projeto:

```bash
cmake --preset tests-kernel-unit
cmake --build --preset tests-kernel-unit-run
```

O segundo comando usa o alvo `genesys_kernel_unit_tests_run`, descrito em
`source/tests/unit/KERNEL_UNIT_TESTS.md`.

### 5.2 Rodar smoke tests

```bash
cmake --preset tests-smoke
cmake --build --preset tests-smoke
ctest --preset tests-smoke
```

O smoke test principal valida a inicializacao basica do simulador.

### 5.3 Rodar o shell de terminal

```bash
cmake --preset terminal-app
cmake --build --preset terminal-app
./build/terminal-app/source/applications/terminal/genesys_terminal_application
```

Dentro do shell, use:

```text
help
exit
```

O shell tambem aceita comandos por argumento. Por exemplo:

```bash
./build/terminal-app/source/applications/terminal/genesys_terminal_application help exit
```

### 5.4 Rodar um exemplo terminal

O preset `terminal-example` compila por padrao:

```text
source/applications/terminal/examples/teaching/AnElectronicAssemblyAndTestSystem.cpp
```

Comandos:

```bash
cmake --preset terminal-example
cmake --build --preset terminal-example
./build/terminal-example/source/applications/terminal/genesys_terminal_application
```

Para trocar o exemplo, configure manualmente o caminho relativo em
`GENESYS_TERMINAL_EXAMPLE`:

```bash
cmake -S . -B build/my-example -G Ninja \
  -DGENESYS_BUILD_TERMINAL_APPLICATION=ON \
  -DGENESYS_BUILD_TESTS=OFF \
  -DGENESYS_TERMINAL_EXAMPLE=smarts/Smart_CellularAutomata.cpp

cmake --build build/my-example --target genesys_terminal_application
./build/my-example/source/applications/terminal/genesys_terminal_application
```

O caminho de `GENESYS_TERMINAL_EXAMPLE` deve ser relativo a
`source/applications/terminal/examples` e apontar para um `.cpp` que tenha o
respectivo `.h`.

### 5.5 Rodar a aplicacao web

```bash
cmake --preset web-app
cmake --build --preset web-app
./build/web-app/source/applications/web/genesys_web_app --port 8080
```

O CMake tambem aceita o alvo/alias `genesys_webhook`:

```bash
cmake --build build/web-app --target genesys_webhook
```

Endpoints publicos uteis:

```text
GET /health
GET /api/v1/worker/info
GET /api/v1/worker/capabilities
```

Exemplo rapido:

```bash
curl http://localhost:8080/health
```

Para fluxos autenticados, crie uma sessao primeiro:

```bash
curl -X POST http://localhost:8080/api/v1/auth/session
```

Depois envie o token retornado no header:

```text
Authorization: Bearer <token>
```

### 5.6 Rodar a GUI Qt

Para compilar localmente:

```bash
cmake --preset gui-app
cmake --build --preset gui-app
./build/gui-app/source/applications/gui/qt/GenesysQtGUI/genesys_qt_gui_application
```

Esse fluxo exige Qt 5 ou Qt 6 instalado. O CMake procura Qt 6 primeiro e cai
para Qt 5 se Qt 6 nao for encontrado.

### 5.7 Rodar via Docker

O projeto tem um script de conveniencia:

```bash
bash docker/exec_genesys.sh
```

Ele oferece opcoes para:

1. usar o GenESyS como usuario pela GUI em noVNC;
2. abrir ambiente de desenvolvedor com QtCreator;
3. iniciar o servidor web.

Para GUI/noVNC, abra:

```text
http://localhost:6080/vnc.html?autoconnect=1&resize=scale
```

Para web, acesse:

```text
http://localhost:8080
```

Variaveis opcionais suportadas pelo script:

```bash
export GENESYS_IMAGE="genesys-simulator:local"
export GENESYS_REPO_URL="https://github.com/rlcancian/Genesys-Simulator.git"
export GENESYS_STATE_DIR="/caminho/para/cache/genesys"
export GENESYS_NOVNC_PORT=6080
export GENESYS_WEB_PORT=8080
```

## 6. Presets CMake mais importantes

| Preset | Uso |
| --- | --- |
| `tests-unit` | Compila testes unitarios principais |
| `tests-kernel-unit` | Configura testes especificos do kernel |
| `terminal-app` | Compila o shell `GenesysShell` |
| `terminal-smart` | Compila um exemplo smart, por padrao `Smart_MarkovChain.cpp` |
| `terminal-example` | Compila exemplo de ensino, por padrao `AnElectronicAssemblyAndTestSystem.cpp` |
| `web-app` | Compila a aplicacao web |
| `genesys_web_app` | Variante equivalente para web app |
| `gui-app` | Compila GUI Qt e web core |
| `tests-smoke` | Compila smoke tests |

## 7. Orientacoes para desenvolvedores

### 7.1 Fluxo recomendado antes de alterar codigo

1. Leia o `CMakeLists.txt` do modulo que voce vai tocar.
2. Identifique se a mudanca pertence ao kernel, parser, plugin, app terminal,
   web, GUI ou tools.
3. Compile o menor alvo possivel.
4. Rode os testes relacionados.
5. So depois rode uma suite maior.

Exemplos:

```bash
cmake --preset tests-unit
cmake --build --preset tests-unit
ctest --preset tests-unit
```

Para mudancas no kernel:

```bash
cmake --preset tests-kernel-unit
cmake --build --preset tests-kernel-unit-run
```

Para mudancas na web:

```bash
cmake --preset web-app
cmake --build --preset web-app
./build/web-app/source/applications/web/genesys_web_app --port 8080 --max-requests 10
```

### 7.2 Onde colocar novas funcionalidades

- Nova regra de simulacao ou bloco de modelagem:
  `source/plugins/components`.
- Nova estrutura de dados de modelo:
  `source/plugins/data`.
- Mudanca no motor de execucao:
  `source/kernel/simulator`.
- Mudanca em estatisticas:
  `source/kernel/statistics`.
- Mudanca em expressao/linguagem:
  `source/parser`.
- Novo exemplo executavel:
  `source/applications/terminal/examples`.
- API HTTP:
  `source/applications/web/api`, `service`, `session`, `worker` ou `http`,
  conforme a responsabilidade.
- Interface grafica:
  `source/applications/gui/qt/GenesysQtGUI`.

### 7.3 Como criar ou alterar plugins

Ao trabalhar em plugins, mantenha a separacao:

- componentes executam comportamento;
- data definitions guardam dados/configuracao compartilhados;
- o kernel nao deve conhecer detalhes especificos de dominio quando isso puder
  ficar em plugin.

Depois de adicionar arquivos `.cpp` em `source/plugins/components` ou
`source/plugins/data`, o CMake tende a inclui-los automaticamente porque usa
`file(GLOB_RECURSE ... CONFIGURE_DEPENDS ...)`. Ainda assim, reconfigure o CMake
para garantir que os novos arquivos foram detectados:

```bash
cmake --preset tests-unit
```

### 7.4 Cuidados com o parser

O projeto mantem os arquivos gerados em `source/parser`, como
`GenesysParser.cpp`, `GenesysParser.h` e `Genesys++-scanner.cpp`.

Nao regenere o parser em uma mudanca comum. So use
`GENESYS_PARSER_REGENERATE=ON` quando alterar:

- `source/parser/parserBisonFlex/bisonparser.yy`
- `source/parser/parserBisonFlex/lexerparser.ll`

### 7.5 Cuidados com GUI

A GUI depende de Qt e de recursos `.qrc`. O alvo final recomendado e:

```bash
cmake --build --preset gui-app
```

O CMake ativa `AUTOMOC`, `AUTOUIC` e `AUTORCC`. Se adicionar dialogs, widgets ou
resources, confira se eles estao sob a arvore da GUI e se os `.ui` estao
encontraveis pelos `AUTOUIC_SEARCH_PATHS`.

### 7.6 Cuidados com a web API

A web app atual e uma implementacao HTTP propria, sem framework externo. O fluxo
esta dividido assim:

- `http/`: transporte HTTP simples;
- `api/`: roteamento e contratos de endpoint;
- `auth/`: tokens de sessao;
- `session/`: contexto e gerenciamento de sessoes;
- `service/`: ponte entre API e simulador;
- `worker/`: abstracao de jobs.

Tenha cuidado com:

- limite de payload;
- timeout de socket;
- validacao de token;
- restricoes de caminho em save/load;
- execucao sincrona de simulacoes.

### 7.7 Testes

Os testes ficam em:

```text
source/tests/unit
source/tests/smoke
```

O projeto usa GoogleTest. Se um pacote de sistema estiver disponivel, o CMake
usa ele; caso contrario, cai para a copia vendorizada em `source/gtest`.

Ao adicionar codigo em modulo compartilhado, adicione ou atualize testes perto
do comportamento alterado. Para mudancas de kernel, prefira testes unitarios em
`source/tests/unit`.

### 7.8 Documentacao util

Arquivos importantes para desenvolvedores:

- `README.md`: visao geral.
- `source/tests/unit/KERNEL_UNIT_TESTS.md`: workflow de testes de kernel.
- `source/applications/web/README.md`: contratos e endpoints da web app.
- `docker/README.md`: execucao via Docker.
- `documentation/kernel-cpp23-modernization-audit.md`: auditoria de modernizacao.
- `documentation/kernel-tests-build-roadmap-2026-1.md`: planejamento de testes.
- `documentation/plugin_components_method_matrix.md`: matriz de componentes.
- `documentation/genesys_wiki_consolidated.md`: material consolidado da wiki.

## 8. Problemas comuns

### `cmake: command not found`

Instale CMake 3.24 ou superior.

Em Ubuntu/Debian, uma base tipica seria:

```bash
sudo apt update
sudo apt install cmake ninja-build g++ python3
```

Para GUI:

```bash
sudo apt install qtbase5-dev qtbase5-dev-tools
```

ou instale Qt 6 equivalente, conforme a distribuicao.

### CMake nao encontra Ninja

Instale `ninja-build` ou use configuracao manual com outro generator. Como os
presets do repositorio especificam `Ninja`, o caminho mais simples e instalar
Ninja.

### CMake nao encontra Qt

Isso so bloqueia GUI e alguns testes de GUI. O kernel, terminal, parser, plugins
e web app podem ser trabalhados sem Qt se os respectivos presets nao exigirem
GUI.

### Nome do executavel web

O executavel real e `genesys_web_app`. O alvo `genesys_webhook` existe como
alias CMake. Se alguma documentacao antiga mencionar `genesys_webhook` como
executavel em disco, use `genesys_web_app` para rodar o binario.

## 9. Roteiro de leitura sugerido

Para entender o projeto sem se perder:

1. Leia `README.md`.
2. Leia este guia.
3. Abra `CMakePresets.json` para entender os fluxos suportados.
4. Leia `source/kernel/simulator/Model.h`.
5. Leia `source/kernel/simulator/ModelSimulation.h`.
6. Leia `source/kernel/simulator/PluginManager.h`.
7. Explore `source/plugins/components` e `source/plugins/data`.
8. Rode um exemplo terminal.
9. Rode os testes unitarios.
10. Depois avance para GUI, web ou parser conforme seu objetivo.
