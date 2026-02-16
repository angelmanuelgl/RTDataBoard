# --- Configuración del Compilador ---
CXX = g++

# Carpeta donde exportaste tu libreria (distDynSysVis) #OJO AQUI
LIB_SDK = DynSysVis

# Ruta del archivo .cpp que pasas por consola
# Uso: mingw32-make run APP=apps/nombrecarpeta/archivo.cpp
APP_PATH = $(APP)

# --- Flags ---
# headers de la libreria  
CXXFLAGS = -I$(LIB_SDK)/include -Wall -std=c++17

# ojooo debe ser -D y luego la macro
ifeq ($(LOG), 1)
    CXXFLAGS += -DDSV_DEBUG
endif

# 2. Linkeo:
# -L apunta a la carpeta de binarios del SDK
# -lDynSysVis busca el archivo libDynSysVis.a (NOTA:  quitamos el 'lib' y el '.a')
LDFLAGS  = -L$(LIB_SDK)/lib -lDynSysVis -lsfml-graphics -lsfml-window -lsfml-system

# --- Regla de Ejecución ---
run:
	@if "$(APP_PATH)"=="" (echo ERROR: Indica el archivo. Ej: make run APP=main.cpp && exit 1)
	@if not exist build mkdir build
	@echo --- Compilando simulacion: $(APP_PATH) ---
	$(CXX) $(APP_PATH) $(CXXFLAGS) -o build/app.exe $(LDFLAGS)
	@echo --- Ejecutando ---
	./build/app.exe