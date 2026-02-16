#ifndef OBJETO_HPP
#define OBJETO_HPP

#include <SFML/Graphics.hpp>

#include<logger.hpp>


// Objeto.hpp
class Objeto {
public:
    sf::Font font;

    // La fuente es estática: compartida por todas las instancias
    static sf::Font fuenteCompartida;
    static bool fuenteCargada;

    // buscar fuente
    static void cargarFuenteGlobal() {
        if( fuenteCargada ) return; // Si ya se cargo, no hacer nada

        std::vector<std::string> rutas = {
            "../../assets/fonts/Roboto.ttf",
            "../assets/fonts/Roboto.ttf",
            "assets/fonts/Roboto.ttf"
        };

        for( const auto& ruta : rutas ){
            if( fuenteCompartida.loadFromFile(ruta) ){
                DSV_LOG_SUCCESS("Fuente cargada globalmente desde: " + ruta);
                fuenteCargada = true;
                return;
            }
        }
        DSV_LOG_ERROR("NO se encontro Roboto.ttf en ninguna ruta");
    }

    Objeto() {
        // si no esta cargada se carga
        cargarFuenteGlobal();
    }

    virtual ~Objeto() = default;
    virtual void draw(sf::RenderWindow& window, sf::RenderStates states, sf::Vector2f pSize) = 0;
};


//  variables estáticas fuera de la clase 
// 'inline' (C++17)
inline sf::Font Objeto::fuenteCompartida;
inline bool Objeto::fuenteCargada = false;


#endif