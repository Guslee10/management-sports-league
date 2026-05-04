# =====================================================
#   Makefile - Sistema de Liga Deportiva
# =====================================================

# Variables (nombres cortos para reutilizar)
CC      = gcc                    # El compilador que usamos
TARGET  = liga                   # Nombre del programa final
SRC     = main.c                 # El archivo de código fuente

# Flags (banderas especiales para la compilación)
CFLAGS  = -Wall -Wextra -g $(shell pg_config --cflags)
# -Wall = mostrar todos los warnings
# -Wextra = más warnings detallados
# -g = incluir información de debug
# $(shell pg_config --cflags) = buscar librerías de PostgreSQL

LIBS    = $(shell pg_config --libs) -lpq
# Librerías de PostgreSQL necesarias para conectar a la BD

# La receta para compilar
all:
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC) $(LIBS)
	@echo "Compilacion exitosa. Ejecuta: ./$(TARGET)"
# Esto significa: 
# - Usa gcc (CC)
# - Con las banderas (CFLAGS)
# - Crea un programa llamado "liga" (-o liga)
# - Usando el código de "main.c"
# - Con las librerías de PostgreSQL (LIBS)

# Comando para limpiar (borrar el programa compilado)
clean:
	rm -f $(TARGET)
# Esto borra el archivo "liga" para empezar de cero

.PHONY: all clean
# Esto le dice que "all" y "clean" son comandos, no archivos