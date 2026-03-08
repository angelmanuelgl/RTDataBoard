#include <SFML/Graphics.hpp>
#include <cmath>
#include "DynSysVis.hpp"

struct EntidadSIR {
    dsv::mod::SIR_Instance sim; // Los datos puros de simulación
    std::string id;             // Metadato visual
    sf::Color color;            // Metadato visual
};

int main() {
    dsv::Color::cargar("assets/config/colores.txt");
    sf::RenderWindow window;
    dsv::Sistema::inicializarVentana(window, "DynSysVis RT - 20 Simulaciones SIR");

    // dsv::Layout miLayout = {
    //     "3d 3d 3d f1",
    //     "3d 3d 3d f2",
    //     "3d 3d 3d f3",
    //     "t1 t1 t1 t1"
    // };
    dsv::Layout miLayout = {
        "3d 3d f1",
        "3d 3d f2",
        "t1 t1 f3",
    };
    dsv::Tablero tablero(window, miLayout,  sf::Color(40,40,50),  sf::Color(20,20,25));

    // Paneles
    auto faseSI = tablero.add<dsv::EspacioFase2D>("Fase: Susceptibles vs Infectados", dsv::Color::azul, "f1");
    auto faseIR = tablero.add<dsv::EspacioFase2D>("Fase: Infectados vs Recuperados", dsv::Color::azul, "f2");
    auto faseRS = tablero.add<dsv::EspacioFase2D>("Fase: Recuperados vs Susceptibles", dsv::Color::azul, "f3");
    auto fase3D = tablero.add<dsv::Grafica3D>("Trayectorias SIR (S,I,R)", dsv::Color::cian, "3d");
    auto tiempo = tablero.add<dsv::GraficaTiempo>("Evolucion Infectados (mutiples Tiradas)", dsv::Color::naranja, "t1");


    // consigurar el fase3D
    fase3D.objeto.getEjes().setLimites(-50, 210);

    // Configuración visual
    // fase->configurarLimites(0, 100, 0, 100, true);
    tiempo->configurarLimites(0, 50, 0, 100, true);
    tiempo->ponerSombreado(false);
    
    // digarams fase 2d
    for( auto fff : { faseSI, faseIR, faseRS } ){
        fff->activarSeguimiento(true);
        // fff->configurarMaxPoints(50);
        fff->ponerCabeza(false);
    }

    // aplicarlo a todos de una vez
    tablero.setPanelDegradado(  sf::Color(30,30,40),  sf::Color(20,20,25));
    
    // Inicializar 20 instancias
    const int numSims = 20;
    std::vector<dsv::mod::SIR_Instance> sims(numSims);
    

    for( int i = 0; i < numSims; i++ ){
        std::string id = "Sim" + std::to_string(i);
        sf::Color col = dsv::Color::Oceano(i, numSims);

        if( i == numSims -1 ){
            col = dsv::Color::rojo; // la primera simulacion en rojo para destacar
            sims[i].model.ruido = 0.0f; // la primera simulacion sin ruido para destacar
        }

        sims[i].state = {200.0f, 10.0f, 0.0f}; // S=99, I=1, R=0
        
        for( auto fase : {faseIR,faseSI,faseRS })
            fase->agregarSerie(id, col);

        tiempo->agregarSerie(id, col);
        fase3D.objeto.getGestor().agregarSerie(id, col);

        if( i == numSims-1 ){
            fase3D.objeto.getGestor().setGrosor(5.0f, id);
            fase3D.objeto.getGestor().setColor( dsv::Color::Magma() , id);
        }
    }

    sf::Clock clock;
    sf::Time accumulator = sf::Time::Zero;
    sf::Time ups = sf::seconds(0.066f);
    bool iniciado = false;

    while(window.isOpen()){
        // --- --- ---  GESTINAR EVENTOS Y REPARTIR --- --- ---
        sf::Event event;
        while(window.pollEvent(event)) {
            if(event.type == sf::Event::Closed) window.close();
            if(event.type == sf::Event::KeyPressed){
                iniciado = true;
                accumulator = sf::Time::Zero;
                clock.restart();
            } 
            fase3D->gestionarEvento(event, window);
        }

        // --- --- --- ACTUALIZAR DATOS DE SIMULACION --- --- ---
        accumulator += clock.restart();
        while(accumulator >= ups && iniciado) {
            // actualizar cada  instancia            
            for( int i = 0; i < numSims; i++){
                auto& s = sims[i];
                std::string id = "Sim" + std::to_string(i);

                //  integrador
                dsv::sim::step(s, ups.asSeconds());
            }

             // --- --- --- AGREGAR DATOS MAS RECIENTES --- --- ---
            for( int i = 0; i < numSims; i++){
                auto& s = sims[i];
                std::string id = "Sim" + std::to_string(i);

                // Llenar graficas
                faseSI->push_back(s.state[0], s.state[1], id);
                faseIR->push_back(s.state[1], s.state[2], id);
                faseRS->push_back(s.state[2], s.state[0], id);

                tiempo->push_back(s.state[1], s.t, id);
                fase3D.objeto.getGestor().push_back( {s.state[0], s.state[1], s.state[2]}, id);
            }

            accumulator -= ups;
        }
        
        
         // --- --- --- DIBUJAR --- --- ---
        tablero.draw();
        window.display();
    
     
    }
    return 0;
}