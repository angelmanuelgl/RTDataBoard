# Manual de Referencia Oficial de DynSysVis (dsv) v1.0

Este es el Manual de Referencia de **DynSysVis (dsv)**. Este documento la guia de usuario completa para construir visualizaciones  interactivas y en tiempo real. 

Aquí se dezgloza exactamente cómo inicializar la librería, vincular tus modelos matemáticos, configurar gráficas 2D/3D y crear paneles de control interactivos. 


---

## 1. Inicio Rápido: Configuración y Uso Básico

Para usar DynSysVis, necesitas inicializar el sistema de colores, la ventana y definir un "Layout" (la cuadrícula donde se acomodarán tus gráficas).

### Cómo configurar el entorno básico
El punto de entrada clásico de una aplicación DSV se ve así:

```cpp
#include <SFML/Graphics.hpp>
#include "DynSysVis.hpp"

int main() {
    // 1. Cargar paleta de colores global // por si gustas usas colores propios ej dsv::Color::c('tucolor')  // estan en el
    dsv::Color::cargar("assets/config/colores.txt");

    // 2. Inicializar la ventana de SFML mediante la utilidad de DSV
    sf::RenderWindow window;
    dsv::Sistema::inicializarVentana(window, "Mi Primera Simulación DSV");

    // 3. Declarar el Layout (Filas y Columnas mediante strings)
    dsv::Layout miLayout = {
        "graf1 graf1 fase",
        "graf1 graf1 fase"
    };

    // 4. Crear el tablero contenedor
    dsv::Tablero tablero(window, miLayout, dsv::Color::panelUp, dsv::Color::panelDown);
    // dsv::Tablero tablero(window, miLayout); // color por defecto

    // ... Aquí agregarás las gráficas, los modelos y el bucle principal ...
    
    while(window.isOpen()) {
        // Bucle de eventos, simulación y dibujado (ver ejemplos siguientes)
    }
    return 0;
}
```
*Nota: Los strings (`"graf1"`, `"fase"`) en el Layout determinan qué celdas se fusionan para alojar una gráfica.*

### Análisis de un Sistema de Ecuaciones Diferenciales Ordinarias`)
Para simular un sistema, DSV usa instancias de modelos. En lugar de actualizar valores a mano, la librería lo integra por ti.

```cpp
// 1. Declarar el modelo (usamos el alias Ants para acortar)
using Ants = dsv::mod::AntColony_Model;

// 2. Crear una instancia local
dsv::mod::Instance<Ants> colonia;

// 3. Establecer el estado inicial (Obreras, Guerreras, Recolectoras)
colonia.state[Ants::O] = 60.0f;
colonia.state[Ants::G] = 20.0f;
colonia.state[Ants::R] = 10.0f;

// En el bucle principal de actualización:
float dt = 0.005f;
dsv::sim::step(colonia, dt); // La librería calcula el siguiente estado

// Enviar los datos calculados a una gráfica imaginaria:
grafica->push_back(colonia.state[Ants::O], colonia.t);
```

---

## 2. Modelado de Sistemas

DynSysVis separa estrictamente las reglas del sistema (el Modelo) del estado actual del sistema (la Instancia).



### Cómo compartir un mismo modelo entre varias gráficas (Instanciación)
Si quieres simular 50 estados que obedezcan el mismo Sistema De Ecuaciones Diferenciales pero con distintos estados iniciales, debes crear un **modelo base** y **vincular** las instancias.

```cpp
// 1. Crear el modelo compartido
dsv::mod::AntColony_Model modelo_comun;
modelo_comun.bO = 0.8f; // Nacimiento de obreras
modelo_comun.dG = 0.05f; // Muerte de guerreras

// 2. Crear un arreglo de instancias
std::vector< dsv::mod::Instance<Ants> > arregloColonias(50);

// 3. Vincular todas al modelo común y darles estados aleatorios
for(int i = 0; i < 50; ++i) {
    arregloColonias[i].model_ref.vincular(modelo_comun);
    arregloColonias[i].state[Ants::O] = valor_aleatorio_x;
    arregloColonias[i].state[Ants::G] = valor_aleatorio_y;
}

// Nota: Si cambias modelo_comun.bO  EN VIVO, ¡las 50 colonias cambian su comportamiento al instante!
```


Mas detales de `Instance<Model>` y `ModelRef`

> **Contexto:** `Instance<Model>` es la clase genérica que reemplaza a las antiguas
> `SIR_Instance`, `Lorenz_Instance`, etc. Combina el estado `(state, t)` con un
> `ModelRef<Model>` que puede apuntar a un modelo **propio** (en stack, sin heap)
> o a un modelo **compartido** (puntero raw, cero overhead).




### Caso 1 — Uso básico: instancia con modelo propio

```cpp
dsv::mod::Instance<dsv::mod::SIR_Model> sir;

// Sobreescribir el estado inicial
sir.state = { 0.95f, 0.05f, 0.0f };

// Correr 100 pasos con el dispatcher automático
for (int i = 0; i < 100; ++i)
    dsv::sim::step(sir, 0.01f);

std::cout << "SIR t=" << sir.t
          << "  S=" << sir.state[0]
          << "  I=" << sir.state[1]
          << "  R=" << sir.state[2] << "\n";
```

`getModel()` devuelve una referencia directa al modelo interno, sin overhead.

---

### Caso 2 — Modificar parámetros del modelo propio

```cpp
dsv::mod::Instance<dsv::mod::SIR_Model> sir2;
sir2.getModel().beta  = 0.5f;   // más contagioso
sir2.getModel().gamma = 0.02f;

sir2.state = { 0.1f, 0.0f, 0.0f };

// También puedes llamar integradores directamente
dsv::sim::RK4_step(sir2, 0.01f);
```

---

### Caso 3 — Modelo compartido entre dos instancias

Útil para comparar trayectorias con **condiciones iniciales distintas** pero la
misma física. `vincular()` solo guarda un puntero; no hay copia del modelo.

```cpp
dsv::mod::SIR_Model modelo_comun;
modelo_comun.beta = 0.4f;

dsv::mod::Instance<dsv::mod::SIR_Model> poblacion_A;
dsv::mod::Instance<dsv::mod::SIR_Model> poblacion_B;

poblacion_A.model_ref.vincular(modelo_comun);
poblacion_B.model_ref.vincular(modelo_comun);

poblacion_A.state = { 0.99f, 0.01f, 0.0f };
poblacion_B.state = { 0.80f, 0.20f, 0.0f }; // brote más avanzado

for (int i = 0; i < 50; ++i) {
    dsv::sim::step(poblacion_A, 0.01f);
    dsv::sim::step(poblacion_B, 0.01f);
}

// Cambiar beta afecta a AMBAS instancias en el siguiente step()
modelo_comun.beta = 0.1f; // intervención: cuarentena

for (int i = 0; i < 50; ++i) {
    dsv::sim::step(poblacion_A, 0.01f);
    dsv::sim::step(poblacion_B, 0.01f);
}
```

---

### Caso 4 — Volver al modelo propio

`usar_propio()` inicializa una copia con los **valores default** del modelo,
no con los parámetros actuales del modelo compartido.

```cpp
poblacion_A.model_ref.usar_propio();
poblacion_A.getModel().beta = 0.9f; // solo afecta a poblacion_A

std::cout << poblacion_A.model_ref.es_propio() << "\n"; // true
std::cout << poblacion_B.model_ref.es_propio() << "\n"; // false
```

### Crear tu propio modelo

```cpp
struct Your_Model {
    // para SISTEMA ECUACIONES DIFERENCIALES
    static constexpr size_t dim       = 2; 
    // para ECUACIONES DIFERENCIALES ESTOCASTICAS
    static constexpr size_t noise_dim = 0; 

    // Parámetros 
    float alpha = 1.1f; 
    float beta  = 0.4f; 
    float delta = 0.1f; 
    float gamma = 0.4f; 

    // si no deseas usar indices
    enum { PRESA, DEPREDADOR };

    // sistema de ECUACIONES DIFERENCIALESD
    void drift(const std::array<float, dim>& x,
               float /*t*/,
               std::array<float, dim>& out) const
    {
        out[PRESA]      = x[PRESA]      * (alpha - beta  * x[DEPREDADOR]);
        out[DEPREDADOR] = x[DEPREDADOR] * (delta * x[PRESA] - gamma);
    }
};
```

``` cpp
struct Your_Stocastic_Model {
    // para SISTEMA ECUACIONES DIFERENCIALES
    static constexpr size_t dim       = 2; 
    // para ECUACIONES DIFERENCIALES ESTOCASTICAS
    static constexpr size_t noise_dim = 2; 

    // Parámetros 
    float alpha = 1.1f; 
    float beta  = 0.4f; 
    float delta = 0.1f; 
    float gamma = 0.4f; 

    // si no deseas usar indices
    enum { PRESA, DEPREDADOR };

    // sistema de ECUACIONES DIFERENCIALESD
    void drift(const std::array<float, dim>& x,
               float /*t*/,
               std::array<float, dim>& out) const
    {
        out[PRESA]      = x[PRESA]      * (alpha - beta  * x[DEPREDADOR]);
        out[DEPREDADOR] = x[DEPREDADOR] * (delta * x[PRESA] - gamma);
    }

    // si tienes parte estocastica  noise_dim >= 1 ej  noise_dim =2
    void diffusion(const std::array<float, dim>& x,
                   float /*t*/,
                   std::array<std::array<float, noise_dim>, dim>& out) const
    {
        out[PRESA]     [0] = sigma1 * x[PRESA];      
        out[PRESA]     [1] = 0.0f;
        out[DEPREDADOR][0] = 0.0f;                    
        out[DEPREDADOR][1] = sigma2 * x[DEPREDADOR];
    }
    
    // // En caso de que  noise_dim = 1 puedes ahcer algo mas simple
    // void diffusion(const std::array<float, dim>& /*x*/,
    //                float /*t*/,
    //                std::array<float, dim>& out) const
    // {
    //     out[PRESA] = sigma1 * x[PRESA];   
    //     out[DEPREDADOR] = sigma2 * x[DEPREDADOR];
    // }
  
};
```

---

---

## Referencia rápida de `ModelRef<Model>`

| Método | Descripción |
|---|---|
| `get()` | Devuelve `Model&` (local o compartido, sin overhead) |
| `vincular(Model& m)` | Apunta al modelo externo `m` |
| `usar_propio()` | Desconecta y crea una copia default del modelo |
| `es_propio()` | `true` si el modelo es local (no compartido) |

## Selección automática de integrador (`step()`)

| `noise_dim` | Integrador elegido |
|---|---|
| `0` | `E_step` — Euler, ODE pura |
| `1` | `EM_step_simple` — Euler-Maruyama, un ruido |
| `> 1` | `EM_step` — Euler-Maruyama, matriz de difusión `d × m` |
---

## 3. El Catálogo de Gráficas (Manual de Funciones)


###  Tipos de Gráfica disponibles

| Clase  | Dimensión | Caso de uso |
|---|---|---|
| `GraficaTiempo`  | 2D (valor vs tiempo) | Series temporales, monoserie o multiserie |
| `EspacioFase2D`  | 2D (x vs y) | Diagramas de fase, atractores 2D |
| `GraficoCircular`  | —————— | Distribución proporcional entre compartimentos |
| `Grafica3D`  | 3D interactivo | Atractores 3D, trayectorias en espacio de fase |



Toda gráfica se agrega al tablero mediante 

```cpp
tablero.add<TipoGrafica>("Titulo", colorBorde, "alias_del_layout", colorTrazo)`.
```
Este método devuelve un objeto `dsv::Vista<T>`, el cual actúa como un contenedor de abstracción dual. Su propósito es unificar en una sola interfaz tanto la lógica de visualización (el Panel) como la lógica de datos y representación (el Objeto o Grafica).

Desde el objeto vista podemos acceder directamente al objeto y al panel. Ademas el operador `-> redirige el flujo de control específicamente al objeto  `<T>`.

```cpp
struct Vista {
    T& objeto;      // el objeto generico como grafica tiempo, fase, circular
    Panel& panel;   // el contenedor

    //  Acceso directo a metodos de la grafica(porejemplo) 
    T* operator->() { return &objeto; }
```



### I GraficaTiempo (Series de tiempo 2D)
**¿Qué hace?** Dibuja el cambio de una o múltiples variables en función del tiempo. 
```cpp
// Declaración
auto obreras = tablero.add<dsv::GraficaTiempo>("Población", dsv::Color::verde, "graf1", dsv::Color::verde);

// Opciones de configuración
obreras.objeto.configurarVentanaTiempo(40); // Muestra los últimos 40 segundos
obreras.objeto.configurarLimitesY(0, 100);  // Fija el eje Y estático entre 0 y 100

// Inyectar datos en el bucle
obreras->push_back(colonia.state[Ants::O], colonia.t);
```

### II. EspacioFase2D (Atractores y Trayectorias X-Y)
**¿Qué hace?** Dibuja la relación entre dos variables sin importar el tiempo explícito.

#### a) Modo Estático (Límites Manuales)
Se establece mediante `configurarLimites(minX, maxX, minY, maxY)`. En este modo, la cámara permanece fija en las coordenadas indicadas, independientemente de si los datos salen del campo de visión.

* **Uso:** Cuando se conocen de antemano los atractores o puntos de equilibrio del sistema y se desea mantener un marco de referencia constante.
* **Comportamiento:** Si los datos exceden los límites, simplemente dejan de dibujarse en el área visible.

#### b) Auto-escalado (Escalado Global)
Se activa con `activarAutoescalado(true)`. Es un modo de "memoria total" donde la gráfica ajusta sus ejes para contener cada punto que ha sido procesado por la `Serie2D`.

* **Lógica de expansión:** Los límites son monótonos; solo pueden crecer para abarcar nuevos máximos o mínimos detectados en las series.
* **Visualización:** Permite observar el recorrido histórico completo y la evolución de la trayectoria desde las condiciones iniciales.
* **Efecto:** Si el sistema se aleja del origen, la gráfica se "aleja" (zoom out) continuamente para mantener todo en pantalla.

#### c) Seguimiento (Ventana Móvil)
Se activa con `activarSeguimiento(true)`. Este modo prioriza la observación del **estado actual** del sistema dinámico sobre su pasado.

* **Dinámica de la "Cola":** A diferencia del auto-escalado, el seguimiento solo toma en cuenta los puntos presentes en el buffer actual (limitado por `maxPoints`).
* **Comportamiento de Cámara:** La ventana se desplaza y reescala para centrarse en la posición actual de la serie. Si el sistema tiene una dirección de equilibrio (drift), la cámara "persigue" al punto principal.
* **Diferencia clave:** * **Auto-escalado desactivado (Seguimiento OFF):** Si un punto viaja de $t=0$ a $t=2000$, verás una ventana de $[0, 2000]$ mostrando toda la línea.
    * **Seguimiento activado:** Si la cola es de 1000 puntos, a tiempo $t=2000$ verás una ventana de $[1000, 2000]$, enfocada en el detalle de la curva actual.

---

### Resumen de Métodos de Control

| Método | Función | Impacto en la Vista |
| :--- | :--- | :--- |
| `configurarLimites()` | Define $[x_{min}, x_{max}, y_{min}, y_{max}]$ | Fija la vista (Modo Estático). |
| `activarAutoescalado(bool)` | Conmuta el ajuste global | La vista crece para mostrar todo el historial. |
| `activarSeguimiento(bool)` | Conmuta la ventana móvil | La vista persigue la "cabeza" de los datos. |


```cpp
auto faseOG = tablero.add<dsv::EspacioFase2D>("Obreras vs Guerreras", dsv::Color::cian, "fase", dsv::Color::cian);

// Configuración de Estilo y Visualización
faseOG.objeto.activarAutoescalado(true);        // Los ejes se adaptan a los datos
faseOG.objeto.configurarLimites(0, 500, 0, 500); // Si autoescalado es false, fuerza estos límites
faseOG.objeto.ponerDesvanecido(true);           // La cola de la trayectoria se desvanece
faseOG.objeto.ponerCabeza(false);               // Oculta el punto brillante final
faseOG.objeto.configurarMaxPoints(200);         // Longitud de la cola
faseOG.objeto.activarSeguimiento(true);         // El encuadre sigue a la punta de la trayectoria

// Inyectar datos
faseOG->push_back(colonia.state[Ants::O], colonia.state[Ants::G]);
```

### III. GraficoCircular (Pie Chart)
**¿Qué hace?** Muestra proporciones relativas en un instante de tiempo.
```cpp
auto pie = tablero.add<dsv::GraficoCircular>("Proporción", dsv::Color::aqua, "pie_alias");
pie->personalizarColores({ dsv::Color::verde, dsv::Color::oro, dsv::Color::rojo });

// Inyectar datos (espera un vector/initializer list)
pie->push_back({ colonia.state[Ants::O], colonia.state[Ants::G], colonia.state[Ants::R] });
```

### IV. Grafica3D (Trayectorias Espaciales)
**¿Qué hace?** Renderiza sistemas de 3 o más variables en un espacio tridimensional navegable.

`Grafica3D` encapsula tres subsistemas accesibles públicamente:
```
Grafica3D
├── GestorSeries  gestor     ← getGestor()   gestión de series
├── Camara3D      camara     ← getCamara()   rotación, traslación, zoom
└── Ejes3D        ejes       ← getEjes()     límites y colores de ejes XYZ
```

**Pipeline de render:**
1. `actualizarBoundsGlobal()` → le pasa el área visible a la cámara
2. `dibujarEjes()` → proyecta los extremos de los ejes al plano 2D
3. Por cada serie en `gestor.lista`: proyecta cada punto 3D con `camara.proyectar()` y llama a `dsv::draw::linea()` con paleta y grosor


Ejemplo de uso

```cpp
auto vista3D = tablero.add<dsv::Grafica3D>("Atractor 3D", dsv::Color::cian, "3d");

// Configurar los límites fijos del cubo 3D (para los ejes guía)
vista3D.objeto.getEjes().setLimites(-50, 100); 

// Inyectar datos requiere pasar por el GestorSeries, pasando {x,y,z} y un ID de string
// Si el ID no existe, se crea una serie nueva con estilo por defecto
vista3D.objeto.getGestor().push_back({x, y, z}, "Sim_ID");
```
El acceso a los datos se realiza a través de `getGestor()`. Este objeto administra las trayectorias y su apariencia.

* **Agregar una nueva serie:** No necesitas crear la serie previamente; al usar `push_back`, si el ID no existe, se crea automáticamente con una paleta por defecto.
* **Personalización (Color, Grosor, Degradado):** Para configurar una serie específica, se accede a ella mediante su identificador:



**Cámara — `Camara3D`:**

Para que la gráfica responda al ratón (rotación, traslación y zoom), es obligatorio pasar el evento de la ventana en cada iteración del bucle principal. 

La  `Grafica3D` gestiona los suyos directamente 

```cpp
sf::Event evento;
while (window.pollEvent(evento)) {
    // Permite: Click Izq (Rotar), Click Der (Trasladar), Rueda (Zoom)
    vista3D.objeto.gestionarEvento(evento);
}
```
Con pasarle el evento la grafica3D ya ofrece los siguientes controles
| Propiedad | Control |
|---|---|
| Rotación | Click izquierdo + arrastrar |
| Traslación | Click derecho + arrastrar |
| Zoom | Rueda del mouse |

La proyección es perspectiva estándar con FOV fijo (500.0f). El eje Y se invierte para corregir el sistema de coordenadas de SFML.

También puedes manipular la cámara manualmente accediendo a sus parámetros:
```cpp
auto& cam = vista3D.objeto.getCamara();
cam.setAngulos(45.f, 30.f); // Rotación inicial
cam.setZoom(1.2f);          // Nivel de acercamiento
```

**Ejes — `Ejes3D`:**
```cpp
fase3D.objeto.getEjes().setLimites(-50, 100);           // uniforme
fase3D.objeto.getEjes().setLimites(mx,Mx, my,My, mz,Mz); // individual
fase3D.objeto.getEjes().setEjeXLimites(min, max);
```

## 4.  Gestionar de multiples varias series

**Por nombre (string)** — para gráficas con series con semántica explícita:
```cpp
fase.objeto.getGestor().agregarSerie("Sim0", sf::Color(100,200,255));
fase.objeto.getGestor().push_back(x, y, "Sim0");
```
**Por índice (size_t)** — exclusivo de `Grafica3D`, para series numéricas masivas:
```cpp
fase3D.objeto.getGestor().agregarSerie(i, sf::Color(255,0,0));
fase3D.objeto.getGestor().push_back({x, y, z}, i);
```

**Push back según tipo de gráfica:**
```cpp
// GraficaTiempo — monoserie
vista->push_back(valor, t);
// GraficaTiempo — multiserie
vista->push_back(valor, t, "nombreSerie");
// EspacioFase2D — monoserie
vista->push_back(x, y);
// EspacioFase2D — multiserie
vista->push_back(x, y, "nombreSerie");
// GraficoCircular
vista->push_back({v1, v2, v3});
// Grafica3D — siempre por gestor
fase3D.objeto.getGestor().push_back({x, y, z}, id_o_nombre);
```

### Múltiples Series en Gráficas 2D
Para dibujar varias líneas en una `GraficaTiempo` o `EspacioFase2D`, primero declaras las series adicionales y luego usas un `push_back` con un tercer parámetro (el nombre/ID de la serie).


```cpp
auto multiserie = tablero.add<dsv::GraficaTiempo>("Total", dsv::Color::celeste, "all");

// Añadir series adicionales
multiserie->agregarSerie("obreras", dsv::Color::verde);
multiserie->agregarSerie("guerreras", dsv::Color::rojo);

// Inyectar datos especificando el nombre
multiserie->push_back(val_obreras, t, "obreras");
multiserie->push_back(val_guerreras, t, "guerreras");
```


### Estética Avanzada en 3D (`GestorSeries`)
Para las gráficas 3D, todo el control de estética se hace mediante el `GestorSeries`.

```cpp
GestorSeries& g = fase3D.objeto.getGestor();

// Agregar series
g.agregarSerie("nombre", sf::Color);
g.agregarSerie("nombre", std::vector<sf::Color>); // paleta
g.agregarSerie(size_t_id, sf::Color);

// Datos
g.push_back({x, y, z}, id);
g.push_back({x, y, z}, "nombre");

// Estilo individual
g.setColor(sf::Color, id_o_nombre);
g.setGrosor(float, id_o_nombre);
g.setDifuminado(bool, id_o_nombre);
g.setAdelgazado(bool, id_o_nombre);

// Estilo global (todas las series actuales y futuras)
g.setColorSeries(sf::Color);
g.setColorSeries(std::vector<sf::Color>);  // paleta degradada
g.setGrosorSeries(float);
g.setDifuminadoSeries(bool);   // cola se desvanece en alpha
g.setAdelgazadoSeries(bool);   // grosor decrece en la cola
g.setMaxPointsSeries(size_t);
```


La clase Ejes3D dibuja el cubo de referencia. Es importante definir los límites para que la rejilla coincida con la escala de tus datos.

```
// Configura el rango visual del cubo de referencia (min, max para X, Y, Z)
vista3D.objeto.getEjes().setLimites(-50, 50, -50, 50, -50, 50);

// Alternativamente, usar un rango uniforme
vista3D.objeto.getEjes().setLimites(-100, 100);
```


Ejemplo
```cpp
// 1. Crear la vista en el tablero
auto v3d = tablero.add<dsv::Grafica3D>("Atractor", sf::Color::White, "layout_id");

// 2. Configuración inicial del gestor
v3d.objeto.getGestor().setGrosorSeries(1.8f);
v3d.objeto.getGestor().setColor(dsv::Color::Arcoiris(), "Lorentz");

// 3. Bucle de actualización
while (window.isOpen()) {
    sf::Event e;
    while (window.pollEvent(e)) v3d.objeto.gestionarEvento(e);

    // ... cálculo de física {nx, ny, nz} ...
    v3d.objeto.getGestor().push_back({nx, ny, nz}, "Lorentz");

    tablero.draw();
}
```


---




### 5. Paletas de color — sistema `dsv::Color`

Aquí tienes la sección de nomenclatura y referencia de colores convertida a Markdown dentro de un único bloque de código, utilizando tablas para organizar la información de las familias cromáticas:

Markdown


Para mantener la consistencia estética, la mayoría de los colores base cuentan con cuatro variantes de intensidad. Esto permite crear interfaces con profundidad, sombras y resaltados coherentes:

* **`color`**: El tono base, optimizado para visibilidad estándar.
* **`color_l` (Light)**: Versión aclarada, ideal para efectos *hover* o áreas de luz.
* **`color_d` (Dark)**: Versión oscurecida, útil para sombras o elementos secundarios.
* **`color_dd` (Deep Dark)**: Versión muy oscura, diseñada para fondos de contraste o capas profundas.

Los colores que presentan estos 4 variantes son: rojo, vino, rosa, rosaMexicano, magenta, naranja, amarillo, oro, ocre, verde, verdeLimon, aqua, cian, celeste, azul, azulMar, violeta, morado, marron, cafe, cobre, plata, blanco, gris, negro..

Teambien tenemos algunos todos exttras: verde_bosque, azul_noche, terracora, fondo_panel, panelUp, panelDown, gris_claro, gris_oscuro


#### Referencia de Colores por Familia

**Colores Cálidos y Vivos**

| Familia | Base (RGB) | Light `_l` | Dark `_d` | Deep Dark `_dd` |
| :--- | :--- | :--- | :--- | :--- |
| **Rojo** | (255, 82, 82) | (255, 150, 150) | (180, 40, 40) | (35, 20, 20) |
| **Vino** | (140, 20, 50) | (190, 80, 100) | (80, 10, 30) | (25, 15, 18) |
| **Naranja** | (255, 150, 50) | (255, 200, 130) | (200, 100, 20) | (35, 25, 15) |
| **Amarillo** | (255, 220, 60) | (255, 240, 150) | (210, 170, 20) | (32, 30, 15) |
| **Rosa** | (255, 120, 180) | (255, 180, 210) | (200, 80, 130) | (35, 20, 28) |
| **Rosa Mexicano** | (230, 0, 125) | (255, 100, 180) | (160, 0, 90) | (30, 15, 22) |
| **Magenta** | (220, 50, 220) | (240, 150, 240) | (150, 30, 150) | (28, 18, 28) |

**Colores Fríos y Naturaleza**

| Familia | Base (RGB) | Light `_l` | Dark `_d` | Deep Dark `_dd` |
| :--- | :--- | :--- | :--- | :--- |
| **Verde** | (100, 255, 170) | (180, 255, 210) | (60, 190, 120) | (20, 40, 30) |
| **Verde Limón** | (180, 255, 50) | (220, 255, 140) | (130, 200, 20) | (25, 32, 18) |
| **Aqua** | (80, 230, 210) | (170, 245, 235) | (50, 170, 155) | (15, 35, 33) |
| **Cian** | (50, 230, 255) | (160, 245, 255) | (30, 160, 190) | (15, 30, 35) |
| **Celeste** | (130, 245, 255) | (200, 250, 255) | (80, 190, 220) | (20, 35, 40) |
| **Azul** | (70, 130, 255) | (160, 190, 255) | (40, 80, 200) | (18, 22, 35) |
| **Morado** | (120, 60, 230) | (170, 130, 255) | (80, 30, 160) | (20, 15, 28) |

**Tierras, Metales y Escala de Grises**

| Familia | Base (RGB) | Light `_l` | Dark `_d` | Deep Dark `_dd` |
| :--- | :--- | :--- | :--- | :--- |
| **Marrón** | (110, 70, 50) | (160, 120, 100) | (70, 40, 30) | (22, 18, 15) |
| **Cobre** | (185, 100, 60) | (220, 150, 120) | (130, 60, 30) | (28, 20, 15) |
| **Plata** | (170, 175, 185) | (210, 215, 225) | (120, 125, 135) | (40, 42, 45) |
| **Oro** | (215, 170, 70) | (240, 210, 130) | (160, 120, 40) | (28, 25, 18) |
| **Gris** | (130, 135, 145) | (180, 185, 195) | (80, 85, 95) | (35, 37, 40) |
| **Negro** | (20, 20, 25) | (50, 50, 60) | (10, 10, 12) | (0, 0, 0) |



Además de las familias anteriores, la librería incluye los siguientes colores especializados (sin variantes obligatorias):

* **Básicos:** `blanco`, `ocre`, `cafe`, `violeta`.
* **Grises de Interfaz:** `gris_claro`, `gris_oscuro`.
* **Muted (Gráficos):** `verde_bosque`, `azul_noche`, `terracota`.
* **Paneles UI:** `fondo_panel`, `panelUp`, `panelDown`.

### Paletas generativas
Además de los colores nombrados (`dsv::Color::verde`, `dsv::Color::naranja`, etc.), hay **paletas generativas** para colorear series múltiples:


```cpp
// Paletas indexadas — devuelven sf::Color para la serie i de N total realizando una interpolacion entre colores por defecto
sf::Color col = dsv::Color::Oceano(i, numSims);
sf::Color col = dsv::Color::Magma(i, numSims);
sf::Color col = dsv::Color::Cyberpunk(i, numTotal);
sf::Color col = dsv::Color::Arcoiris(i, numTotal);
sf::Color col = dsv::Color::FuegoHielo(i, numTotal);
sf::Color col = dsv::Color::Viridis(i, numTotal);
sf::Color col = dsv::Color::Plasma(i, numTotal);
sf::Color col = dsv::Color::Bosque(i, numTotal);


// Paletas completas — devuelven std::vector<sf::Color>
// usadas para asignar degradados a series individuales o al gestor
std::vector<sf::Color> paleta = dsv::Color::Oceano();
std::vector<sf::Color> paleta = dsv::Color::Arcoiris();
// los demas ejemplos Oceano, Magma, Cyberpunk, Arcoiris... tambien funcionan

// Aplicación masiva al gestor
g.setColorSeries( dsv::Color::Oceano() );
```
Tambien se puedde crear su propio degradado usando dos colores o una paleta (un vector de colores)
```cpp
sf::Color interpolarColores(sf::Color a, sf::Color b, float t );
sf::Color obtenerColorDegradado(const std::vector<sf::Color>& paleta, float t);
```

Inlcuimos un modificador de alpha sobre colores nombrados
```cpp
dsv::Color::naranja_dd % 230  // devuelve el color con alpha = 230
```



---


## 6. Interfaz de Usuario (Paneles y Menús)

### 6.0 Panel


**——Metodos de Configuración——**


* **`positionAbsoluta(Ubicacion ubi)`**: Posiciona el panel en puntos clave de la pantalla (ej. `dsv::Ubicacion::CentroDer`).
* **`setColorFondo(sf::Color color)`**: Define el color sólido del cuerpo del panel.
* **`setDegradado(sf::Color top, sf::Color bot)`**: Aplica un gradiente vertical al fondo del panel.
* **`setBorderWidth(float grosor)`**: Ajusta el grosor del borde exterior.
* **`setRadio(float r)`**: Define la curvatura de las esquinas del panel para una estética redondeada.
* **`abrir()` / **`cerrar()`**: Permiten controlar programáticamente el estado de visibilidad del contenido del panel. (este metodo es unico para PanelFlotante)

### 6.1. PanelFlotante: MENU 

El PanelFlotante actúa como el marco principal de la interfaz. Se compone de un "Handle" (una pestaña interactiva siempre visible) y un "Cuerpo" (el contenedor de los controles) que se despliega al interactuar con el handle.


Para controlar la simulación en tiempo real, creamos un `PanelFlotante` y le agregamos un `MenuFlotante` estructurado por filas.

**-- Crear el Contenedor (Panel) --**

```cpp
PanelFlotante(
    sf::RenderWindow& ventana,
    std::string titulo,
    HandleOrientacion orientacion,
    DespliegueDir despliegue,
    sf::Color colorBorde,
);
```


| Parámetro | Tipo | Obligatorio | Descripción |
| :--- | :--- | :--- | :--- |
| **ventana** | `sf::RenderWindow&` | Sí | Referencia a la ventana de renderizado de SFML. |
| **titulo** | `std::string` | Sí | Texto descriptivo que aparece en el handle del panel. |
| **orientacion** | `HandleOrientacion` | No | Horizontal o Vertical. Define la geometría del handle. |
| **despliegue** | `DespliegueDir` | No | Dirección hacia la que se expande el cuerpo (Abajo, Arriba, Der, Izq.). |
| **colorBorde** | `sf::Color` | No | Color del contorno del panel y del handle (por defecto Blanco). |


```cpp
dsv::PanelFlotante panelControl(
    window, 
    "Controles",                         // Texto del handle
    dsv::HandleOrientacion::Vertical,    // Orientación de la pestaña
    dsv::DespliegueDir::Der,             // Hacia dónde se abre
    dsv::Color::morado                   // Color base
);
panelControl.positionAbsoluta(dsv::Ubicacion::CentroIzq);
```


### 6.2. MenuFlotante y CampoControl



** Gestión de Memoria y Ciclo de Vida:** La librería utiliza punteros inteligentes (`std::unique_ptr`) para gestionar de forma segura la jerarquía de la interfaz de usuario (UI). La estructura de propiedad funciona de la siguiente manera:

* **El PanelFlotante** es dueño del Cuerpo.
* **El Cuerpo** posee un `Objeto*` (el `MenuFlotante`).
* **El MenuFlotante** gestiona una lista de `CampoControl`.
* **Cada CampoControl** es dueño de sus widgets (`Campo`).

> **Nota para el Desarrollador:** No es necesario gestionar la memoria manualmente. Al utilizar el método `crearContenido<T>()`, la librería devuelve un puntero cuyo ciclo de vida es administrado automáticamente por la jerarquía del panel, evitando fugas de memoria (*memory leaks*).

```cpp
// Crear el menú dentro del panel
dsv::MenuFlotante* menu = panelControl.crearContenido<dsv::MenuFlotante>(6.f, 12.f);

// --- Añadir Campos ---
// 1. Campo Variable (Solo lectura, muestra texto y valor actual)
dsv::CampoControl& fila1 = menu->agregarFila();
fila1.agregar<dsv::CampoVariable>("FPS: ", &fpsActuales);

// 2. Campo Deslizador (Slider para modificar variables del modelo)
dsv::CampoControl& fila2 = menu->agregarFila();
fila2.agregar<dsv::CampoDeslizador>("Nacimientos ", &modelo_comun.bO, 0.0f, 1.0f);

// 3. Campo Barra (Barra de progreso de colores para estado actual)
dsv::CampoControl& fila3 = menu->agregarFila();
fila3.agregar<dsv::CampoBarra>("Población ", &colonia.state[Ants::O], 0.f, 500.f, dsv::Color::verde);

// 4. Campo Botón (Ejecuta una función lambda al hacer clic)
dsv::CampoControl& fila4 = menu->agregarFila();
fila4.agregar<dsv::CampoBoton>("Reset", [&]{ colonia.t = 0; });

// 5. Campo Toggle (Interruptor On/Off para booleanos)
bool enPausa = false;
dsv::CampoControl& fila5 = menu->agregarFila();
fila5.agregar<dsv::CampoToggleTexto>("", &enPausa, "Play", "Pausa", dsv::Color::naranja);
```

Para mas detalle se muestra acontinuacion
### 6.3. Catálogo de Componentes (Campos)

Cada widget de control se añade a una instancia de `CampoControl` (una fila del menú).

#### A. CampoVariable (Monitor de Solo Lectura)

Muestra el valor actual de una variable en tiempo real. Útil para telemetría y FPS.

**Firma:** `agregar<CampoVariable>(std::string etiqueta, float* punteroValor)`

**Parámetros:**
* **etiqueta:** Texto identificador.
* **punteroValor:** Dirección de memoria de la variable a monitorear.


```cpp
CampoVariable(const std::string& nombre,
                float*             valor,
                bool               esEntero = false,
                bool               esBool   = false,
                sf::Color          colorNombre = Color::gris_claro,
                sf::Color          colorValor  = Color::blanco)
```
---

#### B. CampoDeslizador (Slider)
Permite modificar una variable numérica dentro de un rango definido mediante una barra de desplazamiento.

**Firma:** `agregar<CampoDeslizador>(std::string etiqueta, float* val, float min, float max)`

```cpp
CampoDeslizador(const std::string& nombre,
                    float*             valor,
                    float              minVal,
                    float              maxVal,
                    sf::Color          colorTrack  = Color::fondo_panel,
                    sf::Color          colorRelleno= Color::azul,
                    sf::Color          colorThumb  = Color::azul_l,
                    float              anchoFijo   = 120.f)
```

**Ejemplo:**
```cpp
fila.agregar<dsv::CampoDeslizador>("Gravedad", &fisica.g, -9.8f, 20.0f);
```

---

#### C. CampoBarra (Progreso/Estado)
Visualiza una variable escalar como una barra de relleno con color personalizado.

**Firma:** `agregar<CampoBarra>(std::string etiqueta, float* val, float min, float max, sf::Color color)`

**Parámetros:**
* **color:** Define el tono del relleno (ej. `dsv::Color::verde` para salud o energía).

```cpp
CampoBarra(const std::string& nombre,
            float*             valor,
            float              minVal,
            float              maxVal,
            sf::Color          colorRelleno = Color::azul,
            sf::Color          colorFondo   = Color::fondo_panel,
            sf::Color          colorBorde   = Color::gris_oscuro,
            float              anchoFijo    = 120.f)
```


---

#### D. CampoBoton (Acción Instantánea)
Ejecuta un bloque de código (lambda) al ser presionado.

**Firma:** `agregar<CampoBoton>(std::string texto, std::function<void()> callback)`
Mas ajustees
```cpp
CampoBoton(const std::string&    texto,
               std::function<void()> accion,
               sf::Color             colorFondo  = Color::fondo_panel,
               sf::Color             colorBorde  = Color::gris_oscuro,
               float                 anchoFijo   = 90.f)
```

**Ejemplo:**
```cpp
fila.agregar<dsv::CampoBoton>("Reset Sistema", [&](){ sistema.reiniciar(); });
```

---

#### E. CampoToggleTexto (Interruptor)
Alterna entre dos estados booleanos, mostrando etiquetas de texto distintas para cada estado.

**Firma:** `agregar<CampoToggleTexto>(std::string etiqueta, bool* estado, std::string txtOn, std::string txtOff, sf::Color colActive)`

**Parámetros:**
* **txtOn / txtOff:** Textos para los estados `true` y `false`.
* **colActive:** Color del widget cuando el estado es activo.

```cpp
CampoToggle(const std::string& etiqueta,
                bool*              ptrEstado,
                sf::Color          colorOn    = Color::verde,
                sf::Color          colorOff   = Color::fondo_panel,
                sf::Color          colorBorde = Color::gris_oscuro)
        : nombre(etiqueta), estado(ptrEstado),
```



### 6.4. Renderizar y Gestionar Eventos


**Procesamiento de Eventos:** Para asegurar la interactividad (clicks, hover, dragging) y el correcto renderizado, el panel debe recibir los eventos de SFML antes que otros sistemas si se desea que la UI "bloquee" la interacción con el fondo.

```cpp
sf::Event event;
while (window.pollEvent(event)) {
    // El panel calcula colisiones de mouse y cambios de estado interno
    panelControl.gestionarEvento(event);
}
```

**Actualización y Renderizado:**
El método `.draw()` del panel debe invocarse al final de la función de dibujo para asegurar que la interfaz se superponga a las gráficas y simulaciones.

```cpp
window.clear();
tablero.draw();          // 1. Dibujar gráficas de fondo
panelControl.draw();     // 2. Dibujar UI encima
window.display();
```

> **Nota de Eficiencia:** Los componentes de la interfaz solo recalculan sus geometrías cuando se detecta un cambio en el estado del panel (abrir/cerrar) o una interacción directa del usuario, minimizando el impacto en los frames por segundo (FPS) de la simulación.

---

## 7. Diccionario de Funciones

* **`activarAutoescalado(bool)`**: Ajusta automáticamente los límites X/Y de una `EspacioFase2D` a los datos entrantes.
* **`activarSeguimiento(bool)`**: Centra la cámara de una `EspacioFase2D` en el último punto agregado.
* **`agregarSerie(string, color)`**: Registra una nueva línea en gráficas 2D para admitir inyección multiserie.
* **`configurarLimites(minX, maxX, minY, maxY)`**: Fija rígidamente los ejes de la gráfica 2D.
* **`configurarVentanaTiempo(float)`**: Define cuántos "segundos" de simulación se ven en una `GraficaTiempo`.
* **`draw()`**: Función obligatoria para renderizar el `Tablero` o un `PanelFlotante`.
* **`gestionarEvento(event)`**: Enruta clics y movimientos del mouse a los Paneles Flotantes.
* **`personalizarColores(vector<Color>)`**: Cambia la paleta de un `GraficoCircular`.
* **`ponerDesvanecido(bool)`**: Activa el fade-out (transparencia) en las colas de trayectorias.
* **`push_back(...)`**: Inyecta datos a la gráfica. Sus parámetros varían según el tipo de gráfica (1 variable, 2 variables, vectores, etc.).
* **`setLimites(min, max)`** *(en Ejes3D)*: Define el cubo invisible en el que se pintan los ejes XYZ.
* **`step(instancia, dt)`**: Avanza matemáticamente la simulación un paso de tiempo `dt`.
* **`vincular(modelo)`**: Conecta una instancia a un bloque de parámetros externo compartido.

Referencia rapida de la API `Vista<T>` segun el tipo
// ── GraficaTiempo ──────────────────────────────────────────
vista->agregarSerie("nombre", sf::Color);
vista->push_back(valor, t);
vista->push_back(valor, t, "serie");
vista.objeto.configurarVentanaTiempo(segundos);
vista.objeto.configurarLimitesY(min, max);
vista.objeto.configurarLimites(xMin, xMax, yMin, yMax, autoX);
vista.objeto.ponerSombreado(bool);
vista.panel.setSizeTitulo(int);

// ── EspacioFase2D ──────────────────────────────────────────
vista->push_back(x, y);
vista->push_back(x, y, "serie");
vista->agregarSerie("nombre", sf::Color);
vista->configurarLimites(xMin, xMax, yMin, yMax);
vista->activarAutoescalado(bool);
vista->activarSeguimiento(bool);     // encuadre dinámico
vista->ponerDesvanecido(bool);       // fade en la cola
vista->ponerCabeza(bool);            // punto en posición actual
vista->configurarMaxPoints(n);

// ── GraficoCircular ───────────────────────────────────────
vista->push_back({v1, v2, v3});
vista->personalizarColores({col1, col2, col3});

// ── Grafica3D ─────────────────────────────────────────────
vista.objeto.getEjes().setLimites(min, max);
vista.objeto.getGestor().agregarSerie(id_o_nombre, col_o_paleta);
vista.objeto.getGestor().push_back({x,y,z}, id_o_nombre);
vista.objeto.getGestor().setGrosorSeries(float);
vista.objeto.getGestor().setAdelgazadoSeries(bool);
vista.objeto.getGestor().setDifuminadoSeries(bool);
vista.objeto.getGestor().setMaxPointsSeries(n);
vista.objeto.getGestor().setColorSeries(paleta);
vista.objeto.getGestor().setColor(col, id_o_nombre);
vista.objeto.getGestor().setGrosor(float, id_o_nombre);
vista->gestionarEvento(event, window); // ← firma diferente

## 8. Extras


###  Truco de proyecciones 

El ejemplo se demuestra una técnica  **proyecciones sobre planos** usando series adicionales con una coordenada fija:

```cpp
// Serie real
fase3D_proye.objeto.getGestor().push_back({x, y, z}, i);
// Proyección sobre plano YZ (x fijo en el borde)
fase3D_proye.objeto.getGestor().push_back({-40, y, z}, i + numSims*1);
// Proyección sobre plano XZ (y fijo)
fase3D_proye.objeto.getGestor().push_back({x, -40, z}, i + numSims*2);
// Proyección sobre plano XY (z fijo)
fase3D_proye.objeto.getGestor().push_back({x, y, -40}, i + numSims*3);
```

Cada "sombra" es simplemente una serie nueva con un eje colapsado a un valor constante que coincide con la pared del cubo visible.


