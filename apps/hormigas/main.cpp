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
    sistema dinamicos de hormigas
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


    // --- poaraemtros para simular lo de hormigas ---

    // poblaciones iniciales
    float O = 100.0f; // Obreras
    float G = 20.0f;  // Guerreras
    float R = 10.0f;  // Recolectoras

    // transisciones y etc
    const float bO = 0.102f;     // natalidad obreras
    const float betaOG = 0.1f;   // transición O->G
    const float betaGR = 0.1f;   // transición G->R
    const float betaRG = 0.08f;  // transición R->G
    const float kG = 50.0f;      // saturacion Guerreras
    const float kR = 40.0f;      // saturacion Recolectoras
    const float dG = 0.02f;      // muerte Guerreras
    const float dR = 0.05f;      // muerte Recolectoras

    // ---  Interfaz ---
    float espaciado = 25.f; // epacio entre paneles
    float margenVentana = 20.f;

    // --- Guerreras G(t) ----
    Panel panelG({350, 200}, 20, Tema::Panel, Tema::Guerreras);
    panelG.positionAbsoluta(Ubicacion::CentroDer, window, margenVentana);
    GraficaTiempo graphG(Tema::Guerreras, "Poblacion de Guerreras G(t)");

    // --- Recolectoras R(t) ---
    Panel panelR({350, 200}, 20, Tema::Panel, Tema::Recolectoras);
    panelR.positionRelativa(RelativoA::Arriba, panelG, espaciado);
    GraficaTiempo graphR(Tema::Recolectoras, "Poblacion de Recolectoras R(t)");

    // --- Obreras O(t) ---
    Panel panelO({350, 200}, 20, Tema::Panel, Tema::Obreras);
    panelO.positionRelativa(RelativoA::Abajo, panelG, espaciado);
    GraficaTiempo graphO(Tema::Obreras, "Poblacion de Obreras O(t)");
    //graphO.ponerSobreado(false);

    // --- boceto de fase ---
    Panel nuevoPanel({350, 200}, 20, Tema::Panel, Tema::Color1);
    nuevoPanel.positionAbsoluta(Ubicacion::CentroIzq, window, margenVentana);
    GraficaEspacioFase nuevaGrafica(Tema::Color1, "(Obreras, Guerreras)");

    // --- boceto de fase ---
    Panel nuevoPanel2({350, 200}, 20, Tema::Panel, Tema::Color2);
    nuevoPanel2.positionRelativa(RelativoA::Arriba  , nuevoPanel, espaciado);
    GraficaEspacioFase nuevaGrafica2(Tema::Color2, "(Obreras, Recolectoras)");


    // --- IMPORTANTE: control del tiempo --
    //  (Paso fijo de 0.1s)
    sf::Clock clock;
    sf::Time accumulator = sf::Time::Zero;
    sf::Time ups = sf::seconds(0.1f); // Update por segundo

    while( window.isOpen() ){
        sf::Event event;
        while( window.pollEvent(event) ){
            if (event.type == sf::Event::Closed) window.close();
        }

        // --- simulacion ---
        accumulator += clock.restart();
        while( accumulator >= ups ){
            // --- Ec dif del MODELO NO LINEAL ---
            
            // dO/dt = bO*O - (betaOG / (kG + G))*O
            float dO = (bO * O) - (betaOG / (kG + G)) * O;   
            // dG/dt
            float dG_dt = (betaOG / (kG + G)) * O - (betaGR / (kR + R)) * G - (dG * G) + (betaRG / (kG + G)) * R;  
            // dR/dt
            float dR_dt = (betaGR / (kR + R)) * G - (betaRG / (kG + G)) * R - (dR * R);


            // actualiza variables con metodo de Euler
            float dt_val = ups.asSeconds();
            O += dO * dt_val;
            G += dG_dt * dt_val;
            R += dR_dt * dt_val;

            // agregar datos a graficas
            graphG.addValue(G);
            graphR.addValue(R);
            graphO.addValue(O);
            nuevaGrafica.addValue(O,G);
            nuevaGrafica2.addValue(O,R);

            accumulator -= ups;
        }

        // --- RENDERIZADO ---
        window.clear(sf::Color(15, 15, 15)); // Fondo oscuro tipo Sci-Fi

        // Dibujar paneles y sus graficas internas
        panelG.draw(window);
        graphG.draw(window, panelG);

        panelR.draw(window);
        graphR.draw(window, panelR);

        panelO.draw(window);
        graphO.draw(window, panelO);

        nuevoPanel.draw(window);
        nuevaGrafica.draw(window, nuevoPanel);

        nuevoPanel2.draw(window);
        nuevaGrafica2.draw(window, nuevoPanel2);

        window.display();
    }

    return 0;
}