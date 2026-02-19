/*  * autor: Angel Manuel Gonzalez Lopez 
    * github: https://github.com/angelmanuelgl
    * web: https://angelmanuelgl.github.io/ 
    
    * LIBRERIA DynSysVis RT
*/
/*  MAIN.cpp
    ejemplo para usar mi libreria InsightRT
    
    APLICACION: pendulo simple
    ECUACION DIFERENCIABLE DE SEGUNDO ORDEN
    
    ejemplo SIN USAR dsv::Tablero
    ventajas: mayor personalziacion
    desventajas: tienes punteros (no afecta en nada pero visualmente estresa), mas linea de codigo
        te encatgas de los draw y de crear panel y contenido
*/

#include <SFML/Graphics.hpp>
#include <cmath>
#include <vector>
#include <algorithm>

#include "DynSysVis.hpp"

using namespace dsv;
int main( ){
 
    // --- cargar colores --- SFML
    Color::cargar("assets/config/colores.txt");

    // --- configurar ventana --- DVS
    sf::RenderWindow window;
    Sistema::inicializarVentana(window, "Simulación de Hormigas - Tesis");


    // --- Parametros del Pendulo ---
    float theta = 1.5f;      // angulo inicial (aprox 85 grados)
    float omega = 0.0f;      // velocidad angular inicial
    const float g = 9.81f;   // gravedad
    const float L = 200.0f;  // longitud del pendulo (en pixeles para visualizacion)
    const float amortiguamiento = 0.999f; // opcional: para que se detenga poco a poco

    // --- paneles --- DSV
    Panel panelTheta(window,  "Velocidad Omega(t)"); // parametros por defecto 
    panelTheta.positionAbsoluta(Ubicacion::ArribaDer);
    
    Panel panelOmega(window,  "Velocidad Omega(t)", Color::c("naranja")); // parametros por defecto 
    panelOmega.positionRelativa(RelativoA::Abajo, panelTheta);
    
    Panel panelFase(window,  "Velocidad Omega(t)", Color::c("violeta"), 2,2 ); // cuarto apraemtro color de fondo
    panelFase.positionAbsoluta(Ubicacion::ArribaIzq);
    
    // --- graficas y contenido en general ---
    auto* ptrTheta = panelTheta.crearContenido<GraficaTiempo>(Color::c("rojo"));
    auto* ptrOmega = panelOmega.crearContenido<GraficaTiempo>(Color::c("naranja"));
    auto* ptrFase  = panelFase.crearContenido<GraficaEspacioFase>(Color::c("violeta"));


    // --- acceder a metodos de los objetos -- DSV ( GraficaTiempo,GraficaEspacioFase , etc )
    ptrOmega -> configurarLimites( 0,0, -3, 3, true);
    ptrFase -> configurarLimites(-3.2, 3.2, -2, 2 );


    // --- Control del tiempo ---
    sf::Clock clock;
    sf::Time accumulator = sf::Time::Zero;
    sf::Time ups = sf::seconds(0.016f); // Update por segundo

    while( window.isOpen() ){
        sf::Event event;
        while( window.pollEvent(event) ){
            if( event.type == sf::Event::Closed) window.close();
        }

        accumulator += clock.restart();
        while( accumulator >= ups ){
            float dt = ups.asSeconds();

            // --- pendulo (Euler-Cromer para mejor estabilidad) ---
            float aceleracion = -(g / (L / 100.0f)) * std::sin(theta); // L/100 para escalar metros
            omega += aceleracion * dt;
            omega *= amortiguamiento; // Descomentar para friccion
            theta += omega * dt;

            // --- actualizar graficas ---
            ptrOmega->push_back(omega);
            ptrTheta->push_back(theta);
            ptrFase->push_back(omega, theta);

            accumulator -= ups;
        }

        // --- RENDERIZADO --- DSV
        window.clear(sf::Color(40, 40, 40));

        panelTheta.draw();
        panelOmega.draw();
        panelFase.draw();

        window.display();
    }
    return 0;
}