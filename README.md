# MoneyLang e BankVM

/* =========================
   1) PROGRAMA & DECLARAÇÕES
   ========================= */

Program        = { Statement } ;

Statement      = VarDecl
               | Assignment
               | IfStmt
               | WhileStmt
               | CommandStmt
               | Block
               | ";"                    /* empty stmt permite ; solto */ ;

VarDecl        = "conta" Identifier "=" Expression ";" ;

/* =========================
   2) BLOCO E CONTROLE DE FLUXO
   ========================= */

Block          = "{" { Statement } "}" ;

IfStmt         = "se" "(" BoolExpr ")" Block [ "senão" Block ] ;

WhileStmt      = "enquanto" "(" BoolExpr ")" Block ;

/* =========================
   3) COMANDOS DA VM (BankVM)
   ========================= */

CommandStmt    = DepositCmd
               | WithdrawCmd
               | TransferCmd
               | InterestCmd
               | PrintCmd ;

DepositCmd     = "depositar"  "(" LValue "," Expression ")" ";" ;
WithdrawCmd    = "sacar"      "(" LValue "," Expression ")" ";" ;
TransferCmd    = "transferir" "(" LValue "," LValue "," Expression ")" ";" ;
InterestCmd    = "aplicar_juros" "(" LValue [ "," Expression ] ")" ";" 
                 /* opcional: 2º argumento como taxa customizada */ ;

PrintCmd       = "mostrar" "(" PrintArg { "," PrintArg } ")" ";" ;
PrintArg       = Expression | String ;

/* =========================
   4) ATRIBUIÇÃO
   ========================= */

Assignment     = LValue "=" Expression ";" ;
LValue         = Identifier ;

/* =========================
   5) EXPRESSÕES (com precedência)
   ========================= */

/* Booleanos */
BoolExpr       = OrExpr ;
OrExpr         = AndExpr { "||" AndExpr } ;
AndExpr        = EqExpr  { "&&" EqExpr  } ;

/* Comparações */
EqExpr         = RelExpr { ( "==" | "!=" ) RelExpr } ;
RelExpr        = AddExpr { ( "<" | "<=" | ">" | ">=" ) AddExpr } ;

/* Aritmética */
AddExpr        = MulExpr { ( "+" | "-" ) MulExpr } ;
MulExpr        = UnExpr  { ( "*" | "/" | "%" ) UnExpr } ;
UnExpr         = [ "!" | "-" ] Primary ;

/* Primitivos */
Primary        = Number
               | LValue
               | Sensor
               | "(" Expression ")" ;

Expression     = AddExpr ;

/* =========================
   6) SENSORES (read-only)
   ========================= */

Sensor         = "tempo" | "juros" ;
/* Regra semântica: Sensor não pode aparecer como LValue em Assignment. */

/* =========================
   7) LÉXICO
   ========================= */

Identifier     = IdentStart { IdentCont } ;
IdentStart     = Letter | "_" ;
IdentCont      = Letter | Digit | "_" ;

Number         = Digit { Digit } [ "." Digit { Digit } ] ;
String         = "\"" { StringChar } "\"" ;
StringChar     = /* qualquer caractere diferente de aspas duplas e quebra de linha */
                 ? any character except " and line break ? ;

/* =========================
   8) CARACTERES BÁSICOS
   ========================= */

Letter         = "A" | "B" | "C" | "D" | "E" | "F" | "G" | "H" | "I" | "J" | "K" | "L" | "M"
               | "N" | "O" | "P" | "Q" | "R" | "S" | "T" | "U" | "V" | "W" | "X" | "Y" | "Z"
               | "a" | "b" | "c" | "d" | "e" | "f" | "g" | "h" | "i" | "j" | "k" | "l" | "m"
               | "n" | "o" | "p" | "q" | "r" | "s" | "t" | "u" | "v" | "w" | "x" | "y" | "z" ;

Digit          = "0" | "1" | "2" | "3" | "4" | "5" | "6" | "7" | "8" | "9" ;

/* =========================
   9) PALAVRAS-RESERVADAS
   ========================= */

ReservedWord   = "conta" | "se" | "senão" | "enquanto"
               | "depositar" | "sacar" | "transferir" | "aplicar_juros"
               | "mostrar" | "tempo" | "juros"
               | "verdadeiro" | "falso" ;

/* =========================
   10) BOOLEANOS LITERAIS (opcional)
   ========================= */

BoolLiteral    = "verdadeiro" | "falso" ;

/* =========================
   11) ESPAÇOS E COMENTÁRIOS (léxico)
   ========================= */

/* O analisador léxico deve ignorar: */
Whitespace     = { " " | "\t" | "\r" | "\n" } ;
Comment        = LineComment | BlockComment ;
LineComment    = "//" { ? any char except line break ? } ( "\n" | EOF ) ;
BlockComment   = "/*" { ? any char except "*/" ? } "*/" ;
