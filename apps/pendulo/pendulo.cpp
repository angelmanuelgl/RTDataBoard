/*  * autor: Angel Manuel Gonzalez Lopez 
    * github: https://github.com/angelmanuelgl
    * web: https://angelmanuelgl.github.io/
    * 
    * proyecto: InsightRT - - - - - - - - - - - - - - - - - - - 
    * libreria de herramientas graficas para monitoreo de datos 
    * en en tiempo real y comportamiento de sistemas complejos.
 */
/*  MAIN.cpp
    ejemplo para usar mi libreria InsightRT
    
    APLICACION: pendulo simple
                ECUACION DIFERENCIABLE DE SEGUNDO ORDEN 
*/

#include <SFML/Graphics.hpp>
#include <cmath>
#include <vector>
#include <algorithm>

#include "DynSysVis.hpp"

int main( ){
 
    // --- cargar colores ---
    Tema::cargar("assets/config/colores.txt");

    // --- configurar ventana ---
    sf::RenderWindow window;
    Sistema::inicializarVentana(window, "Simulación de Hormigas - Tesis");


    // --- Parametros del Pendulo ---
    float theta = 1.5f;      // angulo inicial (aprox 85 grados)
    float omega = 0.0f;      // velocidad angular inicial
    const float g = 9.81f;   // gravedad
    const float L = 200.0f;  // longitud del pendulo (en pixeles para visualizacion)
    const float amortiguamiento = 0.999f; // opcional: para que se detenga poco a poco

    // --- paneles --- 
    Panel panelTheta(window, Tema::c("rojo"), "Angulo Theta(t)", 2,4 );
    panelTheta.positionAbsoluta(Ubicacion::ArribaDer);
    
    Panel panelOmega(window, Tema::c("naranja"), "Velocidad Omega(t)", 2,4 );
    panelOmega.positionRelativa(RelativoA::Abajo, panelTheta);
    
    Panel panelFase(window, Tema::c("violeta"), "Fase (omega , theta)", 2,2 );
    panelFase.positionAbsoluta(Ubicacion::ArribaIzq);
    
    // --- graficas y contenido en general ---
    auto* ptrTheta = panelTheta.crearContenido<GraficaTiempo>(Tema::c("rojo"));
    auto* ptrOmega = panelOmega.crearContenido<GraficaTiempo>(Tema::c("naranja"));
    auto* ptrFase  = panelFase.crearContenido<GraficaEspacioFase>(Tema::c("violeta"));


    // --- configurar limites -- 
    // (opcional pero se ve mejor)
    // ptrTheta -> ponerSombreado( true, false);
    // graphTheta -> configurarLimites(-1,1, -1, 1);
    ptrOmega -> configurarLimites( 0,0, -3, 3, true);
    ptrFase -> configurarLimites(-3.2, 3.2, -2, 2 );


    // --- Control del tiempo ---
    sf::Clock clock;
    sf::Time accumulator = sf::Time::Zero;
    sf::Time ups = sf::seconds(0.016f); // ~60 updates por segundo para suavidad
    // sf::Time ups = sf::seconds(0.1f); // Update por segundo

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
            ptrOmega->addValue(omega);
            ptrTheta->addValue(theta);
            ptrFase->addValue(omega, theta);

            accumulator -= ups;
        }

        // --- RENDERIZADO ---
        window.clear(sf::Color(40, 40, 40));

        // // dibujar el pendulo fisicamente
        // sf::Vector2f origen(window.getSize().x / 4.0f, window.getSize().y / 2.0f);
        // sf::Vector2f extremo(origen.x + L * std::sin(theta), origen.y + L * std::cos(theta));

        // sf::Vertex linea[] = { sf::Vertex(origen), sf::Vertex(extremo, sf::Color::White) };
        // window.draw(linea, 2, sf::Lines);

        // sf::CircleShape masa(15.f);
        // masa.setFillColor(sf::Color::Cyan);
        // masa.setOrigin(15.f, 15.f);
        // masa.setPosition(extremo);
        // window.draw(masa);

        // --- paneles y graficas y marco ---
        // Cada panel dibuja: su marco , su contenido interno
        panelTheta.draw();
        panelOmega.draw();
        panelFase.draw();


        window.display();
    }
    return 0;
}