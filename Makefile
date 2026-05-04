# =====================================================
#   Makefile - Sistema de Liga Deportiva
# =====================================================

# Variables
CC      = gcc
TARGET  = liga
SRC     = main.c

# Flags
CFLAGS  = -Wall -Wextra -g $(shell pg_config --cflags)

LIBS    = $(shell pg_config --libs) -lpq
# Librerias de PostgreSQL DB

# Compilation
all:
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC) $(LIBS)
	@echo "Compilacion exitosa. Ejecuta: ./$(TARGET)"


clean:
	rm -f $(TARGET)
# Borra el archivo "liga" para empezar de cero

.PHONY: all clean