/*  * autor: Angel Manuel Gonzalez Lopez 
    * github: https://github.com/angelmanuelgl
    * web: https://angelmanuelgl.github.io/
    * 
    * proyecto: InsightRT - - - - - - - - - - - - - - - - - - - 
    * libreria de herramientas graficas para monitoreo de datos 
    * en tiempo real y comportamiento de sistemas complejos.
*/
/*  Temas.hpp
    sirvira para gestionar colores generales, 
    poder elegir en assets/config/colores.txt una paleta separado del ejecutable 

    nota: algunos colores fueorn tomados de https://pub.norden.org/designmanual-en/colours.html
*/
#ifndef TEMAS_HPP
#define TEMAS_HPP

#include "Logger.hpp"

#include <SFML/Graphics.hpp>
#include <cmath>
#include <algorithm>
#include <fstream>
#include <iostream>

// nota no usa enum class porque aqui guardamos info
// Temas.hpp
namespace Tema{
    
    //  mapa  se guardan todos los colores del txt
    inline std::map<std::string, sf::Color> colores;

    // obtener cualquier color por su nombre en el txt
    inline sf::Color obtener(const std::string& nombre) {
        if( colores.find(nombre) != colores.end()) {
            return colores[nombre];
        }
        return sf::Color::Magenta; // color de error si no existe
    }
    
    // para acceder mas facil
    inline sf::Color c(const std::string& nombre) { return obtener(nombre); }

    // --- lectura --- 
    bool siEs( int valor ){
        return valor >= 0 && valor <= 255;
    }

    void cargar(const std::string& ruta) {
        std::ifstream file(ruta);

        std::string tag;
        int r, g, b, a;
        std::string posible;

        while( file >> tag  ){
            // es posible agregar comentarios con /
            if( tag.size() >= 1 && tag[0] == '/') {
                std::string basura;
                std::getline(file, basura); // Consume el resto de la linea y la tira
                continue;
            }
            
            // si no hay nada mas por leer
            if (!(file >> posible)) break; 

            // --- un color ya usado ---
            if(  std::isalpha( static_cast<unsigned char>(posible[0]) )  ){
                if( colores.count(posible) ){
                    colores[tag] = colores[posible];
                } 
                // no has definido el color ya sadp
                else {
                    std::cerr << "ERROR: El alias '" << posible << "' no existe todavia.\n";
                }
            }
            // --- nuevo color ---
            else{
                // pasar la estring
                try{
                    r = std::stoi(posible);
                    file >> g >> b >> a;
                    if(  siEs(r) && siEs(g) && siEs(b) && siEs(a)  ){
                        colores[tag] = sf::Color(r,g,b,a);
                    }else{
                        DSV_LOG_ERROR("ERROR: Valores fuera de rango (0-255) en " + tag);
                    }
                }catch (...){
                    DSV_LOG_ERROR("ERROR:  Formato numerico invalido en " + tag);
                    // mandar  la basura el resto por si en als demas lineas si hay algo
                    std::string basura;
                    std::getline(file, basura);
                }

            }
            
        } 
    }
}



 // --- configurar ventana ---
 namespace Sistema {
    // Estructura para devolver los parametros configurados
    struct ConfigVentana {
        unsigned int ancho;
        unsigned int alto;
        sf::ContextSettings settings;
    };

    inline ConfigVentana obtenerConfiguracionIdeal(float escala = 0.8f) {
        ConfigVentana config;
        
        //  Antialiasing (Suavizado de bordes)
        config.settings.antialiasingLevel = 8;

        // resolucion del monitor
        sf::VideoMode desktop = sf::VideoMode::getDesktopMode();

        // calcular tam relativo
        config.ancho = static_cast<unsigned int>(desktop.width * escala);
        config.alto = static_cast<unsigned int>(desktop.height * escala);

        return config;
    }

    // crear la venta
    inline void inicializarVentana(sf::RenderWindow& window, const std::string& titulo) {
        ConfigVentana config = obtenerConfiguracionIdeal(1.0f);

        window.create(sf::VideoMode(config.ancho, config.alto), 
                      titulo, 
                      sf::Style::Fullscreen, //  Default // Fullscreen
                      config.settings);
        
        window.setFramerateLimit(60);

        // window.setPosition(sf::Vector2i(
        //     (desktop.width - ancho) / 2,
        //     (desktop.height - alto) / 2
        // ));
    }
}


#endif