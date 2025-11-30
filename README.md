# MoneyLang e BankVM 💰

## 🚀 Início Rápido

**🎯 Novo aqui?** → [**QUICKSTART.md**](QUICKSTART.md) - Comece em 5 minutos!

**📚 Quer entender tudo?** → [**APRESENTACAO.md**](APRESENTACAO.md) - Documento completo

**🎨 Prefere visualizar?** → [**GUIA_VISUAL.md**](GUIA_VISUAL.md) - Diagramas e fluxos

**📋 Ver índice geral?** → [**INDICE.md**](INDICE.md) - Navegue por toda documentação

**📊 Resumo executivo?** → [**EXECUTIVO.md**](EXECUTIVO.md) - Visão geral do projeto

## Visão Geral

MoneyLang é uma DSL (Linguagem de Domínio Específico) para operações bancárias que possui um compilador completo escrito com Flex/Bison. O compilador gera Assembly para a **BankVM**, uma máquina virtual baseada em pilha customizada.

### 📚 Documentação

- **[APRESENTACAO.md](APRESENTACAO.md)** - Documentação completa (Motivação, Características, Curiosidades, Exemplos)
- **[QUICKSTART.md](QUICKSTART.md)** - Guia de início rápido em 5 minutos
- **[docs/VM_SPEC.md](docs/VM_SPEC.md)** - Especificação técnica da BankVM
- **[exemplos/README.md](exemplos/README.md)** - Documentação dos 10 exemplos
- **[AGENTS.md](AGENTS.md)** - Guia de contribuição
- **[RESUMO.md](RESUMO.md)** - Resumo do projeto completo

## Compilador MoneyLang

### Dependências
- GCC/Clang com suporte a C11
- Flex (analisador léxico)
- Bison (gerador de parsers)
- Python 3.6+ (para executar a VM)

### Construção
```bash
make
```
O binário `bin/moneyc` será gerado junto com artefatos intermediários em `build/`.

### Uso Básico

#### Método 1: Script Helper (Recomendado)
```bash
./money.sh programa.money          # Compilar e executar
./money.sh -d programa.money       # Modo debug
./money.sh -k programa.money       # Manter arquivo .asm
```

#### Método 2: Manual
```bash
# Compilar
./bin/moneyc programa.money -o saida.asm

# Executar
python3 vm/bankvm.py saida.asm

# Modo debug
python3 vm/bankvm.py saida.asm --debug
```

#### Método 3: Usando Make
```bash
make test-example EX=01_operacoes_basicas   # Testar exemplo específico
make test-all                                # Testar todos os exemplos
make debug-example EX=08_simulacao_completa # Debug de exemplo
```

### Exemplo Rápido
```money
conta origem = 1000
conta destino = 0

enquanto (origem > 0)
    transferir(origem, destino, 100)

mostrar("saldo final", origem, destino)
```

**Executar:**
```bash
./money.sh exemplo.money
```

## 🎯 Exemplos

O projeto inclui 10 exemplos completos em `exemplos/`:

1. **01_operacoes_basicas.money** - Depósito, saque, impressão
2. **02_transferencias.money** - Transferências entre contas
3. **03_condicionais.money** - Estruturas se/senão
4. **04_loops.money** - Laços enquanto
5. **05_juros.money** - Aplicação de juros compostos
6. **06_sensores.money** - Sensores tempo e juros
7. **07_expressoes.money** - Operadores aritméticos
8. **08_simulacao_completa.money** - Cenário bancário real
9. **09_comparacoes.money** - Operadores de comparação
10. **10_loop_transferencias.money** - Loops com transferências

**Executar todos os testes:**
```bash
./test_exemplos.sh
```

## Estrutura do Projeto
- `src/`: arquivos `.l`, `.y` e fontes em C (AST, codegen, main)
- `include/`: cabeçalhos compartilhados
- `vm/`: **BankVM** - Máquina virtual em Python
- `exemplos/`: 10 programas de exemplo demonstrando todas as características
- `docs/VM_SPEC.md`: especificação textual do Assembly da BankVM
- `Makefile`: recipes para gerar o compilador
- `APRESENTACAO.md`: documentação completa da linguagem
- `QUICKSTART.md`: guia de início rápido
- `AGENTS.md`: guia de contribuição
- `money.sh`: script auxiliar para compilar e executar
- `test_exemplos.sh`: script de testes automatizados

## EBNF da Linguagem

### 1) PROGRAMA, INSTRUÇÕES E DECLARAÇÕES

Program        = { Statement }

Statement     = VarDecl
              | Assignment
              | IfStmt
              | WhileStmt
              | Command ;
              
VarDecl       = "conta" Identifier "=" Expression ;

Assignment    = Identifier "=" Expression ;

### 2) BLOCO E CONTROLE DE FLUXO

IfStmt        = "se" "(" Condition ")" Newline
                IDENT { Statement Newline } DEDENT
                [ "senão" Newline
                  IDENT { Statement Newline } DEDENT ] ;

WhileStmt     = "enquanto" "(" Condition ")" Newline
                IDENT { Statement Newline } DEDENT ;
                
### 3) COMANDOS ESPECÍFICOS DA VM (BankVM)

Command       = DepositCmd
              | WithdrawCmd
              | TransferCmd
              | InterestCmd
              | PrintCmd ;

DepositCmd    = "depositar"  "(" Identifier "," Expression ")" ;

WithdrawCmd   = "sacar"      "(" Identifier "," Expression ")" ;

TransferCmd   = "transferir" "(" Identifier "," Identifier "," Expression ")" ;

InterestCmd   = "aplicar_juros" "(" Identifier "," Expression ")" ;

PrintCmd      = "mostrar" "(" PrintArg { "," PrintArg } ")" ;

PrintArg      = Expression | String ;

### 4) EXPRESSÕES (com precedência)

Expression    = Term { ("+" | "-") Term } ;

Term          = Factor { ("*" | "/" | "%") Factor } ;

Factor        = [ "!" | "-" ] Primary ;

Primary       = Number
              | Identifier
              | Sensor
              | "(" Expression ")" ;


### 5) CONDIÇÕES

Condition     = Expression ( "==" | "!=" | "<" | ">" | "<=" | ">=" ) Expression ;

### 6) SENSORES

Sensor        = "tempo" | "juros" ;

### 7) LÉXICO

Identifier    = Letter { Letter | Digit | "_" } ;

Number        = Digit { Digit } [ "." Digit { Digit } ] ;

String        = '"' { any-character-except-quote } '"' ;

Letter        = "a" | … | "z" | "A" | … | "Z" ;

Digit         = "0" | … | "9" ;

### 8) PALAVRAS-RESERVADAS

ReservedWord   = "conta" | "se" | "senão" | "enquanto"
               | "depositar" | "sacar" | "transferir" | "aplicar_juros"
               | "mostrar" | "tempo" | "juros"
               | "verdadeiro" | "falso" ;

               
## Diagrama Sintático

![Diagrama Sintático](diagrama_sintatico.jpg)