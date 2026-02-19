/*  * autor: Angel Manuel Gonzalez Lopez 
    * github: https://github.com/angelmanuelgl
    * web: https://angelmanuelgl.github.io/
    * 
    * - - - - - - -  -  DynSysVis  - - - - - - - - - - 
    * Dynamical System Visualizer Real-Time
    * libreria de herramientas graficas para monitoreo de datos 
    * y comportamiento de sistemas complejos en tiempo Real.
*/
/*  PANEL.hpp
    gestion de paneles dinamicos
    posicionamiento y obtienen mas


    el gestor del contenido, esto unifica
    -> objeto generico
    -> titulo
    -> posciion
*/
#ifndef PANEL_HPP
#define PANEL_HPP


#include "Geometria.hpp" // para que dibuje
#include "Objeto.hpp" // para que conozca el Objeto generico // no es necesarioq ue conozca todos los objetos
#include "Titulo.hpp" 
#include "Logger.hpp"

#include <SFML/Graphics.hpp>
#include <memory>        // Para unique_ptr
#include <iostream>     // Para manejo de errores


namespace dsv{

enum class Ubicacion {
    ArribaIzq, ArribaCentro, ArribaDer,
    AbajoIzq, AbajoCentro, AbajoDer,
    CentroIzq, CentroDer, Centro
};

enum class RelativoA {
    Arriba, Abajo, Izq, Der
};

class Panel {
private:
    // referencia al padre
    sf::RenderWindow& window;
   
    // elementos 
    RectanguloRedondeado elMarco;
    std::unique_ptr<Titulo> titulo;
    std::unique_ptr<Objeto> contenido;
    
    // el panel en si
    sf::Transform mytransform; 
    sf::Vector2f pos_actual;
    
    // poscionar //todo: hacer una clase que contenga todos los paneles // para tener medidas iguales para todos
    float espaciado = 15.f;
    float margenVentana = 20.f;

    // para el recntauglo
    float radio = 20.0f;
    sf::Vector2f size;
    sf::Color extColor;
    sf::Color bgColor;

    // para titulo
    bool yafuenteCargada;
    sf::Font fuentePredeterminada;


public:
    Panel(sf::RenderWindow& window_, 
                const std::string& tituloPanel ="", 
                sf::Color extColor = sf::Color::White,
                double nx = 3, double ny = 4, // para tamano
                sf::Color bgColor=sf::Color(30,30,30) ); // color de fondo

    void configurarMedidas( float r, float esp, float margen);
       
 
    // posicionamiento 
    void setSize(double nx, double ny);
    void setPosition(float x, float y);
    [[deprecated("Usa positionEnRejilla para un layout mas limpio")]]
    void positionAbsoluta(Ubicacion ubi);
    [[deprecated("Usa positionEnRejilla para un layout mas limpio")]]
    void positionRelativa(RelativoA rel, const Panel& other);
    

    // nuevo posiconamiento
    void sizeEnRejilla(int spanF, int spanC, int totalFilas, int totalCols);
    void positionEnRejilla(int fila, int col, int totalFilas, int totalCols);

    // dibujar
    void draw(void);
    
    // GETs
    sf::Vector2f getPosition() const { return pos_actual; }
    sf::Vector2f getSize() const { return size; }
    sf::RenderStates getInternalState() const;

    // -- titulo --- 
    void cargarFuenteSiFalta();
    void ponerTitulo(const std::string& texto, const sf::Font& fuente);
    void setSizeTitulo( unsigned int s ){ titulo-> setSize(s); }

    // --- como interactua con el contenido --- 
    void setContenido( std::unique_ptr<Objeto> nuevoContenido );

    template <typename T, typename... Args>
    T* crearContenido(Args&&... args ){
        
        
        //  crear la grafica o lo que le pasen
        auto grafico = std::make_unique<T>(std::forward<Args>(args)...);
        
        // guardamos el puntero
        T* ptr = grafico.get();
        
        // hacemos que el grafico se guarde en contenido
        contenido = std::move(grafico);
        
        return ptr;
    }
    
};

// end dsv
}
#endif