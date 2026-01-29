/*  * autor: Angel Manuel Gonzalez Lopez 
    * github: https://github.com/angelmanuelgl
    * web: https://angelmanuelgl.github.io/
    * 
    * proyecto: InsightRT - - - - - - - - - - - - - - - - - - - 
    * libreria de herramientas graficas para monitoreo de datos 
    * en vivo y comportamiento de sistemas complejos.
 */
/*  MAIN.cpp
    ejemplo para usar mi libreria InsightRT
    APLICACION: sistema dinamicos de hormigas
*/
#include <SFML/Graphics.hpp>
#include <cmath>
#include <vector>
#include <algorithm>

#include "Geometria.hpp"
#include "Graficas.hpp"
#include "Temas.hpp"

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
    const float L = 200.0f;  // longitud del pendulo (en pixeles para visualización)
    const float amortiguamiento = 3.995f; // opcional: para que se detenga poco a poco

    // --- Interfaz InsightRT ---
    float espaciado = 25.f;
    float margenVentana = 20.f;

    // Panel para el angulo Theta(t)
    Panel panelTheta({350, 200}, 20, sf::Color(25, 25, 25), Tema::yellow);
    panelTheta.positionAbsoluta(Ubicacion::CentroDer, window, margenVentana);
    GraficaTiempo graphTheta(Tema::yellow, "Angulo Theta(t)");

    // Panel para velocidad angular omega(t)
    Panel panelOmega({350, 200}, 20, sf::Color(25, 25, 25), Tema::yellow);
    panelOmega.positionRelativa(RelativoA::Arriba, panelTheta, espaciado);
    GraficaTiempo graphOmega(Tema::yellow, "Velocidad Omega(t)");

    // panel para el Espacio de Fase (Theta vs Omega)
    Panel panelFase({350, 200}, 20, sf::Color(25, 25, 25), Tema::green);
    panelFase.positionRelativa(RelativoA::Abajo, panelTheta, espaciado);
    GraficaEspacioFase graphFase(Tema::green, "Fase (Theta, Omega)");

    // --- Control del tiempo ---
    sf::Clock clock;
    sf::Time accumulator = sf::Time::Zero;
    sf::Time ups = sf::seconds(0.016f); // ~60 updates por segundo para suavidad
    // sf::Time ups = sf::seconds(0.1f); // Update por segundo

    while( window.isOpen() ){
        sf::Event event;
        while( window.pollEvent(event) ){
            if (event.type == sf::Event::Closed) window.close();
        }

        accumulator += clock.restart();
        while( accumulator >= ups ){
            float dt = ups.asSeconds();

            // --- pendulo (Euler-Cromer para mejor estabilidad) ---
            float aceleracion = -(g / (L / 100.0f)) * std::sin(theta); // L/100 para escalar metros
            omega += aceleracion * dt;
            // omega *= amortiguamiento; // Descomenta para friccion
            theta += omega * dt;

            // actualizar graficas
            graphTheta.addValue(theta);
            graphOmega.addValue(omega);
            graphFase.addValue(theta, omega);

            accumulator -= ups;
        }

        // --- RENDERIZADO ---
        window.clear(sf::Color(10, 10, 10));

        // dibujar el pendulo fisicamente
        sf::Vector2f origen(window.getSize().x / 4.0f, window.getSize().y / 2.0f);
        sf::Vector2f extremo(origen.x + L * std::sin(theta), origen.y + L * std::cos(theta));

        sf::Vertex linea[] = { sf::Vertex(origen), sf::Vertex(extremo, sf::Color::White) };
        window.draw(linea, 2, sf::Lines);

        sf::CircleShape masa(15.f);
        masa.setFillColor(sf::Color::Cyan);
        masa.setOrigin(15.f, 15.f);
        masa.setPosition(extremo);
        window.draw(masa);

        // paneles y graficas
        panelTheta.draw(window);
        graphTheta.draw(window, panelTheta);

        panelFase.draw(window);
        graphFase.draw(window, panelFase);

        
        panelOmega.draw(window);
        graphOmega.draw(window, panelOmega);


        window.display();
    }
    return 0;
}