/*  * autor: Angel Manuel Gonzalez Lopez 
    * github: https://github.com/angelmanuelgl
    * web: https://angelmanuelgl.github.io/
    * 
    * proyecto: InsightRT - - - - - - - - - - - - - - - - - - - 
    * libreria de herramientas graficas para monitoreo de datos 
    * en tiempo real y comportamiento de sistemas complejos.
*/
/*  PANEL.hpp
    gestion de paneles dinamicos
    posicionamiento y obtienen mas
*/
#ifndef PANEL_HPP
#define PANEL_HPP


#include "Geometria.hpp" // para que reconozca la clase de los rectangulos
#include "Objeto.hpp" // para que conozca el Objeto generico 
#include "Titulo.hpp" // para que conozca el Objeto generico 


#include <SFML/Graphics.hpp>
#include <memory>        // Para unique_ptr
#include <iostream>     // Para manejo de errores

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

    // para titulo
    bool yafuenteCargada;
    sf::Font fuentePredeterminada;


public:
    Panel(sf::RenderWindow& window_, sf::Color extColor, 
                const std::string& tituloPanel ="", 
                double nx = 3, double ny = 4, // para tamano
                sf::Color bgColor=sf::Color(30,30,30) ); // color de fondo


    sf::Vector2f getPosition() const { return pos_actual; }
    sf::Vector2f getSize() const { return size; }

    void positionAbsoluta(Ubicacion ubi);
    void positionRelativa(RelativoA rel, const Panel& other);
    void setPosition(float x, float y);
    void configurarMedidas( float r, float esp, float margen);
    
    sf::RenderStates getInternalState() const;
    void draw(void);
    
    // -- titulo --- 
    void cargarFuenteSiFalta();
    void ponerTitulo(const std::string& texto, const sf::Font& fuente);


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


#endif