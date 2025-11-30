# Exemplos MoneyLang

Esta pasta contém exemplos que demonstram todas as características da linguagem MoneyLang.

## Lista de Exemplos

### 01_operacoes_basicas.money
**Características demonstradas:**
- Declaração de contas com valor inicial
- Operação `depositar`
- Operação `sacar`
- Comando `mostrar` para I/O

### 02_transferencias.money
**Características demonstradas:**
- Múltiplas contas
- Operação `transferir` entre contas
- Impressão de múltiplos valores

### 03_condicionais.money
**Características demonstradas:**
- Estrutura condicional `se` / `senão`
- Operadores de comparação (`>=`, `<`)
- Aplicação de lógica condicional em operações bancárias

### 04_loops.money
**Características demonstradas:**
- Laço `enquanto`
- Variáveis como contadores
- Operações repetitivas

### 05_juros.money
**Características demonstradas:**
- Comando `aplicar_juros`
- Cálculo de juros compostos
- Expressões aritméticas com literais

### 06_sensores.money
**Características demonstradas:**
- Sensor `juros` (taxa de juros do sistema)
- Sensor `tempo` (tempo de execução)
- Uso de sensores em operações

### 07_expressoes.money
**Características demonstradas:**
- Operadores aritméticos: `+`, `-`, `*`, `/`, `%`
- Precedência de operadores
- Parênteses para agrupamento
- Operador de negação unário `-`

### 08_simulacao_completa.money
**Características demonstradas:**
- Cenário bancário realista
- Combinação de múltiplas operações
- Estruturas condicionais em contexto
- Cálculo de totais

### 09_comparacoes.money
**Características demonstradas:**
- Todos os operadores de comparação:
  - `==` (igual)
  - `!=` (diferente)
  - `<` (menor)
  - `>` (maior)
  - `<=` (menor ou igual)
  - `>=` (maior ou igual)

### 10_loop_transferencias.money
**Características demonstradas:**
- Combinação de loops e transferências
- Iteração controlada por contador
- Transferências incrementais

## Como Executar

### Pré-requisitos
1. Compilador MoneyLang construído: `make`
2. Python 3.6+ para a VM

### Executando um Exemplo

```bash
# 1. Compilar o programa MoneyLang para assembly
./bin/moneyc exemplos/01_operacoes_basicas.money -o output.asm

# 2. Executar o assembly na BankVM
python3 vm/bankvm.py output.asm
```

### Script de Teste Automatizado

Execute todos os exemplos de uma vez:

```bash
./test_exemplos.sh
```

## Estrutura dos Exemplos

Cada exemplo é autocontido e pode ser executado independentemente. Os exemplos estão ordenados por complexidade crescente, começando com operações básicas e progredindo para simulações completas.
