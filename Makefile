# --- Configuración del Compilador ---
CXX = g++

# Carpeta donde exportaste tu libreria (distDynSysVis) #OJO AQUI
LIB_SDK = DynSysVis

# Ruta del archivo .cpp que pasas por consola
# Uso: mingw32-make run APP=apps/nombrecarpeta/archivo.cpp
APP_PATH = Stochastic-and-Deterministic-Lab/$(APP)


# --- Extrae el nombre base ---
# Si APP=apps/01Epodemiology/SEIR.cpp -> APP_NAME sera SEIR
APP_NAME = $(basename $(notdir $(APP_PATH)))

# --- Flags ---
# headers de la libreria  
CXXFLAGS = -I$(LIB_SDK)/include -Wall -std=c++17

# ojooo debe ser -D y luego la macro que quieras definir, en este caso DSV_DEBUG
CXXFLAGS += -DDSV_DEBUG

# 2. Linkeo:
# -L apunta a la carpeta de binarios del SDK
# -lDynSysVis busca el archivo libDynSysVis.a (NOTA:  quitamos el 'lib' y el '.a')
# -lopengl32 es para el recorte de sissors
LDFLAGS  = -L$(LIB_SDK)/lib -lDynSysVis -lsfml-graphics -lsfml-window -lsfml-system -lopengl32

# --- Regla de Ejecución ---
run:
	@if "$(APP_PATH)"=="" (echo ERROR: Indica el archivo. Ej: make run APP=main.cpp && exit 1)
	@if not exist build mkdir build
	@echo --- Compilando simulacion: $(APP_PATH) -> build/$(APP_NAME).exe ---
	$(CXX) $(APP_PATH) $(CXXFLAGS) -o build/$(APP_NAME).exe $(LDFLAGS)
	@echo --- Ejecutando ---
	./build/$(APP_NAME).exe


# mingw32-make run APP=01Epodemiology\SEIR.cpp
# mingw32-make run APP=01Epodemiology\SEIARD.cpp

# mingw32-make run APP=02Ecology\hormigas.cpp
# mingw32-make run APP=02Ecology\hormigasFase.cpp
# mingw32-make run APP=02Ecology\Growth_MaltusLogistico.cpp
# mingw32-make run APP=02Ecology\SpatialBranchingProcesses\main.cpp
# mingw32-make run APP=02Ecology\tortugas.cpp
# mingw32-make run APP=02Ecology\LotkaVolterra.cpp

# mingw32-make run APP=03Physics\pendulum.cpp
# mingw32-make run APP=03Physics\doublePendulum.cpp
# mingw32-make run APP=03Physics\LorenzAnterior.cpp
# mingw32-make run APP=03Physics\chua.cpp
# mingw32-make run APP=03Physics\rossler.cpp
# mingw32-make run APP=03Physics\lorenz.cpp

# mingw32-make run APP=05Finanzas\mixSDE.cpp
