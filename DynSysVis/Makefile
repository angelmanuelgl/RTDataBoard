# para la carpetas
NAME = DynSysVis
# para el .a libreria estatica		
LIBNAME = lib$(NAME)
# para exportar toda la libreria haciedno una copia
DIST_DIR = out$(NAME)

# --- Configuración del Compilador ---
CXX = g++
CXXFLAGS = -Iinclude -Wall -std=c++17 

ifeq ($(LOG), 1)
    CXXFLAGS += -DDSV_DEBUG
endif

# --- Rutas de Carpetas ---
SRC_DIR = src
BUILD_DIR = build
LIB_DIR = lib

# --- --- --- Deteccion de Archivos --- --- ---

# archivos .cpp en la carpeta src
SOURCES = $(wildcard $(SRC_DIR)/*.cpp)

# lista de archivos .o que iran en build
OBJECTS = $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(SOURCES))

# lista de archivos .hpp
HEADERS = $(wildcard include/*.hpp)

# ruta  de libreria final ojo con :=
LIBRARY := $(LIB_DIR)/$(LIBNAME).a

# --- --- ---  Reglas de Compilacion --- --- --- 

all: directories $(LIBRARY)

# crear build y lib si no existen (Windows)
directories:
	@if not exist "$(BUILD_DIR)" mkdir "$(BUILD_DIR)"
	@if not exist "$(LIB_DIR)" mkdir "$(LIB_DIR)"


# empaqueta toddos los archivos .o en el archivo .a
$(LIBRARY): $(OBJECTS)
	@echo Generando libreria estatica: $@
	@if exist "$(LIBRARY)\*" rmdir /s /q "$(LIBRARY)"
	ar rcs "$@" $^


# compila cada .cpp individualmente solo si ha sido modificado
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp $(HEADERS)
	@echo Compilando $<...
	$(CXX) $(CXXFLAGS) -c $< -o $@



# --- --- ---  LIMPIAR POR SI NECESITA --- --- --- 
clean:
	@if exist "$(BUILD_DIR)" del /q $(BUILD_DIR)\*.o
	@if exist "$(LIB_DIR)" del /q $(LIB_DIR)\*.a
	@echo Limpieza terminda


# --- --- ---  LIMPIAR DISTRIBUCIÓN --- --- --- 
cleandist:
	@if exist "$(DIST_DIR)" ( \
		echo Borrando carpeta de distribucion $(DIST_DIR)... && \
		rmdir /s /q "$(DIST_DIR)" \
	)


# --- --- ---  EXPORTAR --- --- --- 

dist: cleandist all
	@echo --- Preparando distribucion en /$(DIST_DIR) ---
	@if not exist "$(DIST_DIR)" mkdir "$(DIST_DIR)"
	@if not exist "$(DIST_DIR)\include" mkdir "$(DIST_DIR)\include"
	@if not exist "$(DIST_DIR)\lib" mkdir "$(DIST_DIR)\lib"
	@echo Exportando headers...
	cmd /c copy "include\*.hpp" "$(DIST_DIR)\include\"
	@echo Exportando libreria estatica...
	cmd /c copy "lib\$(LIBNAME).a" "$(DIST_DIR)\lib\"
	@echo --- Libreria $(DIST_DIR) lista para usar ---