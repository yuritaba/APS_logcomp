#!/usr/bin/env python3
"""
BankVM - Máquina Virtual Baseada em Pilha para MoneyLang
=========================================================

Interpretador de assembly BankVM conforme especificado em docs/VM_SPEC.md.
Suporta operações bancárias, controle de fluxo, sensores e I/O.
"""

import sys
import time
import re
from typing import Dict, List, Any, Optional


class BankVMError(Exception):
    """Exceção base para erros da BankVM"""
    pass


class BankVM:
    """Máquina Virtual Baseada em Pilha para programas MoneyLang"""
    
    def __init__(self, debug: bool = False):
        self.stack: List[float] = []
        self.accounts: Dict[str, float] = {}
        self.variables: Dict[str, float] = {}
        self.labels: Dict[str, int] = {}
        self.instructions: List[tuple] = []
        self.pc: int = 0  # Program Counter
        self.start_time: float = time.time()
        self.base_interest_rate: float = 0.05  # Taxa de juros base (5%)
        self.debug: bool = debug
        self.halted: bool = False
        
    def load_program(self, assembly_code: str):
        """Carrega e preprocessa o código assembly"""
        lines = assembly_code.strip().split('\n')
        
        # Primeira passagem: identificar labels
        instruction_index = 0
        for line in lines:
            line = line.strip()
            
            # Ignorar linhas vazias e comentários
            if not line or line.startswith('#'):
                continue
                
            # Processar labels
            if line.startswith('LABEL '):
                label_name = line.split()[1]
                self.labels[label_name] = instruction_index
                continue
                
            # Adicionar instrução
            self.instructions.append(self._parse_instruction(line))
            instruction_index += 1
            
        if self.debug:
            print(f"[DEBUG] Labels encontrados: {self.labels}")
            print(f"[DEBUG] Total de instruções: {len(self.instructions)}")
    
    def _parse_instruction(self, line: str) -> tuple:
        """Analisa uma linha de instrução e retorna (opcode, operandos)"""
        # Tratar strings entre aspas
        string_match = re.match(r'(\w+)\s+"([^"]*)"', line)
        if string_match:
            opcode = string_match.group(1)
            operand = string_match.group(2)
            return (opcode, [operand])
        
        parts = line.split()
        if not parts:
            return ('NOP', [])
            
        opcode = parts[0]
        operands = parts[1:] if len(parts) > 1 else []
        
        # Converter operandos numéricos
        converted_operands = []
        for op in operands:
            try:
                converted_operands.append(float(op))
            except ValueError:
                converted_operands.append(op)
                
        return (opcode, converted_operands)
    
    def run(self):
        """Executa o programa carregado"""
        self.start_time = time.time()
        self.pc = 0
        self.halted = False
        
        while self.pc < len(self.instructions) and not self.halted:
            opcode, operands = self.instructions[self.pc]
            
            if self.debug:
                print(f"[DEBUG] PC={self.pc} {opcode} {operands} | Stack: {self.stack}")
            
            self._execute_instruction(opcode, operands)
            self.pc += 1
            
        if self.debug:
            print(f"[DEBUG] Execução finalizada. Stack final: {self.stack}")
            print(f"[DEBUG] Contas: {self.accounts}")
            print(f"[DEBUG] Variáveis: {self.variables}")
    
    def _execute_instruction(self, opcode: str, operands: List[Any]):
        """Executa uma instrução específica"""
        
        # Operações de Pilha
        if opcode == 'PUSH_CONST':
            self.stack.append(float(operands[0]))
            
        elif opcode == 'PUSH_STR':
            self.stack.append(operands[0])
            
        elif opcode == 'LOAD':
            name = operands[0]
            if name in self.accounts:
                self.stack.append(self.accounts[name])
            elif name in self.variables:
                self.stack.append(self.variables[name])
            else:
                raise BankVMError(f"Variável/conta '{name}' não definida")
                
        elif opcode == 'STORE':
            if not self.stack:
                raise BankVMError("Stack vazia ao tentar STORE")
            name = operands[0]
            value = self.stack.pop()
            if name in self.accounts:
                self.accounts[name] = value
            else:
                self.variables[name] = value
                
        # Aritmética
        elif opcode == 'ADD':
            b, a = self._pop2()
            self.stack.append(a + b)
            
        elif opcode == 'SUB':
            b, a = self._pop2()
            self.stack.append(a - b)
            
        elif opcode == 'MUL':
            b, a = self._pop2()
            self.stack.append(a * b)
            
        elif opcode == 'DIV':
            b, a = self._pop2()
            if b == 0:
                raise BankVMError("Divisão por zero")
            self.stack.append(a / b)
            
        elif opcode == 'MOD':
            b, a = self._pop2()
            self.stack.append(a % b)
            
        elif opcode == 'NEG':
            if not self.stack:
                raise BankVMError("Stack vazia ao tentar NEG")
            self.stack.append(-self.stack.pop())
            
        elif opcode == 'NOT':
            if not self.stack:
                raise BankVMError("Stack vazia ao tentar NOT")
            value = self.stack.pop()
            self.stack.append(1.0 if value == 0 else 0.0)
            
        # Comparações
        elif opcode == 'CMP_EQ':
            b, a = self._pop2()
            self.stack.append(1.0 if a == b else 0.0)
            
        elif opcode == 'CMP_NE':
            b, a = self._pop2()
            self.stack.append(1.0 if a != b else 0.0)
            
        elif opcode == 'CMP_LT':
            b, a = self._pop2()
            self.stack.append(1.0 if a < b else 0.0)
            
        elif opcode == 'CMP_LE':
            b, a = self._pop2()
            self.stack.append(1.0 if a <= b else 0.0)
            
        elif opcode == 'CMP_GT':
            b, a = self._pop2()
            self.stack.append(1.0 if a > b else 0.0)
            
        elif opcode == 'CMP_GE':
            b, a = self._pop2()
            self.stack.append(1.0 if a >= b else 0.0)
            
        # Controle de Fluxo
        elif opcode == 'JMP':
            label = operands[0]
            if label not in self.labels:
                raise BankVMError(f"Label '{label}' não encontrado")
            self.pc = self.labels[label] - 1  # -1 porque será incrementado
            
        elif opcode == 'JMP_IF_TRUE':
            if not self.stack:
                raise BankVMError("Stack vazia ao tentar JMP_IF_TRUE")
            value = self.stack.pop()
            if value != 0:
                label = operands[0]
                if label not in self.labels:
                    raise BankVMError(f"Label '{label}' não encontrado")
                self.pc = self.labels[label] - 1
                
        elif opcode == 'JMP_IF_FALSE':
            if not self.stack:
                raise BankVMError("Stack vazia ao tentar JMP_IF_FALSE")
            value = self.stack.pop()
            if value == 0:
                label = operands[0]
                if label not in self.labels:
                    raise BankVMError(f"Label '{label}' não encontrado")
                self.pc = self.labels[label] - 1
                
        elif opcode == 'HALT':
            self.halted = True
            
        # Primitivos Bancários
        elif opcode == 'ACCOUNT_INIT':
            name = operands[0]
            self.accounts[name] = 0.0
            
        elif opcode == 'DEPOSIT':
            if not self.stack:
                raise BankVMError("Stack vazia ao tentar DEPOSIT")
            name = operands[0]
            amount = self.stack.pop()
            if name not in self.accounts:
                raise BankVMError(f"Conta '{name}' não existe")
            self.accounts[name] += amount
            
        elif opcode == 'WITHDRAW':
            if not self.stack:
                raise BankVMError("Stack vazia ao tentar WITHDRAW")
            name = operands[0]
            amount = self.stack.pop()
            if name not in self.accounts:
                raise BankVMError(f"Conta '{name}' não existe")
            self.accounts[name] -= amount
            
        elif opcode == 'TRANSFER':
            if not self.stack:
                raise BankVMError("Stack vazia ao tentar TRANSFER")
            src = operands[0]
            dst = operands[1]
            amount = self.stack.pop()
            if src not in self.accounts:
                raise BankVMError(f"Conta origem '{src}' não existe")
            if dst not in self.accounts:
                raise BankVMError(f"Conta destino '{dst}' não existe")
            self.accounts[src] -= amount
            self.accounts[dst] += amount
            
        elif opcode == 'APPLY_INTEREST':
            if not self.stack:
                raise BankVMError("Stack vazia ao tentar APPLY_INTEREST")
            name = operands[0]
            rate = self.stack.pop()
            if name not in self.accounts:
                raise BankVMError(f"Conta '{name}' não existe")
            interest = self.accounts[name] * rate
            self.accounts[name] += interest
            
        # Sensores
        elif opcode == 'SENSOR_TEMPO':
            elapsed = time.time() - self.start_time
            self.stack.append(elapsed)
            
        elif opcode == 'SENSOR_JUROS':
            self.stack.append(self.base_interest_rate)
            
        # I/O
        elif opcode == 'PRINT':
            if not self.stack:
                raise BankVMError("Stack vazia ao tentar PRINT")
            value = self.stack.pop()
            if isinstance(value, float):
                # Formatação inteligente: mostra inteiro se não tem parte decimal
                if value == int(value):
                    print(int(value))
                else:
                    print(f"{value:.2f}")
            else:
                print(value)
                
        elif opcode == 'PRINT_STR_LITERAL':
            print(operands[0])
            
        elif opcode == 'PRINT_TOP':
            if not self.stack:
                raise BankVMError("Stack vazia ao tentar PRINT_TOP")
            value = self.stack.pop()
            if isinstance(value, float):
                if value == int(value):
                    print(int(value))
                else:
                    print(f"{value:.2f}")
            else:
                print(value)
                
        elif opcode == 'NOP':
            pass  # Sem operação
            
        else:
            raise BankVMError(f"Instrução desconhecida: {opcode}")
    
    def _pop2(self) -> tuple:
        """Remove e retorna dois valores da pilha"""
        if len(self.stack) < 2:
            raise BankVMError("Stack insuficiente para operação binária")
        b = self.stack.pop()
        a = self.stack.pop()
        return (b, a)


def main():
    """Ponto de entrada CLI para a BankVM"""
    import argparse
    
    parser = argparse.ArgumentParser(
        description='BankVM - Interpretador de Assembly para MoneyLang'
    )
    parser.add_argument(
        'input_file',
        help='Arquivo assembly (.asm) para executar'
    )
    parser.add_argument(
        '-d', '--debug',
        action='store_true',
        help='Ativar modo debug (exibe execução passo a passo)'
    )
    
    args = parser.parse_args()
    
    try:
        with open(args.input_file, 'r', encoding='utf-8') as f:
            assembly_code = f.read()
            
        vm = BankVM(debug=args.debug)
        vm.load_program(assembly_code)
        vm.run()
        
    except FileNotFoundError:
        print(f"Erro: Arquivo '{args.input_file}' não encontrado", file=sys.stderr)
        sys.exit(1)
    except BankVMError as e:
        print(f"Erro de execução: {e}", file=sys.stderr)
        sys.exit(1)
    except Exception as e:
        print(f"Erro inesperado: {e}", file=sys.stderr)
        sys.exit(1)


if __name__ == '__main__':
    main()
