/*  * autor: Angel Manuel Gonzalez Lopez 
    * github: https://github.com/angelmanuelgl
    * web: https://angelmanuelgl.github.io/ 
    
    * LIBRERIA DynSysVis RT
*/
/*  MAIN.cpp
    ejemplo para usar mi libreria
    
    APLICACION: pendulo simple
    ECUACION DIFERENCIABLE DE SEGUNDO ORDEN 

    ejemplo usando dsv::Tablero
    ventajas: menor codigo, al declarar a la vez panel y contenido
        ya no te encargas de los draw
*/

#include <SFML/Graphics.hpp>
#include <cmath>
#include "DynSysVis.hpp"


// using namespace dsv;
int main( ){
 
    // --- cargar colores --- DSV
    dsv::Color::cargar("assets/config/colores.txt");

    // --- configurar ventana --- SFML
    sf::RenderWindow window;
    dsv::Sistema::inicializarVentana(window, "DynSysVis RT - Péndulo Simple");

    // --- tablero con datos --- DSV
    dsv::Layout miLayout = {
        "f  .",
        "f  o",
        "t t",
    };
    dsv::Tablero tablero(window, miLayout);


    // --- parametros del Pendulo ---
    float theta = 1.5f;      // angulo inicial (aprox 85 grados)
    float omega = 0.0f;      // velocidad angular inicial
    const float g = 9.81f;   // gravedad
    const float L = 200.0f;  // longitud del pendulo (en pixeles para visualizacion)
    const float amortiguamiento = 0.995f; // opcional: para que se detenga poco a poco

    // --- paneles ---  DSV
    auto gTheta = tablero.add<dsv::GraficaTiempo>("Angulo Theta(t)", dsv::Color::c("rojo"), "t", dsv::Color::c("rojo"));
    auto gOmega = tablero.add<dsv::GraficaTiempo>("Velocidad Omega(t)", dsv::Color::c("naranja"), "o",  dsv::Color::c("naranja"));
    auto gFase  = tablero.add<dsv::GraficaEspacioFase>("Espacio de Fase", dsv::Color::c("violeta"), "f", dsv::Color::c("violeta"));

    // --- acceder a metodos de los objetos -- DSV ( GraficaTiempo,GraficaEspacioFase , etc )
    gOmega->configurarLimites(0, 0, -3, 3, true);
    gTheta->configurarLimites(0, 0, -2, 2, true);
    gFase->configurarLimites(-3.2, 3.2, -2, 2);


    // --- Control del tiempo ---
    sf::Clock clock;
    sf::Time accumulator = sf::Time::Zero;
    sf::Time ups = sf::seconds(0.016f); //  Update por segundo

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
            gOmega->push_back(omega);
            gTheta->push_back(theta);
            gFase->push_back(omega, theta);

            accumulator -= ups;
        }

        // --- RENDERIZADO --- DSV
        tablero.draw();
        window.display();
    }
    return 0;
}