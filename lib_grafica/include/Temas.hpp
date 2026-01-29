/*  * autor: Angel Manuel Gonzalez Lopez 
    * github: https://github.com/angelmanuelgl
    * web: https://angelmanuelgl.github.io/
    * 
    * proyecto: InsightRT - - - - - - - - - - - - - - - - - - - 
    * libreria de herramientas graficas para monitoreo de datos 
    * en vivo y comportamiento de sistemas complejos.
*/
/*  Temas.hpp
    sirvira para gestionar colores generales, 
    poder elegir en assets/config/colores.txt una paleta separado del ejecutable 

    nota: algunos colores fueorn tomados de https://pub.norden.org/designmanual-en/colours.html
*/
#ifndef TEMAS_HPP
#define TEMAS_HPP

#include <SFML/Graphics.hpp>
#include <cmath>
#include <algorithm>
#include <fstream>
#include <iostream>

// Temas.hpp
namespace Tema{
    // nota no usa enum class porque aqui guardamos info
    inline sf::Color Fondo;
    inline sf::Color Panel;
    inline sf::Color Obreras;
    inline sf::Color Guerreras;
    inline sf::Color Recolectoras;
    inline sf::Color Color1;
    inline sf::Color Color2;

    inline sf::Color dark_blue, green, primary_blue, red, yellow, dark_grey;
    inline sf::Color light_violet, light_green, light_blue, light_red, light_yellow, light_grey;
    inline sf::Color white, black;

    bool siEs( int valor ){
        return valor >= 0 && valor <= 255;
    }

    void cargar(const std::string& ruta) {
        std::ifstream file(ruta);
        std::map<std::string, sf::Color> m;
        std::string tag;
        int r, g, b, a;
        std::string posible;

        while( file >> tag  ){
            // TODO: hacer psoible agregar coemntarios
            if( tag.size() >= 1 && tag[0] == '/') {
                std::string basura;
                std::getline(file, basura); // Consume el resto de la línea y la tira
                continue;
            }
            
            // si no hay nada mas por leer
            if (!(file >> posible)) break; 

            // --- un color ya usado ---
            if(  std::isalpha( static_cast<unsigned char>(posible[0]) )  ){
                if( m.count(posible) ){
                    m[tag] = m[posible];
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
                         m[tag] = sf::Color(r,g,b,a);
                    }else{
                        std::cerr << "ERROR: Valores fuera de rango (0-255) en " << tag << "\n";
                    }
                }catch (...){
                    std::cerr << "ERROR: Formato numerico invalido en " << tag << "\n";
                    // mandar  la basura el resto por si en als demas lineas si hay algo
                    std::string basura;
                    std::getline(file, basura);
                }

            }
            
        } 

        // generales
        dark_blue    = m["dark_blue"];
        green        = m["green"];
        primary_blue = m["primary_blue"];
        red          = m["red"];
        yellow       = m["yellow"];
        dark_grey    = m["dark_grey"];
        light_violet = m["light_violet"];
        light_green  = m["light_green"];
        light_blue   = m["light_blue"];
        light_red    = m["light_red"];
        light_yellow = m["light_yellow"];
        light_grey   = m["light_grey"];
        white        = m["white"];
        black        = m["black"];
        // especificos
        Fondo = m["fondo"];
        Obreras = m["obreras"];
        Guerreras = m["guerreras"];
        Recolectoras = m["recolectoras"];
        Panel = m["panel"];
        Color1 = m["color1"];
        Color2 = m["color2"];
    }
}



 // --- configurar ventana ---
 namespace Sistema {
    // Estructura para devolver los parámetros configurados
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