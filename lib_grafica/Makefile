# --- Configuración del Compilador ---
CXX = g++
CXXFLAGS = -Iinclude -Wall -std=c++17
# Nota: No incluimos SFML aquí porque esta es una librería estática (.a)
# SFML se linkea al compilar la aplicación final (.exe)

# --- Rutas de Carpetas ---
SRC_DIR = src
BUILD_DIR = build
LIB_DIR = lib

# --- Deteccion de Archivos ---
# Busca todos los archivos .cpp en la carpeta src
SOURCES = $(wildcard $(SRC_DIR)/*.cpp)
# Genera la lista de archivos .o que iran en build
OBJECTS = $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(SOURCES))
# Nombre del archivo de libreria final
LIBRARY = $(LIB_DIR)/libinsightrt.a

# --- Reglas de Compilacion ---


# Regla por defecto: Ejecuta la creación de carpetas y luego la librería
all: directories $(LIBRARY)

# Crea las carpetas build y lib si no existen (Sintaxis compatible con Windows)
directories:
	@if not exist "$(BUILD_DIR)" mkdir "$(BUILD_DIR)"
	@if not exist "$(LIB_DIR)" mkdir "$(LIB_DIR)"

# Empaqueta todos los archivos .o en el archivo .a
$(LIBRARY): $(OBJECTS)
	ar rcs $@ $^

# Busca todos los headers
HEADERS = $(wildcard include/*.hpp)

# Compila cada .cpp individualmente solo si ha sido modificado
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Limpia los archivos generados para recompilar desde cero
clean:
	@if exist "$(BUILD_DIR)" del /q $(BUILD_DIR)\*.o
	@if exist "$(LIB_DIR)" del /q $(LIB_DIR)\*.a


dist: all
	@if not exist "dist" mkdir "dist"
	@if not exist "dist/include" mkdir "dist/include"
	@if not exist "dist/lib" mkdir "dist/lib"
	copy include\*.hpp dist\include\
	copy lib\libinsightrt.a dist\lib\