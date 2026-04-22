/*  * autor: Angel Manuel Gonzalez Lopez 
    * github: https://github.com/angelmanuelgl
    * web: https://angelmanuelgl.github.io/
    * 
    * - - - - - - -  -  DynSysVis  - - - - - - - - - - 
    * Dynamical System Visualizer Real-Time
    * libreria de herramientas graficas para monitoreo de datos 
    * y comportamiento de sistemas complejos en tiempo Real.
*/

/*  GEOMETRIA.hpp
    sistema de clases base para graficar
    clases derivadas: para la visualización de series temporales  
                      para retratos de fase.
*/

#ifndef GRAFICAS_HPP
#define GRAFICAS_HPP


// std y SFML
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <algorithm>
#include <map>          // para las series
#include <functional>   //  mapearPunto

// dsv
#include "dsv/core/Objeto.hpp" // para que conozca el Objeto generico
#include "dsv/core/Logger.hpp"

#include "dsv/dataStructures/Estructuras.hpp" // MonotonicMaxQueue


namespace dsv{
/*  
    --- --- --- --- --- --- --- --- ---     
    --- --- --- AUXILIARES  --- --- --- 
    --- --- --- --- --- --- --- --- --- 
*/


class Serie2D {
private:
    // info de esta serie
    std::string nombre;
    sf::Color color;
    MonotonicQueue2D<float> puntos;
    // info que sera compartida
    unsigned int maxPoints;
    

    // cache de limites para no iterar 60 veces por segundo si no hay datos nuevos
    Limites misLimites = {0, 0, 0, 0};
    bool primerPuntoAgregado = false;

public:
    // constructores
    Serie2D() : nombre(""), color(sf::Color::White), maxPoints(500) {}

    Serie2D(std::string nombre, sf::Color color, unsigned int maxPts) 
        : nombre(nombre), color(color), maxPoints(maxPts){}

    // utilidades
    void recalcularExtremos(void);
    void agregarPunto(sf::Vector2f p);

    // dibujar
    void draw(sf::RenderWindow& window, sf::RenderStates states, 
                std::function<sf::Vector2f(sf::Vector2f)> mapearPunto,
                bool sombreado=false, bool desvanece=false, bool cabeza = false,
                float valorReferenciaY=0);
    void aportarCabeza(sf::VertexArray& arrayGlobal, std::function<sf::Vector2f(sf::Vector2f)> mapearPunto);
    
    // ila seire interactua dcon los datos



    // SETs
    void setColor(sf::Color col){ color = col; }
    void setMaxPoints(int mp){ maxPoints = mp; }
    
    // GETs
    sf::Color getColor() const { return color; }
    std::string getNombre() const { return nombre; }
    bool vacia() const { return puntos.empty(); }
    Limites getLimites() const { return misLimites; }
    // serie
    sf::Vector2f back(){ return puntos.back(); }
    sf::Vector2f front(){ return puntos.front(); }
    void pop( ){ puntos.pop(); }
};



/*  
    --- --- --- --- --- --- --- --- ---     
    --- --- --- GRAFICA GENERICA  --- --- --- 
    --- --- --- --- --- --- --- --- --- 
*/




class GraficaBase : public Objeto {
protected:
    // lineas
    std::map<std::string, Serie2D> series;
    std::string seriePrincipal = "default";

    // datos de todas las lineas
    unsigned int maxPoints;
    sf::Color lineaResaltado;
    bool sombreado, sombreadoAlEje;
    bool desvanece, cabeza;

    Limites lim;
    
    // ejes
    std::string nombreEjeX, nombreEjeY;
    std::string unidadEjeX, unidadEjeY;
    int numMarcasX, numMarcasY;
    
    // efectos   
    bool mostrarEtiquetasEjes; 
 

   
    // --- --- --- tipos de escalado --- --- 
    enum { TIPOestatico, TIPOautoEscalado, TIPOautoSeguimiento }; // Mapeo de índices
    int estadoEES  = TIPOestatico;
    bool limitesPersonalizadosIngresados = false;
    bool YaSeActualizaronLimites = false; // la primera vez que se actualizan es dieferente porque antes no hay datos, asi que se ponen esos limites, si no se expanden


public:
    GraficaBase(unsigned int maxPts, sf::Color color);


    virtual ~GraficaBase(){}

    // neceistan implementar las subclases
    virtual void recalcularExtremos(void) = 0;

    // interactuar con las series
    void push_back_Gen(sf::Vector2f p, std::string clave ="");
    void agregarSerie(std::string nombre, sf::Color color );
    void configurarMaxPoints(int mp);

    // preproceso
    std::string getEtiquetaY(int indice);
    std::string getEtiquetaX(int indice);

    // --- estilo ---
    void setMostrarEtiquetas(bool mostrar) { mostrarEtiquetasEjes = mostrar; }
    void configurarEjes(std::string nx, std::string ux, std::string ny, std::string uy) { nombreEjeX = nx; unidadEjeX = ux; nombreEjeY = ny; unidadEjeY = uy; }
    void configurarMarcas(int mx, int my) { numMarcasX = mx; numMarcasY = my; }

    void configurarLimites( float mx, float MX, float my, float MY){  
        lim = {mx,MX,my,MY}; 
        limitesPersonalizadosIngresados = true;
    }
   

    void ponerSombreado( bool s, bool eje = true ){ sombreado = s; sombreadoAlEje = eje;}
    void ponerDesvanecido( bool s, bool c ){ desvanece = s, cabeza =c;}
    void ponerDesvanecido( bool s){ desvanece = s;}
    void ponerCabeza( bool c ){ cabeza =c;}
    
    // dibujar
    void dibujarContenido(sf::RenderWindow& window, sf::RenderStates states, float paddingL, float offsetTop, float graphWidth, float graphHeight);
    void draw(sf::RenderWindow& window, sf::RenderStates states, sf::Vector2f pSize);


    // --- --- --- tipos de escalado --- --- 
    void activarAutoescalado( bool s = true ){ 
        if(s) estadoEES = TIPOautoEscalado;
        else estadoEES = TIPOestatico;
      
    }
    void activarSeguimiento( bool s = true){ 
        if(s) estadoEES = TIPOautoSeguimiento;
        else estadoEES = TIPOestatico;
    }

    bool esEstatico(){ 
        bool r = (estadoEES == TIPOestatico);
        return r;
    }
    bool esEscalado(){ return (estadoEES == TIPOautoEscalado);}
    bool esSegumiento(){ return (estadoEES == TIPOautoSeguimiento);}

};

/*  
    --- --- --- --- --- --- --- --- ---     
    --- --- --- ESPECIALZIACIONES  --- --- --- 
    --- --- --- --- --- --- --- --- --- 
*/


class GraficaTiempo : public GraficaBase {
private:
    float contadorSegundos;
    float maxLimX; // cantiddad de ancho fija
    
public:
    GraficaTiempo(sf::Color color = sf::Color::Blue);
    // --- datos ---
    void recalcularExtremos(void) override;
    void push_back(float val, std::string clave ="");
    void push_back(float val , float t, std::string clave ="");
    
    void configurarVentanaTiempo(float maxlx){ maxLimX = maxlx; };
    void configurarLimitesY( float my, float MY){  
        lim = {0,0,my,MY}; 
        activarAutoescalado();
    }
};

class EspacioFase2D : public GraficaBase {
// diferencias entre seguimiento y autoescalado: 
// el auto escalado recalcula los limites de las series y ajusta la vista para que siempre se vean
// AUTOESCALADO: expande limites, pero no los centra, asi que si las series se mueven a un lado, la grafica se va a ese lado, pero siempre mostrando todo el rango de las series
// SEGUIMIENTO: ajusta los limites para que siempre esten centrados en las series, asi que si las series se mueven a un lado, la grafica se va a ese lado pero siempre centrada en las series, y con un margen extra para que no se mueva tan rapido
// ESTATICO 
private:
    
public:
    EspacioFase2D(sf::Color color = sf::Color::Blue );
    // --- datos ---
    void recalcularExtremos(void) override;
    void push_back(float x, float y, std::string clave ="");
};


// end dvs
}
#endif