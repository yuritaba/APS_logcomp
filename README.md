# MoneyLang e BankVM

## 1) PROGRAMA, INSTRUÇÕES E DECLARAÇÕES

Program        = { Statement }

Statement     = VarDecl
              | Assignment
              | IfStmt
              | WhileStmt
              | Command ;
              
VarDecl       = "conta" Identifier "=" Expression ;

Assignment    = Identifier "=" Expression ;

## 2) BLOCO E CONTROLE DE FLUXO

IfStmt        = "se" "(" Condition ")" Newline
                TAB { Statement Newline } DEDENT
                [ "senão" Newline
                  TAB { Statement Newline } DEDENT ] ;

WhileStmt     = "enquanto" "(" Condition ")" Newline
                TAB { Statement Newline } DEDENT ;
                
## 3) COMANDOS ESPECÍFICOS DA VM (BankVM)

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

## 4) EXPRESSÕES (com precedência)

Expression    = Term { ("+" | "-") Term } ;

Term          = Factor { ("*" | "/" | "%") Factor } ;

Factor        = [ "!" | "-" ] Primary ;

Primary       = Number
              | Identifier
              | Sensor
              | "(" Expression ")" ;


## 6) CONDIÇÕES

Condition     = Expression ( "==" | "!=" | "<" | ">" | "<=" | ">=" ) Expression ;

## 7) SENSORES

Sensor        = "tempo" | "juros" ;

## 8) LÉXICO

Identifier    = Letter { Letter | Digit | "_" } ;

Number        = Digit { Digit } [ "." Digit { Digit } ] ;

String        = '"' { any-character-except-quote } '"' ;

Letter        = "a" | … | "z" | "A" | … | "Z" ;

Digit         = "0" | … | "9" ;

## 9) PALAVRAS-RESERVADAS

ReservedWord   = "conta" | "se" | "senão" | "enquanto"
               | "depositar" | "sacar" | "transferir" | "aplicar_juros"
               | "mostrar" | "tempo" | "juros"
               | "verdadeiro" | "falso" ;

               
