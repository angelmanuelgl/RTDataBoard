# InsightRT - Framework de Telemetría para Simulaciones

**InsightRT** 
es una herramienta (en desarrollo) para la visualización de datos en tiempo real y análisis de sistemas dinámicos desarrollada en C++ utilizando la biblioteca SFML. 

Está diseñada para integrarse fácilmente en simulaciones complejas, permitiendo monitorear mediante gráficas temporales y retratos de fase.

## Caracteristicas
* **Visualización en Tiempo Real**: Gráficas de evolución temporal con sombreado de degradado (gradient fill).
* **Análisis de Espacio Fase**: Gráficas de trayectoria (X vs Y) para estudio de sistemas dinámicos.
* **Interfaz Adaptativa**: Paneles con bordes redondeados y títulos dinámicos que ajustan el área de dibujo automáticamente.
* **Arquitectura Extensible**: Basada en herencia para facilitar la creación de nuevos tipos de visualizaciones.

## Requisitos
* Compilador de C++ (GCC/MinGW recomendado).
* [SFML](https://www.sfml-dev.org/) (Simple and Fast Multimedia Library) instalada y configurada.

## 📂 Estructura del Proyecto

* `lib_grafica/`: Núcleo de la librería (Clases de geometría y renderizado).
* `apps/`: Experimentos y simulaciones que utilizan la librería.
* `assets/`: Recursos compartidos (Fuentes como Roboto, archivos de configuración).
* `build/`: Directorio para ejecutables y archivos objeto.



## 🚀 Instrucciones de Compilación

### Compilar la Libreria (Objetos)
Para no recompilar la librería entera cada vez que hagas un cambio en tus simulaciones, primero genera los archivos objeto:

```bash
g++ -c lib_grafica/src/Geometria.cpp -I lib_grafica/include -o build/Geometria.o
g++ -c lib_grafica/src/Graficas.cpp -I lib_grafica/include -o build/Graficas.o
```

### Compilar una Simulación Específica
Una vez generados los .o, puedes compilar cualquier aplicación de la carpeta apps/ de forma rápida:
```bash
g++ apps/hormigas/main_hormigas.cpp *.o -I lib_grafica/include -o build/hormigas.exe -lsfml-graphics -lsfml-window -lsfml-system
g++ apps/hormigas/main_hormigas.cpp build/Geometria.o build/Graficas.o -I lib_grafica/include -o build/hormigas.exe -lsfml-graphics -lsfml-window -lsfml-system
```
luego compilar el proyecto
HORMIGAS:
```bash
g++ apps/hormigas/main.cpp build/Geometria.o build/Graficas.o -I lib_grafica/include -o build/hormigas.exe -lsfml-graphics -lsfml-window -lsfml-system
```
PENDULO: 
```bash
g++ apps/pendulo/pendulo.cpp build/Geometria.o build/Graficas.o -I lib_grafica/include -o build/pendulo.exe -lsfml-graphics -lsfml-window -lsfml-system
```

## compilación (version anterior)

Para compilar el proyecto manualmente, utiliza el siguiente comando:

```bash
g++ apps/hormigas/main_hormigas.cpp lib_grafica/src/*.cpp -I lib_grafica/include -o build/hormigas.exe -lsfml-graphics -lsfml-window -lsfml-system
```