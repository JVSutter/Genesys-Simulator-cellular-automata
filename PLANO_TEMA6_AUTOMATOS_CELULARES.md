# Plano de trabalho - Tema 6: Automatos celulares universais no GenESyS

Este arquivo organiza uma sequencia provavel de etapas para desenvolver o
trabalho de modelagem e implementacao do componente de automatos celulares no
GenESyS. Ele parte do codigo real deste repositorio e da descricao do Tema 6.

## 0. Regras gerais da atividade

Segundo a descricao geral da atividade no Moodle, o trabalho deve:

- ser desenvolvido sobre o codigo real do GenESyS;
- partir do branch-base `2026-1`;
- estar efetivamente integrado ao simulador;
- ter evidencias claras de funcionamento;
- incluir testes unitarios e, quando pertinente, testes de integracao ou
  validacoes experimentais;
- demonstrar como a funcionalidade se conecta ao restante do GenESyS;
- ser entregue com codigo-fonte, pull request sem conflitos para `2026-1` e
  relatorio tecnico.

Para este Tema 6, a versao recomendada e a **terminal/shell**, porque o trabalho
e essencialmente de kernel, componente, plugin, persistencia, validacao e testes.
A GUI Qt so deve ser priorizada se o escopo final incluir configuracao visual ou
demonstracao grafica. A web app nao e necessaria para cumprir o nucleo do tema.

## 1. Escopo real no repositorio

O ponto de entrada atual nao esta diretamente em `source/plugins/components`,
mas sim em:

```text
source/plugins/components/ModalModel/CellularAutomataComp.h
source/plugins/components/ModalModel/CellularAutomataComp.cpp
```

A hierarquia especifica fica em:

```text
source/plugins/components/ModalModel/CellularAutomata/
```

Arquivos principais:

- `CellularAutomataBase.h/.cpp`
- `CellularAutomata_Classic.h`
- `CellularAutomata_1DTimed.h`
- `Lattice.h/.cpp`
- `Cell.h/.cpp`
- `State.h`
- `StateSet.h/.cpp`
- `StateSet_Enumerable.h/.cpp`
- `Neighborhood.h`
- `Neighborhood_Center.h`
- `Neighborhood_Moore.h`
- `Neighborhood_VonNeumann.h`
- `BoundaryCondition.h`
- `Boundary_Fixed.h`
- `Boundary_Closed.h`
- `LocalRule.h`
- `LocalRule_Elementary.h`
- `LocalRule_GameOfLife.h`
- `LocalRule_Growty.h`

Tambem ha exemplos de terminal:

```text
source/applications/terminal/examples/smarts/Smart_CellularAutomata.cpp
source/applications/terminal/examples/smarts/Smart_CellularAutomata1.cpp
```

E existe suporte a compilacao dinamica em:

```text
source/plugins/data/ExternalIntegration/CppCompiler.h
source/plugins/data/ExternalIntegration/CppCompiler.cpp
```

## 2. Objetivo pratico do trabalho

O objetivo nao deve ser tentar implementar todos os tipos possiveis de automato
celular de uma vez. A meta mais defensavel e transformar a base atual em um
componente configuravel, testavel e extensivel, com uma arquitetura clara para
suportar uma familia ampla de automatos.

Entrega minima recomendada:

- consolidar o componente `CellularAutomataComp`;
- completar persistencia e carregamento;
- reforcar validacao semantica;
- explicitar politica de atualizacao;
- corrigir os problemas mais graves da base atual;
- demonstrar pelo menos um automato classico e uma regra definida pelo usuario;
- deixar a arquitetura preparada para lattices, vizinhancas, contornos, estados
  e regras mais gerais.

## 3. Leitura inicial obrigatoria

Antes de codificar, leia nesta ordem:

1. `GUIA_DO_REPOSITORIO.md`
2. `CMakePresets.json`
3. `source/plugins/components/ModalModel/CellularAutomataComp.h`
4. `source/plugins/components/ModalModel/CellularAutomataComp.cpp`
5. `source/plugins/components/ModalModel/CellularAutomata/CellularAutomataBase.h`
6. `source/plugins/components/ModalModel/CellularAutomata/CellularAutomataBase.cpp`
7. `source/plugins/components/ModalModel/CellularAutomata/Lattice.h`
8. `source/plugins/components/ModalModel/CellularAutomata/Lattice.cpp`
9. `source/plugins/components/ModalModel/CellularAutomata/Neighborhood*.h`
10. `source/plugins/components/ModalModel/CellularAutomata/Boundary*.h`
11. `source/plugins/components/ModalModel/CellularAutomata/LocalRule*.h`
12. `source/plugins/data/ExternalIntegration/CppCompiler.h`
13. `source/plugins/data/ExternalIntegration/CppCompiler.cpp`
14. `source/tests/unit/test_simulator_runtime.cpp`

## 4. Diagnostico inicial da implementacao atual

O codigo ja possui uma estrutura conceitual boa, mas ainda incompleta.

Pontos positivos:

- `CellularAutomataComp` ja tem enums para tipos de automato, lattice,
  vizinhanca, contorno, conjunto de estados e regra local.
- `Lattice` ja trabalha com vetor de dimensoes, entao existe uma base para
  lattice N-dimensional.
- `Cell` ja separa `currentState`, `nextState` e `previousState`, o que ajuda em
  atualizacao sincronica.
- Ja existem vizinhancas `Center`, `Moore` e `VonNeumann`.
- Ja existem contornos `Fixed` e `Closed`.
- Ja existem regras locais como `Elementary`, `GameOfLife` e `Growty`.
- Ja existe `CppCompiler`, que pode servir para regras locais compiladas.

Limitacoes e riscos atuais:

- `_loadInstance(...)` e `_saveInstance(...)` de `CellularAutomataComp` nao
  persistem os campos do componente.
- `_check(...)` conecta ponteiros internos, mas praticamente nao valida
  coerencia.
- `_onDispatchEvent(...)` chama somente `_cellularAutomata->step()`, sem uma
  politica temporal configuravel.
- Tipos `USERDEFINED` aparecem nos enums, mas nao possuem semantica consolidada.
- `setCellularAutomataType(...)` chama destrutor diretamente em vez de `delete`,
  o que e uma pratica perigosa.
- Varios construtores assumem ponteiros nao nulos, especialmente
  `parentCellularAutomata`.
- `Neighborhood_Moore` e `Neighborhood_VonNeumann` possuem duplicacao suspeita
  no caso 1D: adicionam `-r` duas vezes, em vez de `-r` e `+r`.
- `Neighborhood_Moore` e `Neighborhood_VonNeumann` ainda nao sao realmente
  genericas para N dimensoes.
- `CellularAutomata_1DTimed` tem um ponteiro `State* state` usado sem
  inicializacao.
- `State` possui copy constructor vazio, o que pode perder valor em copias.
- `StateSet` base quase nao expressa contrato; estados enumerados existem, mas
  estados integer/bit/double ainda nao estao modelados de forma robusta.
- `LocalRule` recebe apenas `Cell*`, entao regras complexas dependem
  indiretamente de `cell->getNeighbors()` e nao recebem contexto explicito como
  tempo, posicao global, dimensoes ou parametros.

Esses pontos devem guiar as primeiras etapas. Nao avance para regra dinamica
antes de estabilizar a base minima.

## 5. Sequencia recomendada de etapas

### Etapa 1 - Preparar branch, ambiente e linha de base

Confirme que a sua branch de trabalho deriva de `2026-1`. O repositorio atual
pode estar em uma branch de desenvolvimento, mas o pull request final deve mirar
`2026-1`.

Fluxo recomendado:

```bash
git fetch origin
git checkout 2026-1
git pull
git checkout -b tema6-automatos-celulares
```

Se voce ja estiver em uma branch propria, confira a base antes de abrir o PR:

```bash
git branch --show-current
git status
git log --oneline --decorate -5
```

Instale os pre-requisitos:

```bash
sudo apt update
sudo apt install cmake ninja-build g++ python3
```

Depois valide o build principal de testes:

```bash
cmake --preset tests-unit
cmake --build --preset tests-unit
ctest --preset tests-unit
```

Se o foco for kernel e plugins:

```bash
cmake --preset tests-kernel-unit
cmake --build --preset tests-kernel-unit-run
```

Observacao: neste ambiente, no momento da analise, `cmake` e `ninja` nao estavam
instalados. So havia `g++` e `docker`.

### Etapa 2 - Criar um documento curto de decisao arquitetural

Antes de alterar muitas classes, escreva uma nota pequena em
`documentation/` explicando a solucao escolhida para regra local:

- parser;
- C++ compilado dinamicamente;
- ou hibrido.

Recomendacao pragmatica: escolha uma arquitetura hibrida, mas implemente
primeiro uma base C++ interna simples e testavel. Depois conecte a regra
definida pelo usuario.

Uma boa divisao e:

- regras classicas: classes derivadas de `LocalRule`;
- regras simples do usuario: tabela ou expressao;
- regras avancadas: funcao C++ compilada dinamicamente via `CppCompiler`.

### Etapa 3 - Estabilizar ciclo de vida e ponteiros

Antes de adicionar features, corrija a base:

- substituir chamadas diretas de destrutor por `delete` ou, idealmente,
  `std::unique_ptr`;
- garantir que setters criem objetos em ordem segura;
- impedir `_check(...)` de desreferenciar ponteiros nulos;
- garantir que `setLocalRuleType(...)`, `setLatticeType(...)`,
  `setNeighboorhoodType(...)`, `setBoundaryType(...)` e `setStateSetType(...)`
  funcionem mesmo quando chamados em ordem diferente da esperada;
- corrigir o copy constructor de `State`;
- revisar `CellularAutomata_1DTimed` antes de usar esse tipo em testes.

Resultado esperado: o componente deve poder ser criado, configurado e checado
sem segmentation fault.

### Etapa 4 - Definir um contrato de configuracao do componente

Crie campos persistentes claros em `CellularAutomataComp`.

Campos minimos:

- `cellularAutomataType`
- `latticeType`
- `latticeDimensions`
- `neighboorhoodType`
- `neighboorhoodRadius`
- `boundaryType`
- `fixedBoundaryState`, se o contorno fixed for usado
- `stateSetType`
- `stateValues`, para estado enumerado ou inteiro simples
- `localRuleType`
- `localRuleParameters`, por exemplo numero da regra elementar
- `updatePolicy`
- `stepsPerEvent`
- campos de regra definida pelo usuario, se houver

Sugestao: mantenha tipos enum como `int` na persistencia, por coerencia com os
enums atuais.

### Etapa 5 - Implementar persistencia

Complete:

```cpp
CellularAutomataComp::_loadInstance(PersistenceRecord* fields)
CellularAutomataComp::_saveInstance(PersistenceRecord* fields, bool saveDefaultValues)
```

Use como referencia:

```text
source/plugins/data/ExternalIntegration/CppCompiler.cpp
```

Objetivo:

- salvar toda configuracao necessaria para reconstruir o automato;
- carregar campos antigos com defaults seguros;
- chamar os setters apropriados apos carregar os enums;
- reconstruir lattice, vizinhanca, contorno, state set e regra local.

Teste esperado:

- criar componente;
- configurar;
- salvar modelo `.gen`;
- carregar modelo;
- verificar se os campos voltam iguais.

### Etapa 6 - Reforcar checagem semantica

Melhore `_check(...)` para validar, no minimo:

- `_cellularAutomata != nullptr`;
- `_lattice != nullptr`;
- `_neighboorhood != nullptr`;
- `_boundary != nullptr`;
- `_stateSet != nullptr`;
- `_localRule != nullptr`;
- dimensoes do lattice nao vazias e maiores que zero;
- raio da vizinhanca maior que zero;
- regra `ELEMENTAR_CA` usada apenas com estado binario e vizinhanca 1D adequada;
- regra `GAME_OF_LIFE` usada com lattice 2D, estados binarios e vizinhanca
  Moore, ou pelo menos emitir erro claro se outra combinacao nao for suportada;
- `USERDEFINED` so e valido quando a regra do usuario estiver configurada e
  compilada/carregada ou parseavel.

O erro deve explicar o problema. Evite retornar apenas `false`.

### Etapa 7 - Separar politica temporal

Crie uma representacao explicita para politica de atualizacao.

Sugestao de enum:

```cpp
enum class UpdatePolicy : int {
    SYNCHRONOUS = 1,
    SEQUENTIAL = 2,
    RANDOM_ASYNCHRONOUS = 3,
    BLOCK = 4,
    TIMED = 5
};
```

Primeira entrega recomendada:

- implementar bem `SYNCHRONOUS`;
- implementar `SEQUENTIAL` simples, se houver tempo;
- deixar `RANDOM_ASYNCHRONOUS`, `BLOCK` e `TIMED` com arquitetura clara, mesmo
  que parcialmente suportadas.

`_onDispatchEvent(...)` deve usar essa politica:

```text
para cada evento recebido:
    executar stepsPerEvent passos
    enviar entidade para o proximo componente
```

Nao deixe a politica implicita dentro de `CellularAutomata_Classic`.

### Etapa 8 - Generalizar vizinhancas essenciais

Corrija primeiro os casos atuais:

- `VonNeumann` 1D deve retornar `-r` e `+r`;
- `Moore` 1D tambem deve retornar `-r` e `+r`;
- remover `std::cout` de `Neighborhood_Moore::getNeighbors(...)` ou trocar por
  trace controlado;
- validar comportamento para raio 1.

Depois, se houver tempo:

- implementar Von Neumann N-dimensional como deslocamentos com distancia
  Manhattan ate o raio;
- implementar Moore N-dimensional como todos os deslocamentos no hipercubo
  `[-r, r]^N`, exceto o vetor zero quando `includeCellItself == false`;
- criar testes para 1D e 2D.

Nao tente triangular/hexagonal antes de reticular 1D/2D estar correto e testado.

### Etapa 9 - Consolidar condicoes de contorno

Primeira entrega:

- `FIXED`;
- `CLOSED`, interpretada como contorno periodico/wrap-around.

Testes minimos:

- lattice 1D com 3 celulas e vizinhanca de raio 1;
- verificar vizinhos da primeira e ultima celula em `FIXED`;
- verificar vizinhos da primeira e ultima celula em `CLOSED`.

Depois, se houver tempo:

- `REFLEXIVE`;
- `ADIABATIC`;
- `USERDEFINED`.

### Etapa 10 - Consolidar conjunto de estados

Comece simples:

- manter `State` com valor numerico;
- corrigir copia e inicializacao;
- fazer `StateSet_Enumerable` representar uma lista valida de estados;
- validar se estados das celulas pertencem ao conjunto permitido.

Depois disso, decida se vale criar classes especificas:

- `StateSet_Integer`;
- `StateSet_Bit`;
- `StateSet_Double`.

Nao crie essas classes se elas ficarem sem uso real. Para o trabalho, e melhor
ter `ENUMERATED` e `INTEGERBASED` funcionando bem do que cinco tipos apenas
declarados.

### Etapa 11 - Definir o contrato de regra local

Este e o ponto central do trabalho.

Problema atual:

```cpp
virtual void applyRule(Cell* cell) = 0;
```

Esse contrato e simples, mas limitado. Ele nao deixa explicito:

- tempo atual;
- indice da celula;
- posicao;
- dimensoes;
- politica de atualizacao;
- parametros externos.

Proposta de evolucao:

```cpp
struct LocalRuleContext {
    Cell* cell;
    const std::vector<Cell*>* neighbors;
    const Lattice* lattice;
    unsigned long step;
    std::vector<int> position;
};
```

Depois, criar uma API nova sem quebrar tudo de uma vez:

```cpp
virtual State evaluate(const LocalRuleContext& context) = 0;
```

O fluxo sincronico passa a ser:

```text
para cada celula:
    nextState = localRule.evaluate(context)
para cada celula:
    updateState()
```

Se for arriscado alterar todas as regras de uma vez, mantenha `applyRule(...)`
temporariamente e crie adaptadores.

### Etapa 12 - Implementar regra definida pelo usuario

Recomendacao para uma entrega robusta: implemente primeiro uma regra definida
pelo usuario baseada em tabela, e depois deixe a compilacao dinamica como
extensao.

Opcao A - tabela de transicao:

- adequada para automatos pequenos e binarios;
- facil de testar;
- persistencia simples;
- boa para demonstrar regra definida pelo usuario.

Exemplo de ideia:

```text
current,neighbors -> next
0,111 -> 1
1,000 -> 0
```

Opcao B - parser:

- elegante para expressoes simples;
- depende de expor variaveis como `self`, `n0`, `n1`, `x`, `y`, `t`;
- pode ser limitada para regras complexas.

Opcao C - C++ dinamico via `CppCompiler`:

- mais potente;
- mais dificil de testar e mais sensivel ao ambiente;
- exige contrato de ABI claro.

Contrato sugerido para C++ dinamico:

```cpp
extern "C" long genesys_ca_rule(
    long current,
    const long* neighbors,
    unsigned int neighborCount,
    const int* position,
    unsigned int dimensionCount,
    unsigned long step
);
```

O componente compila/carrega a biblioteca, resolve o simbolo com `dlsym` e chama
a funcao para obter o proximo estado.

Para o trabalho, uma boa estrategia e:

1. entregar tabela ou regra C++ interna como `USERDEFINED`;
2. documentar e, se possivel, prototipar o caminho `CppCompiler`;
3. nao depender exclusivamente de compilacao dinamica para todos os testes.

### Etapa 13 - Criar testes unitarios especificos

Adicione testes em:

```text
source/tests/unit/test_simulator_runtime.cpp
```

Ou, se ficar grande demais, crie um novo arquivo:

```text
source/tests/unit/test_cellular_automata.cpp
```

Se criar novo arquivo, inclua-o em:

```text
source/tests/unit/CMakeLists.txt
```

Testes minimos:

- `CellularAutomataComp` cria configuracao default valida;
- persistencia salva/carrega enums e dimensoes;
- Von Neumann 1D retorna vizinhos corretos;
- Moore 2D raio 1 retorna 8 vizinhos;
- contorno fixed retorna celula fixa fora do lattice;
- contorno closed faz wrap-around;
- Game of Life executa um passo conhecido;
- regra elementar 1D executa um padrao pequeno conhecido;
- regra definida pelo usuario retorna estados esperados;
- `_check(...)` rejeita configuracao invalida com mensagem clara.

### Etapa 14 - Atualizar exemplo terminal

Atualize ou crie um exemplo em:

```text
source/applications/terminal/examples/smarts/
```

Sugestao:

```text
Smart_CellularAutomataUniversal.cpp
Smart_CellularAutomataUniversal.h
```

Esse exemplo deve demonstrar:

- criacao do componente;
- lattice 2D;
- vizinhanca Moore;
- contorno closed ou fixed;
- regra Game of Life ou regra definida pelo usuario;
- execucao de alguns passos;
- salvamento em `models/`.

Se quiser compilar esse exemplo, use:

```bash
cmake -S . -B build/ca-universal -G Ninja \
  -DGENESYS_BUILD_TERMINAL_APPLICATION=ON \
  -DGENESYS_BUILD_TESTS=OFF \
  -DGENESYS_TERMINAL_EXAMPLE=smarts/Smart_CellularAutomataUniversal.cpp

cmake --build build/ca-universal --target genesys_terminal_application
./build/ca-universal/source/applications/terminal/genesys_terminal_application
```

### Etapa 15 - Documentar o componente

Crie documentacao curta em:

```text
documentation/cellular-automata-universal-component.md
```

Conteudo minimo:

- quais tipos foram implementados;
- quais ficaram apenas arquitetados;
- como configurar lattice;
- como configurar vizinhanca;
- como configurar contorno;
- como configurar estados;
- como configurar regra local;
- como funciona a politica temporal;
- exemplos de uso;
- limitacoes conhecidas.

## 6. Arquitetura recomendada

Separe o problema em quatro camadas:

### 6.1 Estrutura espacial

Responsavel por:

- dimensoes;
- coordenadas;
- mapeamento posicao -> indice;
- tipo de lattice.

Classes relacionadas:

- `Lattice`
- `Cell`

Primeira meta: reticular 1D e 2D corretos. N-dimensional deve ficar possivel
pelo contrato de `std::vector<int>`/`std::vector<unsigned short>`.

### 6.2 Estrutura relacional

Responsavel por:

- vizinhanca;
- raio;
- inclusao ou nao da celula central;
- tratamento de borda.

Classes relacionadas:

- `Neighborhood`
- `BoundaryCondition`

Primeira meta: Von Neumann e Moore corretos para reticular 1D/2D.

### 6.3 Estrutura semantica

Responsavel por:

- estados;
- conjunto de estados;
- regra local.

Classes relacionadas:

- `State`
- `StateSet`
- `LocalRule`

Primeira meta: estados numericos/enumerados e regras classicas mais uma regra
definida pelo usuario.

### 6.4 Estrutura temporal

Responsavel por:

- passo atual;
- sincronico vs assincronico;
- quantidade de subpassos por evento;
- ordem de atualizacao.

Classes relacionadas:

- `CellularAutomataBase`
- `CellularAutomata_Classic`
- possivel nova classe/enum `UpdatePolicy`
- `CellularAutomataComp::_onDispatchEvent(...)`

Primeira meta: `SYNCHRONOUS` bem definido e testado.

## 7. Plano de entregas sugerido

### Entrega 1 - Base compila e CA atual fica seguro

Conteudo:

- corrigir ponteiros nulos e ciclo de vida;
- corrigir bugs obvios de `State`, vizinhancas 1D e `CellularAutomata_1DTimed`
  se ele for mantido;
- adicionar testes basicos.

Resultado: o componente existente deixa de ser fragil.

### Entrega 2 - Persistencia e validacao

Conteudo:

- `_loadInstance(...)`;
- `_saveInstance(...)`;
- `_check(...)` robusto;
- testes de round-trip.

Resultado: o componente vira parte real do modelo GenESyS, nao apenas objeto em
memoria.

### Entrega 3 - Vizinhancas e contornos

Conteudo:

- Von Neumann 1D/2D;
- Moore 1D/2D;
- Fixed;
- Closed;
- testes unitarios.

Resultado: demonstracao de generalidade estrutural.

### Entrega 4 - Politica temporal

Conteudo:

- enum de politica de atualizacao;
- `SYNCHRONOUS`;
- opcional `SEQUENTIAL`;
- `stepsPerEvent`;
- testes.

Resultado: comportamento dinamico deixa de ser implicito.

### Entrega 5 - Regra local definida pelo usuario

Conteudo:

- tabela de transicao, parser ou C++ dinamico;
- persistencia dos parametros da regra;
- validacao;
- exemplo de uso.

Resultado: principal ponto aberto do tema fica respondido.

### Entrega 6 - Exemplo, documentacao e acabamento

Conteudo:

- exemplo terminal;
- documentacao em `documentation/`;
- revisao de nomes e mensagens de erro;
- suite de testes passando.

Resultado: trabalho apresentavel e verificavel.

### Entrega 7 - Pull request e pacote Moodle

Conteudo:

- branch limpa, com commits relacionados ao Tema 6;
- pull request sem conflitos para `2026-1` (é essa branch mesmo? Confuso);
- codigo-fonte modificado;
- testes adicionados ou atualizados;
- relatorio tecnico;
- instrucoes de reproducao dos testes e exemplos.

Resultado: entrega compatilvel com os criterios formais da disciplina.

## 8. Orientacoes de desenvolvimento

### Mantenha o escopo sob controle

Nao implemente triangular, hexagonal, rede, reflexive, adiabatic, estados double,
parser e C++ dinamico todos ao mesmo tempo. Isso aumenta muito o risco de
entregar muitas APIs incompletas.

Priorize:

1. reticular 1D/2D;
2. fixed/closed;
3. Von Neumann/Moore;
4. estados numericos/enumerados;
5. Game of Life, elementary e uma regra do usuario;
6. sincronico e, se der tempo, sequencial/assincronico.

### Prefira contratos pequenos e testaveis

Cada nova classe deve responder uma pergunta simples:

- `Lattice`: onde esta a celula?
- `Neighborhood`: quem influencia a celula?
- `BoundaryCondition`: o que acontece fora do lattice?
- `StateSet`: quais estados sao validos?
- `LocalRule`: qual e o proximo estado?
- `UpdatePolicy`: quando a celula muda?

Se uma classe comecar a responder varias dessas perguntas, provavelmente o
desenho esta acoplando responsabilidades.

### Preserve compatibilidade com o GenESyS

O componente deve continuar sendo um `ModelComponent`. Portanto:

- respeite `_onDispatchEvent(...)`;
- mantenha envio da entidade para a proxima conexao;
- use persistencia padrao do GenESyS;
- use `PluginInformation`;
- nao introduza dependencia externa desnecessaria.

### Teste a semantica, nao apenas getters/setters

Getters e setters ajudam, mas o trabalho precisa provar comportamento:

- padroes conhecidos de Game of Life;
- regra elementar com entrada pequena;
- vizinhos esperados em bordas;
- configuracoes invalidas rejeitadas.

### Documente limitacoes explicitamente

Se N-dimensional completo nao for entregue, diga:

- o que funciona;
- o que ficou arquitetado;
- quais classes devem ser estendidas.

Isso e melhor do que declarar suporte universal sem testes.

### Organize o pull request para avaliacao

O avaliador deve conseguir entender rapidamente:

- qual problema foi resolvido;
- quais arquivos foram alterados;
- quais tipos de automato celular funcionam;
- quais limites continuam conhecidos;
- quais comandos rodam os testes;
- qual exemplo terminal demonstra a funcionalidade.

Evite misturar refatoracoes sem relacao com o Tema 6. Se precisar mexer em
infraestrutura compartilhada, explique no PR e no relatorio.

## 9. Comandos uteis durante o trabalho

Listar arquivos de automatos celulares:

```bash
find source -path '*CellularAutomata*' -type f | sort
```

Buscar referencias ao componente:

```bash
rg -n "CellularAutomata|LocalRule|Neighborhood|Boundary" source models documentation
```

Configurar testes:

```bash
cmake --preset tests-unit
```

Compilar testes:

```bash
cmake --build --preset tests-unit
```

Rodar testes:

```bash
ctest --preset tests-unit
```

Compilar exemplo terminal especifico:

```bash
cmake -S . -B build/ca-example -G Ninja \
  -DGENESYS_BUILD_TERMINAL_APPLICATION=ON \
  -DGENESYS_BUILD_TESTS=OFF \
  -DGENESYS_TERMINAL_EXAMPLE=smarts/Smart_CellularAutomata.cpp

cmake --build build/ca-example --target genesys_terminal_application
```

## 10. Criterios de conclusao

Considere o trabalho tecnicamente pronto quando for possivel demonstrar:

- um modelo com automato celular classico executando corretamente;
- duas vizinhancas funcionando;
- duas condicoes de contorno funcionando;
- persistencia da configuracao;
- validacao de configuracoes invalidas;
- pelo menos uma politica de atualizacao explicita;
- uma regra classica e uma regra definida pelo usuario;
- testes automatizados cobrindo os casos acima;
- documentacao explicando arquitetura, uso e limitacoes.

## 11. Relatorio tecnico

O relatorio deve ser curto, tecnico e rastreavel ao codigo. Estrutura sugerida:

1. **Capa**: disciplina, tema, autor, repositorio/branch e data.
2. **Introducao**: contexto de automatos celulares, objetivo do Tema 6 e por que
   isso pertence ao GenESyS.
3. **Desenvolvimento**: o que foi implementado, corrigido ou estendido; arquivos
   modificados; classes envolvidas; decisoes de projeto; integracao com o
   kernel, plugins e aplicacao terminal.
4. **Validacao**: testes unitarios, testes de integracao ou exemplos; comandos
   usados; resultados obtidos; evidencias de funcionamento.
5. **Conclusao**: sintese do que foi entregue, limites conhecidos e proximos
   passos tecnicos.
6. **Referencias**: bibliografia, documentacao, artigos, paginas do GenESyS ou
   materiais usados.

Para o Tema 6, inclua explicitamente no relatorio:

- por que a versao terminal foi usada;
- como `CellularAutomataComp` se conecta ao fluxo de componentes do GenESyS;
- como a regra local definida pelo usuario foi representada;
- quais variantes de lattice, vizinhanca, contorno, estado e politica temporal
  ficaram implementadas;
- quais variantes ficaram apenas arquitetadas.

## 12. Checklist final para Moodle

Antes de entregar, confira:

- codigo compila no preset terminal ou testes;
- testes unitarios relevantes passam;
- ha pelo menos um exemplo executavel demonstrando o componente;
- o PR mira `2026-1` e esta sem conflitos;
- o relatorio cita os arquivos e classes alterados;
- o relatorio mostra evidencias de funcionamento;
- limitacoes conhecidas foram documentadas;
- todos os arquivos necessarios estao no commit/PR.

## 13. Recomendacao final

O caminho mais forte para este trabalho e tratar "universal" como
**arquitetura extensivel**, nao como promessa de implementar todas as variantes
possiveis. Entregue um nucleo reticular 1D/2D correto, persistente, validado e
testado, com uma interface limpa para novas vizinhancas, contornos, politicas de
atualizacao e regras locais. A parte de regra definida pelo usuario deve ser o
principal diferencial tecnico da entrega.
