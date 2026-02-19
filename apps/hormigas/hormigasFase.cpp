#include <SFML/Graphics.hpp>
#include <cmath>
#include <vector>
#include <string>
#include "DynSysVis.hpp"
#include <random> // generar puntos aleatoriamente
using namespace dsv;

struct Colonia {
    float O, G, R;
    std::string id;
    sf::Color color;

    void actualizar(float dt ){
        // Constantes del modelo
        const float bO = 0.05f, betaOG = 4.0f, betaGR = 2.5f;
        const float betaRG = 1.2f, kG = 50.0f, kR = 50.0f, dG = 0.005f, dR = 0.005f;

        float dO = (bO * O) - (betaOG / (kG + G)) * O;
        float dG_dt = (betaOG / (kG + G)) * O - (betaGR / (kR + R)) * G - (dG * G) + (betaRG / (kG + G)) * R;
        float dR_dt = (betaGR / (kR + R)) * G - (betaRG / (kG + G)) * R - (dR * R);

        O += dO * dt;
        G += dG_dt * dt;
        R += dR_dt * dt;
    }
};

int main( ){
    dsv::Color::cargar("assets/config/colores.txt");
    sf::RenderWindow window;
    dsv::Sistema::inicializarVentana(window, "DynSysVis RT - Análisis de Poblaciones");

    // Layout según tu especificación
    dsv::Layout miLayout = {
        "f1 f2 f3",
        "f1 f2 f3",
        "f1 f2 f3",
        "f1 f2 f3",
        "g1 g2 g3 g4",
        "g5 g6 g7 g8 ",
    };
    dsv::Tablero tablero(window, miLayout);

    // 1. Paneles de Fase (Contendrán las 20 simulaciones cada uno)
    auto faseOG = tablero.add<dsv::GraficaEspacioFase>("Espacio Fase (O, G)", dsv::Color::azul, "f1");
    auto faseOR = tablero.add<dsv::GraficaEspacioFase>("Espacio Fase (O, R)", dsv::Color::azul, "f2");
    auto faseRG = tablero.add<dsv::GraficaEspacioFase>("Espacio Fase (R, G)", dsv::Color::azul, "f3");

    faseOG -> configurarLimites( 0, 400, 0 , 200, true);
    faseOR -> configurarLimites( 0, 400, 0 , 200, true);
    faseRG -> configurarLimites( 0, 100, 0 , 200, true);

    faseOG->configurarMaxPoints(2000);
    faseOR->configurarMaxPoints(2000);
    faseRG->configurarMaxPoints(2000);

    // Paneles Temporales 
    const int MAXP = 8;
    std::vector<  Vista<dsv::GraficaTiempo> > panelesT;
    for(int i = 1; i <= MAXP; ++i ){
        auto g = tablero.add<dsv::GraficaTiempo>("Simulacion " + std::to_string(i), dsv::Color::naranja, "g" + std::to_string(i));
        g->agregarSerie("Obreras",  sf::Color(50,200,50));
        g->agregarSerie("Guerreras", sf::Color(200,50,50));
        g->agregarSerie("Recolectoras",  sf::Color(150,150,50));
        g->configurarMaxPoints(3000);
        g.panel.setSizeTitulo(10);
        panelesT.push_back(g);
    }

    // --- Inicializar 20 Simulaciones ---
    const int numTotal = 40;
    std::vector<Colonia> colonias;
    
    // rangos aleatorios
    std::random_device rd;  
    std::mt19937 gen(rd()); 
    std::uniform_real_distribution<float> disG( 20.0f, 200.0f);
    std::uniform_real_distribution<float> disR( 20.0f, 200.0f);
    std::uniform_real_distribution<float> disO( 20.0f, 200.0f);


    for( int i = 0; i < numTotal; ++i ){
        float initO = disO(gen);
        float initG = disG(gen);
        float initR = disR(gen);

        std::string id = "C" + std::to_string(i);
        sf::Color col = dsv::Color::Cyberpunk(i, numTotal);

        colonias.push_back({initO, initG, initR, id, col});

        // agregar series con colores
        faseOG->agregarSerie(id, col);
        faseOR->agregarSerie(id, col);
        faseRG->agregarSerie(id, col);

    }

    sf::Clock clock;
    sf::Time accumulator = sf::Time::Zero;
    sf::Time ups = sf::seconds(0.01f);
    bool ini = false;
    while(window.isOpen() ){
        sf::Event event;
        while(window.pollEvent(event) ){
            if( event.type == sf::Event::Closed) window.close();
            if(event.type == sf::Event::KeyPressed){
                clock.restart(); 
                accumulator = sf::Time::Zero;
                ini = true;
            }
        }

        accumulator += clock.restart();
        while(accumulator >= ups && ini ){
            float dt = ups.asSeconds();

            for(int i = 0; i < numTotal; ++i ){
                colonias[i].actualizar(dt);

                // Todas las 20 van a los diagramas de fase
                faseOG->push_back(colonias[i].O, colonias[i].G, colonias[i].id);
                faseOR->push_back(colonias[i].O, colonias[i].R, colonias[i].id);
                faseRG->push_back(colonias[i].R, colonias[i].G, colonias[i].id);

                // Solo las primeras 6 van a los paneles individuales
                if( i < MAXP ){
                    panelesT[i]->push_back(colonias[i].O, "Obreras");
                    panelesT[i]->push_back(colonias[i].G, "Guerreras");
                    panelesT[i]->push_back(colonias[i].R, "Recolectoras");
                }
            }
            accumulator -= ups;
        }

        tablero.draw();
        window.display();
    }
    return 0;
}