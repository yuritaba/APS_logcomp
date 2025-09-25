# MoneyLang e BankVM

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

![Diagrama Sintático](diagrama_sintatico.png)