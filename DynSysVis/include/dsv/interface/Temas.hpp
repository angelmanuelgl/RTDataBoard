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
    -> sirvira para gestionar colores generales, 
    -> poder elegir en assets/config/colores.txt una paleta separado del ejecutable 
    -> agregar nombres de colroes de acceso rapido dsv::Color::terracota
    -> funciones que devulven un degradado dado la posicon i y el largo totoal (int i, int n)
    -> //todo interpolar entre dos colores facilemnte 
*/
#ifndef TEMAS_HPP
#define TEMAS_HPP


// std y sfl
#include <SFML/Graphics.hpp>
#include <cmath>
#include <algorithm>
#include <fstream>
#include <iostream>

// dsv
#include "dsv/core/Logger.hpp"



namespace dsv {


namespace Color{
    // --- ROJOS Y VINOS ---
    // --- ROJOS, VINOS Y ROSAS ---
    const sf::Color rojo(255, 82, 82);
    const sf::Color rojo_l(255, 150, 150);
    const sf::Color rojo_d(180, 40, 40);
    const sf::Color rojo_dd(35, 20, 20);

    const sf::Color vino(140, 20, 50);
    const sf::Color vino_l(190, 80, 100);
    const sf::Color vino_d(80, 10, 30);
    const sf::Color vino_dd(25, 15, 18);

    const sf::Color rosa(255, 120, 180);
    const sf::Color rosa_l(255, 180, 210);
    const sf::Color rosa_d(200, 80, 130);
    const sf::Color rosa_dd(35, 20, 28);

    const sf::Color rosaMexicano(230, 0, 125);
    const sf::Color rosaMexicano_l(255, 100, 180);
    const sf::Color rosaMexicano_d(160, 0, 90);
    const sf::Color rosaMexicano_dd(30, 15, 22);

    const sf::Color magenta(220, 50, 220);
    const sf::Color magenta_l(240, 150, 240);
    const sf::Color magenta_d(150, 30, 150);
    const sf::Color magenta_dd(28, 18, 28);

    // --- NARANJAS, AMARILLOS Y OROS ---
    const sf::Color naranja(255, 150, 50);
    const sf::Color naranja_l(255, 200, 130);
    const sf::Color naranja_d(200, 100, 20);
    const sf::Color naranja_dd(35, 25, 15);

    const sf::Color amarillo(255, 220, 60);
    const sf::Color amarillo_l(255, 240, 150);
    const sf::Color amarillo_d(210, 170, 20);
    const sf::Color amarillo_dd(32, 30, 15);

    const sf::Color oro(215, 170, 70);
    const sf::Color oro_l(240, 210, 130);
    const sf::Color oro_d(160, 120, 40);
    const sf::Color oro_dd(28, 25, 18);

    const sf::Color ocre(190, 130, 50);
    const sf::Color ocre_l(220, 170, 100);
    const sf::Color ocre_d(140, 90, 30);
    const sf::Color ocre_dd(25, 22, 15);

    // --- VERDES Y AQUA ---
    const sf::Color verde(100, 255, 170);
    const sf::Color verde_l(180, 255, 210);
    const sf::Color verde_d(60, 190, 120);
    const sf::Color verde_dd(20, 40, 30);

    const sf::Color verdeLimon(180, 255, 50);
    const sf::Color verdeLimon_l(220, 255, 140);
    const sf::Color verdeLimon_d(130, 200, 20);
    const sf::Color verdeLimon_dd(25, 32, 18);

    const sf::Color aqua(80, 230, 210);
    const sf::Color aqua_l(170, 245, 235);
    const sf::Color aqua_d(50, 170, 155);
    const sf::Color aqua_dd(15, 35, 33);

    // --- AZULES Y CIAN ---
    const sf::Color cian(50, 230, 255);
    const sf::Color cian_l(160, 245, 255);
    const sf::Color cian_d(30, 160, 190);
    const sf::Color cian_dd(15, 30, 35);

    const sf::Color celeste(130, 245, 255);
    const sf::Color celeste_l(200, 250, 255);
    const sf::Color celeste_d(80, 190, 220);
    const sf::Color celeste_dd(20, 35, 40);

    const sf::Color azul(70, 130, 255);
    const sf::Color azul_l(160, 190, 255);
    const sf::Color azul_d(40, 80, 200);
    const sf::Color azul_dd(18, 22, 35);

    const sf::Color azulMar(30, 80, 160);
    const sf::Color azulMar_l(100, 140, 210);
    const sf::Color azulMar_d(15, 50, 110);
    const sf::Color azulMar_dd(12, 18, 28);

    // --- VIOLETAS Y MORADOS ---
    const sf::Color violeta(160, 110, 255);
    const sf::Color violeta_l(200, 170, 255);
    const sf::Color violeta_d(110, 60, 200);
    const sf::Color violeta_dd(25, 20, 35);

    const sf::Color morado(120, 60, 230);
    const sf::Color morado_l(170, 130, 255);
    const sf::Color morado_d(80, 30, 160);
    const sf::Color morado_dd(20, 15, 28);

    // --- TIERRAS Y METALES ---
    const sf::Color marron(110, 70, 50);
    const sf::Color marron_l(160, 120, 100);
    const sf::Color marron_d(70, 40, 30);
    const sf::Color marron_dd(22, 18, 15);

    const sf::Color cafe(80, 50, 40);
    const sf::Color cafe_l(130, 95, 80);
    const sf::Color cafe_d(50, 30, 25);
    const sf::Color cafe_dd(18, 14, 12);

    const sf::Color cobre(185, 100, 60);
    const sf::Color cobre_l(220, 150, 120);
    const sf::Color cobre_d(130, 60, 30);
    const sf::Color cobre_dd(28, 20, 15);

    const sf::Color plata(170, 175, 185);
    const sf::Color plata_l(210, 215, 225);
    const sf::Color plata_d(120, 125, 135);
    const sf::Color plata_dd(40, 42, 45);

    // --- ESCALA DE GRISES ---
    const sf::Color blanco(245, 245, 245);
    const sf::Color blanco_l(255, 255, 255);
    const sf::Color blanco_d(200, 200, 200);
    const sf::Color blanco_dd(60, 60, 65);

    const sf::Color gris(130, 135, 145);
    const sf::Color gris_l(180, 185, 195);
    const sf::Color gris_d(80, 85, 95);
    const sf::Color gris_dd(35, 37, 40);

    const sf::Color negro(20, 20, 25);
    const sf::Color negro_l(50, 50, 60);
    const sf::Color negro_d(10, 10, 12);
    const sf::Color negro_dd(0, 0, 0);

    const sf::Color gris_claro(167, 169, 171, 255); // grey_40
    // const sf::Color gris(135, 137, 140, 255);       // grey_56
    const sf::Color gris_oscuro(105, 106, 108, 255); // grey_72



    // --- Variantes Extra para Gráficos (Muted Colors) ---
    const sf::Color verde_bosque(75, 140, 100, 255);
    const sf::Color azul_noche(60, 80, 120, 255);
    const sf::Color terracota(180, 90, 80, 255);


    // --- paneles ---
    const sf::Color fondo_panel(69, 69, 71, 255);    // dark_grey
    const sf::Color panelUp(40,40,50);
    const sf::Color panelDown(20,20,25); 

    

    // --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- ---
    // --- --- --- --- ---  DEGRADADOS POR INDICE  --- --- --- --- ---
    // --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- ---

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
    
    // morado oscuro -> Verde -> Amarillo brillante. 
    inline sf::Color Viridis(int i, int n) {
        float r = static_cast<float>(i) / (n - 1);
        // Aproximación de los nodos de Viridis
        return sf::Color(
            static_cast<sf::Uint8>(253 * r * r + 68 * (1-r)), // R
            static_cast<sf::Uint8>(231 * r + 12 * (1-r)),     // G
            static_cast<sf::Uint8>(37 + 150 * r * (1-r))      // B
        );
    }
    // Negro/Morado -> Rojo/Naranja -> Blanco/Amarillo
    inline sf::Color Magma(int i, int n) {
        float r = static_cast<float>(i) / (n - 1);
        return sf::Color(
            static_cast<sf::Uint8>(255 * std::pow(r, 0.4f)), // R sube rápido
            static_cast<sf::Uint8>(200 * std::pow(r, 1.5f)), // G sube lento
            static_cast<sf::Uint8>(150 * std::pow(r, 3.0f) + 50 * (1-r)) // B
        );
    }
    //l Violeta -> Magenta -> Naranja.
    inline sf::Color Plasma(int i, int n) {
        float r = static_cast<float>(i) / (n - 1);
        return sf::Color(
            static_cast<sf::Uint8>(237 * std::pow(r, 0.7f) + 13 * (1-r)),
            static_cast<sf::Uint8>(252 * std::pow(r, 2.0f) + 30 * (1-r)),
            static_cast<sf::Uint8>(255 * std::sin(r * 1.5f) + 100 * (1-r))
        );
    }
    //  Verde oscuro -> Esmeralda -> Lima.
    inline sf::Color Bosque(int i, int n) {
        float r = static_cast<float>(i) / (n - 1);
        return sf::Color(
            30 + 150 * r,  // Rojo bajo
            80 + 175 * r,  // Verde dominante
            50             // Azul fijo
        );
    }
    // Azul marino -> Turquesa -> Blanco espuma.
    inline sf::Color Oceano(int i, int n){
        float r = static_cast<float>(i) / (n - 1);
        return sf::Color(
            static_cast<sf::Uint8>(255 * std::pow(r, 2.0f)), // Solo al final blanquea
            static_cast<sf::Uint8>(100 + 155 * r),
            static_cast<sf::Uint8>(150 + 105 * r)
        );
    }

    inline int interpolaCol( int a, int b, float t ){
        t = std::max(0.0f, std::min(1.0f, t));
        int ans = a + (b-a) * t;

        if( ans > 255) return 225;
        if( ans < 0 ) return 0;
        return ans;
    }

    
    // --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- ---
    // --- --- --- --- ---  DEGRADADOS POR PALETA  --- --- --- --- ---
    // --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- ---

    // --- Generador Universal de Paletas ---
    template<typename Func>
    inline std::vector<sf::Color> generarPaleta(Func funcionColor, int pasos = 10) {
        std::vector<sf::Color> paleta;
        paleta.reserve(pasos);
        for( int i = 0; i < pasos; ++i){
            paleta.push_back(funcionColor(i, pasos));
        }
        return paleta;
    }

    // --- Versiones de Paleta Completa (con Lambdas) ---
    inline std::vector<sf::Color> FuegoHielo() { 
        return generarPaleta([](int i, int n) { return FuegoHielo(i, n); }); 
    }
    inline std::vector<sf::Color> Cyberpunk()  { 
        return generarPaleta([](int i, int n) { return Cyberpunk(i, n); }); 
    }
    inline std::vector<sf::Color> Arcoiris()   { 
        return generarPaleta([](int i, int n) { return Arcoiris(i, n); }); 
    }
    inline std::vector<sf::Color> Viridis()    { 
        return generarPaleta([](int i, int n) { return Viridis(i, n); }); 
    }
    inline std::vector<sf::Color> Magma()      { 
        return generarPaleta([](int i, int n) { return Magma(i, n); }); 
    }
    inline std::vector<sf::Color> Plasma()     { 
        return generarPaleta([](int i, int n) { return Plasma(i, n); }); 
    }
    inline std::vector<sf::Color> Bosque()     { 
        return generarPaleta([](int i, int n) { return Bosque(i, n); }); 
    }
    inline std::vector<sf::Color> Oceano()     { 
        return generarPaleta([](int i, int n) { return Oceano(i, n); }); 
    }


    // --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- ---
    // --- --- --- --- ---  DEGRADADOS utileria  --- --- --- --- ---
    // --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- ---
    inline sf::Color interpolarColores(sf::Color a, sf::Color b, float t ){
    return sf::Color(
        static_cast<sf::Uint8>( interpolaCol( a.r, b.r, t ) ),
        static_cast<sf::Uint8>( interpolaCol( a.g, b.g, t ) ),
        static_cast<sf::Uint8>( interpolaCol( a.b, b.b, t ) ),
        static_cast<sf::Uint8>( interpolaCol( a.a, b.a, t ) )
    );
    }
    inline sf::Color obtenerColorDegradado(const std::vector<sf::Color>& paleta, float t) {
        if(paleta.empty()) return sf::Color::Magenta;
        if( paleta.size() == 1) return paleta[0];
        if( t <= 0.0f ) return paleta.front();
        if( t >= 1.0f ) return paleta.back();

        // ver en que segmento estamos
        float fraccion = t * (paleta.size() - 1);
        size_t indice = static_cast<size_t>(fraccion);
        // el sobrante para interpolar entre indice e indice+1
        float tLocal = fraccion - indice; 
        return interpolarColores(paleta[indice], paleta[indice + 1], tLocal);
    }


    
    // --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- ---
    // --- --- --- --- ---  LOL LOL LOL LOL  --- --- --- --- ---
    // --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- ---
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
    inline bool siEs( int valor ){
        return valor >= 0 && valor <= 255;
    }

    inline void cargar(const std::string& ruta) {
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

} // end Color
} // end dsv


inline sf::Color operator%(sf::Color color, int alpha) {
    color.a = static_cast<uint8_t>(alpha);
    return color;
}


 // --- configurar ventana ---
 namespace dsv {
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
                      sf::Style::Default, //  Default // Fullscreen
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