CC ?= gcc
CFLAGS ?= -std=c11 -Wall -Wextra -pedantic -O2 -Iinclude -Ibuild
BISON ?= bison
FLEX ?= flex
LIBS ?=

BUILD_DIR := build
BIN_DIR := bin
TARGET := $(BIN_DIR)/moneyc

BISON_C := $(BUILD_DIR)/parser.c
BISON_H := $(BUILD_DIR)/parser.h
FLEX_C := $(BUILD_DIR)/lexer.c

SRC := src/ast.c src/codegen.c src/main.c
OBJ := $(BUILD_DIR)/ast.o $(BUILD_DIR)/codegen.o $(BUILD_DIR)/main.o $(BUILD_DIR)/parser.o $(BUILD_DIR)/lexer.o

.PHONY: all clean distclean run

all: $(TARGET)

$(TARGET): $(OBJ) | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $(OBJ) $(LIBS)

$(BUILD_DIR)/parser.o: $(BISON_C) $(BISON_H)
	$(CC) $(CFLAGS) -c $(BISON_C) -o $@

$(BUILD_DIR)/lexer.o: $(FLEX_C) $(BISON_H)
	$(CC) $(CFLAGS) -c $(FLEX_C) -o $@

$(BUILD_DIR)/ast.o: src/ast.c include/ast.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c src/ast.c -o $@

$(BUILD_DIR)/codegen.o: src/codegen.c include/codegen.h include/ast.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c src/codegen.c -o $@

$(BUILD_DIR)/main.o: src/main.c include/ast.h include/codegen.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c src/main.c -o $@

$(BISON_C) $(BISON_H): src/parser.y | $(BUILD_DIR)
	$(BISON) -d -o $(BISON_C) $<

$(FLEX_C): src/lexer.l $(BISON_H) | $(BUILD_DIR)
	$(FLEX) -o $@ $<

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

clean:
	rm -rf $(BUILD_DIR)

distclean: clean
	rm -rf $(BIN_DIR)

run: $(TARGET)
	$(TARGET)
