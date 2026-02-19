/*  * autor: Angel Manuel Gonzalez Lopez 
    * github: https://github.com/angelmanuelgl
    * web: https://angelmanuelgl.github.io/
    * 
    * - - - - - - -  -  DynSysVis  - - - - - - - - - - 
    * Dynamical System Visualizer Real-Time
    * libreria de herramientas graficas para monitoreo de datos 
    * y comportamiento de sistemas complejos en tiempo Real.
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

namespace dsv {

namespace Color{
    const sf::Color rojo(255, 100, 130, 255);
    const sf::Color naranja(255, 160, 60, 255);
    const sf::Color amarillo(255, 235, 100, 255);
    const sf::Color verde(100, 255, 170, 255);
    const sf::Color aqua(80, 230, 210, 255);     // Nuevo
    const sf::Color celeste(130, 245, 255, 255);
    const sf::Color cian(100, 210, 255, 255);
    const sf::Color azul(110, 170, 255, 255);
    const sf::Color violeta(160, 110, 255, 255);
    const sf::Color morado(190, 130, 255, 255);
    const sf::Color rosa(255, 80, 140, 255);
    const sf::Color marron(160, 110, 90, 255);    // Nuevo

    // --- Acento / Claros (Desaturados/Pastel) ---
    const sf::Color rojo_l(214, 122, 131, 255);
    const sf::Color naranja_l(235, 165, 145, 255);
    const sf::Color amarillo_l(255, 225, 160, 255);
    const sf::Color verde_l(185, 212, 160, 255);
    const sf::Color aqua_l(160, 210, 200, 255);
    const sf::Color cian_l(155, 210, 225, 255);
    const sf::Color azul_l(150, 180, 210, 255);
    const sf::Color morado_l(205, 165, 195, 255);
    const sf::Color rosa_l(255, 150, 180, 255);   // Ajustado para ser más claro
    const sf::Color marron_l(190, 155, 140, 255);

    // --- Escala de Grises y Neutros ---
    const sf::Color blanco(255, 255, 255, 255);
    const sf::Color gris_claro(167, 169, 171, 255); // grey_40
    const sf::Color gris(135, 137, 140, 255);       // grey_56
    const sf::Color gris_oscuro(105, 106, 108, 255); // grey_72
    const sf::Color fondo_panel(69, 69, 71, 255);    // dark_grey
    const sf::Color fondo_oscuro(30, 31, 34, 255);   // Tono típico de dashboard
    const sf::Color negro(0, 0, 0, 255);

    // --- Variantes Extra para Gráficos (Muted Colors) ---
    const sf::Color verde_bosque(75, 140, 100, 255);
    const sf::Color azul_noche(60, 80, 120, 255);
    const sf::Color terracota(180, 90, 80, 255);
    const sf::Color oro(215, 170, 70, 255);
    
    // ---------------------------------------------------
    //  Arcoiris
    inline sf::Color Arcoiris(int i, int n) {
        float f = 0.3f; // frecuencia
        return sf::Color(
            std::sin(f * i + 0) * 127 + 128,
            std::sin(f * i + 2) * 127 + 128,
            std::sin(f * i + 4) * 127 + 128
        );
    }

    // Cyberpunk: Magenta Neon -> Cyan Electrico
    inline sf::Color Cyberpunk(int i, int n) {
        float ratio = static_cast<float>(i) / (n - 1);
        return sf::Color(
            255 * (1.0f - ratio), // El rojo baja
            200 * ratio,          // El verde sube
            255                   // El azul siempre a tope
        );
    }

    // Fuego e Hielo: De Rojo ->  Azul 
    inline sf::Color FuegoHielo(int i, int n) {
        float ratio = static_cast<float>(i) / (n - 1);
        return sf::Color(
            255 * (1.0f - ratio), // Rojo en i=0
            50,                    // Un toque de verde para suavizar
            255 * ratio           // Azul en i=n
        );
    }
    



    // ---------------------------------------------------
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

}
#endif