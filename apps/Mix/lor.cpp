#include <SFML/Graphics.hpp>
#include <cmath>
#include "DynSysVis.hpp"


int main() {
    dsv::Color::cargar("assets/config/colores.txt");
    sf::RenderWindow window;
    dsv::Sistema::inicializarVentana(window, "DynSysVis RT - 20 Simulaciones SIR");

    dsv::Layout miLayout = {
        "3d 3d 3d 3d 3D 3D",
        "3d 3d 3d 3d 3D 3D",
        "3d 3d 3d 3d f2 . ",
        "3d 3d 3d 3d f3 f4 ",
    };
    // dsv::Layout miLayout = {
    //     "3d 3d  f2",
    //     "3d 3d  f3 ",
    //     "3d 3d  f4 "
    // };
    // dsv::Layout miLayout = {
    //     "3d",
    // };
    dsv::Tablero tablero(window, miLayout,  sf::Color(30,30,40),  sf::Color(20,20,25));
    
    // Paneles
    dsv::Vista< dsv::EspacioFase2D > faseXY = tablero.add<dsv::EspacioFase2D>("Fase: XY", dsv::Color::violeta, "f2");
    dsv::Vista< dsv::EspacioFase2D > faseYZ = tablero.add<dsv::EspacioFase2D>("Fase: YZ", dsv::Color::violeta, "f3");
    dsv::Vista< dsv::EspacioFase2D > faseXZ = tablero.add<dsv::EspacioFase2D>("Fase: XZ", dsv::Color::violeta, "f4");

    dsv::Vista< dsv::Grafica3D > fase3D = tablero.add<dsv::Grafica3D>("Trayectorias Modelo Lorentz", dsv::Color::violeta, "3d");
    dsv::Vista< dsv::Grafica3D > fase3D_proye = tablero.add<dsv::Grafica3D>("Trayectorias Modelo Lorentz", dsv::Color::rosa, "3D");
     
    tablero.setPanelDegradado( sf::Color(30,30,40),  sf::Color(20,20,25));
    

    // --- --- --- Configuracion visual --- --- --- 
    fase3D.objeto.getEjes().setLimites(-50, 100);
    for( auto fff : { faseXY, faseYZ, faseXZ } ){
        fff->activarSeguimiento(true);
        fff->configurarMaxPoints(50);
        fff->ponerCabeza(false);
    }
    for( auto fff : { fase3D, fase3D_proye } ){
        fff.objeto.getGestor().setMaxPointsSeries(50);
        fff.objeto.getGestor().setGrosorSeries(1.0f);
        fff.objeto.getGestor().setAdelgazadoSeries(false);
        fff.objeto.getGestor().setDifuminadoSeries(false);
        // ajustrar el degradado de TODAS las series
        fff.objeto.getGestor().setColorSeries( dsv::Color::Oceano() );
    }
  
    


    
    // --- --- ---  Inicializar 20 instancias --- --- --- 
    const int numSims = 500;
    std::vector<dsv::mod::Lorenz_Instance> sims(numSims);
    
    // rangos aleatorios
    std::random_device rd;  
    std::mt19937 gen(rd()); 
    std::uniform_real_distribution<float> uniform( -20.0f, 20.0f );


    for( int i = 0; i < numSims; i++ ){
        std::string id = "Sim" + std::to_string(i);
        sf::Color col = dsv::Color::Magma(i, numSims);
        // col.a = 20;

    
        sims[i].state = { uniform(gen),uniform(gen),uniform(gen)}; 
        
        
        for( auto fff : { faseXY, faseYZ, faseXZ} )
            fff-> agregarSerie(id, col);
        
        
        // por si queremos personalizar los colores individuales
        // std::vector<sf::Color> xd = {dsv::Color::morado, dsv::Color::azul,  dsv::Color::cian, dsv::Color::violeta, dsv::Color::morado,  dsv::Color::azul_noche};
        std::vector<sf::Color> xd = dsv::Color::Oceano();
        // fase3D.objeto.getGestor().agregarSerie(i, xd);
        // fase3D_proye.objeto.getGestor().agregarSerie(i, xd);

        if( !i){
            fase3D.objeto.getGestor().agregarSerie(i, sf::Color(255,0,0) );
            fase3D_proye.objeto.getGestor().agregarSerie(i, sf::Color(255,0,0));
            fase3D.objeto.getGestor().setGrosor(8, i);
            fase3D_proye.objeto.getGestor().setGrosor(8, i);
        }

    }
    
    // --- --- --- --- --- --- --- -- --- --- --- --- --- --- ---
    // --- --- --- --- --- --- SIMULACION --- --- --- --- --- ---
    // --- --- --- --- --- --- --- -- --- --- --- --- --- --- ---
    sf::Clock clock;
    sf::Time accumulator = sf::Time::Zero;
    sf::Time ups = sf::seconds(0.005f);
    bool iniciado = false;

    while(window.isOpen()) {
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
            fase3D_proye->gestionarEvento(event, window);
        }

        // --- --- --- ACTUALIZAR DATOS DE SIMULACION --- --- ---
        accumulator += clock.restart();
        while(accumulator >= ups && iniciado) {

            float dt=  ups.asSeconds()/10 ; // velocidad x0.5
            // actualizar cada  instancia            
            for( int i = 0; i < numSims; i++){
                auto& s = sims[i];
                std::string id = "Sim" + std::to_string(i);
                //  integrador
                dsv::sim::step(s, dt);
            }

            accumulator -= ups;
        }   

        // --- --- --- AGREGAR DATOS MAS RECIENTES --- --- ---
        // actualizar cada  instancia            
        for( int i = 0; i < numSims; i++){
            auto& s = sims[i];
            std::string id = "Sim" + std::to_string(i);

            // Llenar graficas
            faseXY->push_back(s.state[0], s.state[1], id);
            faseYZ->push_back(s.state[1], s.state[2], id);
            faseXZ->push_back(s.state[0], s.state[2], id);

            fase3D.objeto.getGestor().push_back( {s.state[0], s.state[1], s.state[2]}, i);
            
            if( i < 50 ){
                fase3D_proye.objeto.getGestor().push_back( {s.state[0], s.state[1], s.state[2]}, i);
                fase3D_proye.objeto.getGestor().push_back( {-40, s.state[1], s.state[2]}, i +numSims *1 );
                fase3D_proye.objeto.getGestor().push_back( {s.state[0], -40, s.state[2]}, i +numSims*2 );
                fase3D_proye.objeto.getGestor().push_back( {s.state[0], s.state[1], -40}, i +numSims*3 );
            }

        }

        // --- --- --- DIBUJAR --- --- ---
        tablero.draw();
        window.display();
        
     
    }
    return 0;
}