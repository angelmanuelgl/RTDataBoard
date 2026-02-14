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

struct Limites {
    float minX, maxX, minY, maxY;
};

class GraficaBase : public Objeto {
protected:
    // linea 
    unsigned int maxPoints;
    sf::Color lineaResaltado;

    // ejes
    std::string nombreEjeX, nombreEjeY;
    std::string unidadEjeX, unidadEjeY;
    int numMarcasX, numMarcasY;
    
    // efectos   
    bool mostrarEtiquetasEjes; 
    bool sombreado;
    bool desvanece;
    bool sombreadoAlEje;


    // datos
    std::vector<sf::Vector2f> puntos; //  pares (x, y)
    Limites lim;

public:
    GraficaBase(unsigned int maxPts, sf::Color color);

    virtual ~GraficaBase() {}

    // neceistan implementar las hijas, solo por peroosnalziacion
    virtual void recalcularExtremos(void) = 0;

    // preproceso
    std::string getEtiquetaY(int indice);
    std::string getEtiquetaX(int indice);

    // --- estilo ---
    void setMostrarEtiquetas(bool mostrar) { mostrarEtiquetasEjes = mostrar; }
    void configurarEjes(std::string nx, std::string ux, std::string ny, std::string uy) { nombreEjeX = nx; unidadEjeX = ux; nombreEjeY = ny; unidadEjeY = uy; }
    void configurarMarcas(int mx, int my) { numMarcasX = mx; numMarcasY = my; }
    void configurarMaxPoints(int mp) { maxPoints = mp; }
    void configurarLimites( float mx, float MX, float my, float MY ){  lim = {mx,MX,my,MY};  }

    void ponerSombreado( bool s, bool eje = true ){ sombreado = s; sombreadoAlEje = eje;}
    void ponerDesvanecido( bool s ){ desvanece = s;}
    
    // dibujar
    void dibujarContenido(sf::RenderWindow& window, sf::RenderStates states, float paddingL, float offsetTop, float graphWidth, float graphHeight);
    void draw(sf::RenderWindow& window, sf::RenderStates states, sf::Vector2f pSize);
};

class GraficaTiempo : public GraficaBase {
private:
    float contadorSegundos;
    
public:
    GraficaTiempo(sf::Color color);
    // --- datos ---
    void recalcularExtremos(void) override;
    void addValue(float val);
};

class GraficaEspacioFase : public GraficaBase {
private:
    //

public:
    GraficaEspacioFase(sf::Color color);
    // --- datos ---
    void recalcularExtremos(void) override;
    void addValue(float x, float y);
};

#endif