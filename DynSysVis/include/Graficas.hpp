/*  * autor: Angel Manuel Gonzalez Lopez 
    * github: https://github.com/angelmanuelgl
    * web: https://angelmanuelgl.github.io/
    * 
    * proyecto: InsightRT - - - - - - - - - - - - - - - - - - - 
    * libreria de herramientas graficas para monitoreo de datos 
    * en tiempo real y comportamiento de sistemas complejos.
*/
/*  GEOMETRIA.hpp
    sistema de clases base para graficar
    clases derivadas: para la visualización de series temporales  
                      para retratos de fase.
*/

#ifndef GRAFICAS_HPP
#define GRAFICAS_HPP


// #include "Panel.hpp" // para que reconozca la clase panel 
#include "Objeto.hpp" // para que conozca el Objeto generico

#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <algorithm>
#include <map>          // para las series
#include <functional>   //  mapearPunto

/*  
    --- --- --- --- --- --- --- --- ---     
    --- --- --- AUXILIARES  --- --- --- 
    --- --- --- --- --- --- --- --- --- 
*/
struct Limites {
    float minX, maxX, minY, maxY;
};

class Serie {
private:
    // info de esta serie
    std::string nombre;
    sf::Color color;
    std::vector<sf::Vector2f> puntos;

    // info que sera compartida
    unsigned int maxPoints;
    

    // cache de limites para no iterar 60 veces por segundo si no hay datos nuevos
    Limites misLimites = {0, 0, 0, 0};
    bool primerPuntoAgregado = false;

public:
    // constructores
    Serie() : nombre(""), color(sf::Color::White), maxPoints(500) {}
    Serie(std::string nombre, sf::Color color, unsigned int maxPts) 
        : nombre(nombre), color(color), maxPoints(maxPts){}

    // utilidades
    void recalcularExtremos(void);
    void agregarPunto(sf::Vector2f p);

    // dibujar
    void draw(sf::RenderWindow& window, sf::RenderStates states, 
                std::function<sf::Vector2f(sf::Vector2f)> mapearPunto,
                bool sombreado=false, bool desvanece=false, float valorReferenciaY=0);
    
    // SETs
    void setColor(sf::Color col){ color = col; }
    void setMaxPoints(int mp){ maxPoints = mp; }

    // GETs
    const std::vector<sf::Vector2f>& getPuntos() const { return puntos; }
    sf::Color getColor() const { return color; }
    std::string getNombre() const { return nombre; }
    bool vacia() const { return puntos.empty(); }
    Limites getLimites() const { return misLimites; }
};



/*  
    --- --- --- --- --- --- --- --- ---     
    --- --- --- GRAFICA GENERICA  --- --- --- 
    --- --- --- --- --- --- --- --- --- 
*/




class GraficaBase : public Objeto {
protected:
    // lineas
    std::map<std::string, Serie> series;
    std::string seriePrincipal = "default";

    // datos de todas las lineas
    unsigned int maxPoints;
    sf::Color lineaResaltado;
    bool sombreado; 
    bool sombreadoAlEje;
    bool desvanece;

    Limites lim;
    bool autoEscalado = true;
    // ejes
    std::string nombreEjeX, nombreEjeY;
    std::string unidadEjeX, unidadEjeY;
    int numMarcasX, numMarcasY;
    
    // efectos   
    bool mostrarEtiquetasEjes; 
 

   

public:
    GraficaBase(unsigned int maxPts, sf::Color color);


    virtual ~GraficaBase() {}

    // neceistan implementar las subclases
    virtual void recalcularExtremos(void) = 0;

    // interactuar con las series
    void addValueGenerico(sf::Vector2f p, std::string clave ="");
    void agregarSerie(std::string nombre, sf::Color color );
    void configurarMaxPoints(int mp);

    // preproceso
    std::string getEtiquetaY(int indice);
    std::string getEtiquetaX(int indice);

    // --- estilo ---
    void setMostrarEtiquetas(bool mostrar) { mostrarEtiquetasEjes = mostrar; }
    void configurarEjes(std::string nx, std::string ux, std::string ny, std::string uy) { nombreEjeX = nx; unidadEjeX = ux; nombreEjeY = ny; unidadEjeY = uy; }
    void configurarMarcas(int mx, int my) { numMarcasX = mx; numMarcasY = my; }

    void configurarLimites( float mx, float MX, float my, float MY, bool autoEsc = false ){  
        lim = {mx,MX,my,MY}; 
        autoEscalado = autoEsc;  
    }

    void ponerSombreado( bool s, bool eje = true ){ sombreado = s; sombreadoAlEje = eje;}
    void ponerDesvanecido( bool s ){ desvanece = s;}
    
    // dibujar
    void dibujarContenido(sf::RenderWindow& window, sf::RenderStates states, float paddingL, float offsetTop, float graphWidth, float graphHeight);
    void draw(sf::RenderWindow& window, sf::RenderStates states, sf::Vector2f pSize);
};

/*  
    --- --- --- --- --- --- --- --- ---     
    --- --- --- ESPECIALZIACIONES  --- --- --- 
    --- --- --- --- --- --- --- --- --- 
*/


class GraficaTiempo : public GraficaBase {
private:
    float contadorSegundos;
    
public:
    GraficaTiempo(sf::Color color);
    // --- datos ---
    void recalcularExtremos(void) override;
    void addValue(float val, std::string clave ="");
};

class GraficaEspacioFase : public GraficaBase {
private:
    //

public:
    GraficaEspacioFase(sf::Color color);
    // --- datos ---
    void recalcularExtremos(void) override;
    void addValue(float x, float y, std::string clave ="");
};

#endif