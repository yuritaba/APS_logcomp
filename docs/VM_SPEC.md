# Especificação Assembly da BankVM

A BankVM é uma máquina virtual baseada em pilha personalizada para programas MoneyLang. Cada instrução opera em uma pilha global e um armazenamento mutável de contas nomeadas (declaradas via `conta`). Rótulos identificam alvos de salto.

## Modelo de Dados
- **Números**: Valores de ponto flutuante de 64 bits.
- **Booleanos**: `verdadeiro` empilha `1`, `falso` empilha `0`.
- **Contas**: Registradores nomeados que armazenam saldos numéricos.
- **Strings**: Sequências UTF-8 usadas apenas com `PRINT_STR`.

## Operações de Pilha
- `PUSH_CONST <valor>`: Empilha um literal numérico.
- `PUSH_STR "<texto>"`: Empilha um literal de string.
- `LOAD <nome>`: Empilha o valor atual de variável/conta `<nome>`.
- `STORE <nome>`: Desempilha e armazena o valor em `<nome>`.

## Aritmética e Lógica
- `ADD`, `SUB`, `MUL`, `DIV`, `MOD`: Desempilha dois operandos, empilha o resultado.
- `NEG`: Nega o valor do topo.
- `NOT`: Negação lógica (`0` torna-se `1`, caso contrário `0`).
- `CMP_EQ`, `CMP_NE`, `CMP_LT`, `CMP_LE`, `CMP_GT`, `CMP_GE`: Desempilha o lado direito depois o lado esquerdo, empilha `1` se a comparação for verdadeira, caso contrário `0`.

## Fluxo de Controle
- `LABEL <nome>`: Declara um alvo de salto.
- `JMP <nome>`: Salto incondicional.
- `JMP_IF_TRUE <nome>` / `JMP_IF_FALSE <nome>`: Desempilha o valor do topo e salta condicionalmente.
- `HALT`: Termina a execução.

## Primitivos Bancários
- `ACCOUNT_INIT <nome>`: Cria uma nova conta com saldo `0`.
- `DEPOSIT <nome>`: Desempilha o valor, adiciona a `<nome>`.
- `WITHDRAW <nome>`: Desempilha o valor, subtrai de `<nome>`.
- `TRANSFER <origem> <destino>`: Desempilha o valor, subtrai de `<origem>`, adiciona a `<destino>`.
- `APPLY_INTEREST <nome>`: Desempilha a taxa (como decimal, ex.: `0.05`), adiciona `taxa * saldo(nome)` a `<nome>`.

## Sensores
- `SENSOR_TEMPO`: Empilha o tempo atual (segundos desde o início do programa).
- `SENSOR_JUROS`: Empilha a taxa de juros base atual.

## Entrada/Saída
- `PRINT`: Desempilha o valor e imprime como número.
- `PRINT_STR_LITERAL "<texto>"`: Imprime a string literal (sem uso da pilha).
- `PRINT_TOP`: Desempilha o valor e imprime.

## Estrutura do Programa
1. Inicializa contas com `ACCOUNT_INIT` seguido por sequências `LOAD`/`STORE` para definir saldos iniciais.
2. Emite blocos rotulados para estruturas de fluxo de controle.
3. Finaliza programas com `HALT` para sinalizar conclusão.

O assembler é orientado por linha; comentários começam com `#`. Rótulos aparecem em suas próprias linhas (`LABEL inicio_loop`). Instruções são em maiúscula e separadas por espaço dos operandos.
