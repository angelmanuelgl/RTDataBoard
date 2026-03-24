#include <SFML/Graphics.hpp>
#include <cmath>
#include "DynSysVis.hpp"

int main() {

    // --- --- --- --- --- --- --- --- --- --- --- --- --- --- ---
    // --- --- --- --- ---  PANELES Y VENTANA  --- --- --- --- ---
    // --- --- --- --- --- --- --- --- --- --- --- --- --- --- ---

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
    dsv::Vista< dsv::EspacioFase2D >  faseSI = tablero.add<dsv::EspacioFase2D>("Fase: Susceptibles vs Infectados", dsv::Color::azul, "f1");
    dsv::Vista< dsv::EspacioFase2D > faseIR = tablero.add<dsv::EspacioFase2D>("Fase: Infectados vs Recuperados", dsv::Color::azul, "f2");
    dsv::Vista< dsv::EspacioFase2D > faseRS = tablero.add<dsv::EspacioFase2D>("Fase: Recuperados vs Susceptibles", dsv::Color::azul, "f3");
    dsv::Vista< dsv::Grafica3D > fase3D = tablero.add<dsv::Grafica3D>("Trayectorias SIR (S,I,R)", dsv::Color::cian, "3d");
    dsv::Vista< dsv::GraficaTiempo > tiempo = tablero.add<dsv::GraficaTiempo>("Evolucion Infectados (mutiples Tiradas)", dsv::Color::naranja, "t1");


    // consigurar el fase3D
    fase3D.objeto.getEjes().setLimites(-50, 200);
    fase3D.objeto.getGestor().setAdelgazadoSeries(false);
    fase3D.objeto.getGestor().setDifuminadoSeries(false);

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
    

    // --- --- --- --- --- --- --- -- --- --- --- --- --- --- ---
    // --- --- --- --- --- --- INICIALIZAR --- --- --- --- --- ---
    // --- --- --- --- --- --- --- -- --- --- --- --- --- --- ---

    // ---------- Inicializar 20 instancias -----------
    const int numSims = 20;

    dsv::mod::SIR_Model modeloSIR; 
    modeloSIR.beta= 0.1f; modeloSIR.gamma= 0.01f; modeloSIR.ruido= 0.01f ; // infectados recuperados ruido
    std::vector<dsv::mod::SIR_Instance> sims(numSims);


    // inicializar modelo
    for( int i = 0; i < numSims; i++ ){
        sims[i].state = {200.0f, 10.0f, 0.0f}; // S=99, I=1, R=0
        sims[i].vincularModelo(modeloSIR); // comaprtimos el modelo para todo
    }


    // (OPCIONAL) puedes borrar esto y sigue sirviendo, solo toma colores por defcto
    // inicializar colores de cada serie
    for( int i = 0; i < numSims; i++ ){
        std::string id = "Sim" + std::to_string(i);
        sf::Color col = dsv::Color::Oceano(i, numSims);
        
        for( auto fase : {faseIR,faseSI,faseRS })
            fase->agregarSerie(id, col);

        tiempo->agregarSerie(id, col);
        fase3D.objeto.getGestor().agregarSerie(id, col);

    }


    // ---- ---- --- --- inicialziar instancia determinista --- --- ---
    dsv::mod::SIR_Instance sd; // determinista
    sd.getModel().beta = 0.1f;
    sd.getModel().gamma = 0.01f;
    sd.getModel().ruido = 0.0f;
    sd.state = {200.0f, 10.0f, 0.0f};
    // colores grosor, etc (OPCIONAL)
    std::string id_det = "SimDeterminista";
    sf::Color col(255,0,0);
    for( auto fase : {faseIR,faseSI,faseRS })
            fase->agregarSerie(id_det, col);
    tiempo->agregarSerie(id_det, col);
    fase3D.objeto.getGestor().agregarSerie(id_det, col);
    fase3D.objeto.getGestor().setColor( sf::Color(255,0,0) , id_det);
    fase3D.objeto.getGestor().setGrosor( 6.0f, id_det);


    // --- --- --- --- --- --- --- -- --- --- --- --- --- --- ---
    // --- --- --- --- --- --- SIMULACION --- --- --- --- --- ---
    // --- --- --- --- --- --- --- -- --- --- --- --- --- --- ---
    sf::Clock clock;
    sf::Time accumulator = sf::Time::Zero;
    sf::Time ups = sf::seconds(0.066f);
    bool iniciado = false;

    while(window.isOpen()){
        // --- --- ---  GESTINAR EVENTOS Y REPARTIR --- --- ---
        sf::Event event;
        while(window.pollEvent(event)) {
            if(event.type == sf::Event::Closed) window.close();
            // inicia
            if(event.type == sf::Event::KeyPressed && !iniciado){
                iniciado = true;
                accumulator = sf::Time::Zero;
                clock.restart();
            } 
            // interactuar
            if (event.type == sf::Event::KeyPressed) {
                // Aumentar un valor (Ej: Volumen o Velocidad)
                if (event.key.code == sf::Keyboard::Add || event.key.code == sf::Keyboard::Up) {
                    modeloSIR.beta *= 1.1;
                }

                // Disminuir un valor
                if (event.key.code == sf::Keyboard::Subtract || event.key.code == sf::Keyboard::Down) {
                    modeloSIR.beta /= 1.1;
                }

                // Resetear a un valor default
                if (event.key.code == sf::Keyboard::R) {
                     modeloSIR.ruido = 0;
                }

            }
            // la grafica 3d gestiona sus propios eventos
            fase3D->gestionarEvento(event, window);
        }

        // --- --- --- ACTUALIZAR DATOS DE SIMULACION --- --- ---
        accumulator += clock.restart();
        while(accumulator >= ups && iniciado) {
            
            // actualizar cada  instancia            
            for( int i = 0; i < numSims; i++){
                auto& s = sims[i];
                dsv::sim::step(s, ups.asSeconds());
            }
            dsv::sim::step(sd, ups.asSeconds());
            
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
            // Llenar graficas del determinista
            faseSI->push_back(sd.state[0], sd.state[1], id_det);
            faseIR->push_back(sd.state[1], sd.state[2], id_det);
            faseRS->push_back(sd.state[2], sd.state[0], id_det);

            tiempo->push_back(sd.state[1], sd.t, id_det);
            fase3D.objeto.getGestor().push_back( {sd.state[0], sd.state[1], sd.state[2]}, id_det);
            
            accumulator -= ups;
        }
        
        
         // --- --- --- DIBUJAR --- --- ---
        tablero.draw();
        window.display();
    
     
    }
    return 0;
}