#include <SFML/Graphics.hpp>
#include <cmath>
#include <vector>
#include "DynSysVis.hpp"

// Estructura para manejar cada péndulo individual
struct EstadoPendulo {
    float theta;
    float omega;
    std::string id;
    sf::Color color;
};

int main() {
    dsv::Color::cargar("assets/config/colores.txt");
    sf::RenderWindow window;
    dsv::Sistema::inicializarVentana(window, "DynSysVis RT - Retrato de Fase Multiserie");

    // Layout: Fase a la izquierda (grande) y una gráfica de tiempo a la derecha
    dsv::Layout miLayout = {
        "FASE FASE FASE .",
        "FASE FASE FASE .",
        "FASE FASE FASE .",
        "T T T T"
    };
    dsv::Tablero tablero(window, miLayout);

    // Paneles
    auto gFase = tablero.add<dsv::GraficaEspacioFase>("Retrato de Fase (20 trayectorias)", dsv::Color::azul, "FASE");
    auto gTiempo = tablero.add<dsv::GraficaTiempo>("Velocidades Angulares", dsv::Color::violeta, "T");

    
    gFase -> configurarLimites(-3.2, 3.2, -5.5, 5.5, false );
    gTiempo -> configurarLimites(0, 0, -6, 6 , true );
    gTiempo ->configurarMaxPoints (1000 );

    // --- Configuracion de las 20 series ---
    std::vector<EstadoPendulo> simulaciones;
    const int numSeries = 20;
    
    for( int i = 0; i < numSeries; i++){
        // Generamos condiciones iniciales variadas (ángulos de -3 a 3)
        float thetaInicial = -3.0f + (i * 6.0f / (numSeries - 1));
        std::string id = "P" + std::to_string(i);
        
        // sf::Color col = dsv::Color::Arcoiris(i, numSeries); 
        sf::Color col = dsv::Color::Cyberpunk(i, numSeries);
        // sf::Color col = dsv::Color::FuegoHielo(i, numSeries);
        simulaciones.push_back({thetaInicial, 0.0f, id, col});
        
        sf::Color col2 = col;
        col2.a = 70;
        // Registrar la serie en los paneles
        gFase->agregarSerie(id, col);
        gTiempo->agregarSerie(id, col2);
    }

    // parametros
    const float g = 7.81f;
    const float L = 1.0f; 
    const float amortiguamiento = 0.997f;

    sf::Clock clock;
    sf::Time accumulator = sf::Time::Zero;
    sf::Time ups = sf::seconds(0.016f);
    bool iniciado = false;

    while( window.isOpen() ){
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window.close();
            if (event.type == sf::Event::KeyPressed){
                clock.restart(); 
                accumulator = sf::Time::Zero;
                iniciado = true;
            }
        }

        accumulator += clock.restart();
        while( accumulator >= ups && iniciado ){
            float dt = ups.asSeconds();

            // Actualizar cada una de las 20 simulaciones
            for( auto& p : simulaciones){
                float aceleracion = -(g / L) * std::sin(p.theta);
                p.omega += aceleracion * dt;
                p.omega *= amortiguamiento;
                p.theta += p.omega * dt;

                // Enviar datos a sus series correspondientes usando el ID
                gFase->push_back(p.theta, p.omega, p.id);
                gTiempo->push_back(p.omega, p.id);
            }
            accumulator -= ups;
        }

        tablero.draw();
        window.display();
    }
    return 0;
}