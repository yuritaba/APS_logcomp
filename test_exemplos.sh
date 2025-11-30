#!/bin/bash
# Script de teste para executar todos os exemplos MoneyLang

set -e  # Parar em caso de erro

COMPILER="./bin/moneyc"
VM="python3 vm/bankvm.py"
EXEMPLOS_DIR="exemplos"
OUTPUT_DIR="build/exemplos"

# Cores para output
GREEN='\033[0;32m'
BLUE='\033[0;34m'
RED='\033[0;31m'
NC='\033[0m' # No Color

echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}  MoneyLang - Teste de Exemplos${NC}"
echo -e "${BLUE}========================================${NC}\n"

# Criar diretório de saída se não existir
mkdir -p "$OUTPUT_DIR"

# Verificar se o compilador existe
if [ ! -f "$COMPILER" ]; then
    echo -e "${RED}Erro: Compilador não encontrado!${NC}"
    echo "Execute 'make' primeiro para construir o compilador."
    exit 1
fi

# Contador de sucessos e falhas
SUCCESS=0
FAILED=0
TOTAL=0

# Iterar sobre todos os arquivos .money
for money_file in "$EXEMPLOS_DIR"/*.money; do
    if [ -f "$money_file" ]; then
        TOTAL=$((TOTAL + 1))
        filename=$(basename "$money_file" .money)
        asm_file="$OUTPUT_DIR/${filename}.asm"
        
        echo -e "${BLUE}[$TOTAL] Testando: ${filename}${NC}"
        echo "----------------------------------------"
        
        # Compilar
        if $COMPILER "$money_file" -o "$asm_file" 2>/dev/null; then
            # Executar na VM
            if $VM "$asm_file" 2>/dev/null; then
                echo -e "${GREEN}✓ Sucesso${NC}\n"
                SUCCESS=$((SUCCESS + 1))
            else
                echo -e "${RED}✗ Erro na execução da VM${NC}\n"
                FAILED=$((FAILED + 1))
            fi
        else
            echo -e "${RED}✗ Erro na compilação${NC}\n"
            FAILED=$((FAILED + 1))
        fi
    fi
done

# Resumo
echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}  Resumo dos Testes${NC}"
echo -e "${BLUE}========================================${NC}"
echo -e "Total de exemplos: $TOTAL"
echo -e "${GREEN}Sucessos: $SUCCESS${NC}"
if [ $FAILED -gt 0 ]; then
    echo -e "${RED}Falhas: $FAILED${NC}"
else
    echo -e "Falhas: $FAILED"
fi

if [ $FAILED -eq 0 ]; then
    echo -e "\n${GREEN}Todos os testes passaram! 🎉${NC}"
    exit 0
else
    echo -e "\n${RED}Alguns testes falharam.${NC}"
    exit 1
fi
