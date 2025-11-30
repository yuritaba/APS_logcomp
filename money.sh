#!/bin/bash
# MoneyLang - Script auxiliar para compilar e executar programas

set -e

COMPILER="./bin/moneyc"
VM="python3 vm/bankvm.py"

# Cores
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m'

show_help() {
    echo -e "${BLUE}MoneyLang - Compilador e Executor${NC}"
    echo ""
    echo "Uso: ./money.sh [opções] <arquivo.money>"
    echo ""
    echo "Opções:"
    echo "  -d, --debug     Executar em modo debug"
    echo "  -k, --keep-asm  Manter arquivo assembly após execução"
    echo "  -o, --output    Especificar arquivo de saída assembly"
    echo "  -h, --help      Mostrar esta ajuda"
    echo ""
    echo "Exemplos:"
    echo "  ./money.sh programa.money"
    echo "  ./money.sh -d programa.money"
    echo "  ./money.sh -o saida.asm programa.money"
}

# Verificar se o compilador existe
check_compiler() {
    if [ ! -f "$COMPILER" ]; then
        echo -e "${RED}Erro: Compilador não encontrado!${NC}"
        echo "Execute 'make' primeiro para construir o compilador."
        exit 1
    fi
}

# Variáveis
DEBUG=false
KEEP_ASM=false
OUTPUT_FILE=""
INPUT_FILE=""

# Processar argumentos
while [[ $# -gt 0 ]]; do
    case $1 in
        -d|--debug)
            DEBUG=true
            shift
            ;;
        -k|--keep-asm)
            KEEP_ASM=true
            shift
            ;;
        -o|--output)
            OUTPUT_FILE="$2"
            KEEP_ASM=true
            shift 2
            ;;
        -h|--help)
            show_help
            exit 0
            ;;
        *)
            if [ -z "$INPUT_FILE" ]; then
                INPUT_FILE="$1"
            else
                echo -e "${RED}Erro: múltiplos arquivos de entrada${NC}"
                show_help
                exit 1
            fi
            shift
            ;;
    esac
done

# Verificar arquivo de entrada
if [ -z "$INPUT_FILE" ]; then
    echo -e "${RED}Erro: nenhum arquivo de entrada especificado${NC}"
    show_help
    exit 1
fi

if [ ! -f "$INPUT_FILE" ]; then
    echo -e "${RED}Erro: arquivo '$INPUT_FILE' não encontrado${NC}"
    exit 1
fi

# Verificar compilador
check_compiler

# Determinar arquivo de saída
if [ -z "$OUTPUT_FILE" ]; then
    BASENAME=$(basename "$INPUT_FILE" .money)
    OUTPUT_FILE="build/${BASENAME}.asm"
    mkdir -p build
fi

# Compilar
echo -e "${BLUE}Compilando $INPUT_FILE...${NC}"
if ! $COMPILER "$INPUT_FILE" -o "$OUTPUT_FILE" 2>&1; then
    echo -e "${RED}Erro na compilação${NC}"
    exit 1
fi
echo -e "${GREEN}✓ Compilação bem-sucedida${NC}"

# Executar
echo -e "${BLUE}Executando...${NC}"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

if [ "$DEBUG" = true ]; then
    $VM "$OUTPUT_FILE" --debug
else
    $VM "$OUTPUT_FILE"
fi

EXIT_CODE=$?
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

# Limpar arquivo assembly se não for para manter
if [ "$KEEP_ASM" = false ]; then
    rm -f "$OUTPUT_FILE"
fi

if [ $EXIT_CODE -eq 0 ]; then
    echo -e "${GREEN}✓ Execução concluída com sucesso${NC}"
else
    echo -e "${RED}✗ Erro durante a execução${NC}"
    exit $EXIT_CODE
fi
