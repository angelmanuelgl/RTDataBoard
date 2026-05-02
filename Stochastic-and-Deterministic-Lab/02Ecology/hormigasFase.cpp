#include <SFML/Graphics.hpp>
#include <cmath>
#include <vector>
#include <string>
#include "DynSysVis.hpp"
#include <random> // generar puntos aleatoriamente
using namespace dsv;


int main( ){
     // --- cargar colores --- DSV
    dsv::Color::cargar("assets/config/colores.txt");

    // --- configurar ventana --- SFML
    sf::RenderWindow window;
    dsv::Sistema::inicializarVentana(window, "DynSysVis RT - Colonia de Hormigas");
    // Layout según tu especificación


    // --- --- --- --- --- --- --- -- --- --- --- --- --- --- ---
    // --- --- --- --- --- --- MODELO --- --- --- --- --- --- ---
    // --- --- --- --- --- --- --- -- --- --- --- --- --- --- ---

    // --- INSTANCIA DEL MODELO ---
    using Ants = dsv::mod::AntColony_Model;

    dsv::mod::AntColony_Model modelo_comun;
    modelo_comun.bO= 0.8;
    modelo_comun.dG= 0.05;
    modelo_comun.dR= 0.05;

    std::vector< dsv::mod::Instance<Ants>  > arregloColonias (50);

    // --- inicilaizar  Simulaciones ---
    const int numTotal = 50;
    
    // rangos aleatorios
    std::random_device rd;  
    std::mt19937 gen(rd()); 
    std::uniform_real_distribution<float> disG( 20.0f, 200.0f);
    std::uniform_real_distribution<float> disR( 20.0f, 200.0f);
    std::uniform_real_distribution<float> disO( 20.0f, 200.0f);

    // inicialziar y vinuclar el miso modelo a todas
    for(auto& unaColonia : arregloColonias){
        float initO = disO(gen);
        float initG = disG(gen);
        float initR = disR(gen);


        // Condiciones iniciales
        unaColonia.state[Ants::O] = initO;
        unaColonia.state[Ants::G] = initG;
        unaColonia.state[Ants::R] = initR;

        // tomar el mismo modelo (osea mismas constantes para todas)
        unaColonia.model_ref.vincular(modelo_comun);
    }


    // Referencia corta al modelo para los menús
    // no deberria iportar cual agarramos
    auto& m = arregloColonias[0].getModel();

    // --- MENU PARA AJUSTAR MODELO (VINCULADO A LA INSTANCIA) ---
    // PANEL
    dsv::PanelFlotante PanelMenuHormigas(
        window, 
        "info", 
        dsv::HandleOrientacion::Vertical,
        dsv::DespliegueDir::Der,
        dsv::Color::morado      
    );
    PanelMenuHormigas.setDegradado( sf::Color(50,30,60,240),  sf::Color(20,20,25, 240) ); 
    PanelMenuHormigas.positionAbsoluta( dsv::Ubicacion::CentroIzq );
    
    
    // MENU
    dsv::MenuFlotante* menuHormigas = PanelMenuHormigas.crearContenido<dsv::MenuFlotante>(6.f, 12.f);

    // --- contenido --- 
    // obreras
    
    dsv::CampoControl& mH_fila2 = menuHormigas->agregarFila();
    mH_fila2.agregar<dsv::CampoDeslizador>("Nacimiento Obreras ", &m.bO, 0.0f, 0.5f);

    // Recolectoras
    dsv::CampoControl& mH_fila4 = menuHormigas->agregarFila();
    mH_fila4.agregar<dsv::CampoDeslizador>("Muerte Recolectoras ", &m.dR, 0.0f, 0.5f);

    // Guerreras
    dsv::CampoControl& mH_fila6 = menuHormigas->agregarFila();
    mH_fila6.agregar<dsv::CampoDeslizador>("Muerte Guerreras ", &m.dG, 0.0f, 0.5f);

    
    
    // --- --- --- --- --- --- --- -- --- --- --- --- --- --- ---
    // --- --- --- --- --- --- PANELES --- --- --- --- --- --- --
    // --- --- --- --- --- --- --- -- --- --- --- --- --- --- ---


    dsv::Layout miLayout = {
        "g5 g6 g7 g8 ",
        "f1 f2 f3 ",
        "f1 f2 f3 ",
        "f1 f2 f3 ",
        "f1 f2 f3 ",
        "g1 g2 g3 g4 ",
    };
    dsv::Tablero tablero(window, miLayout, dsv::Color::panelUp, dsv::Color::panelDown);


    //Diagramas fase
    dsv::Vista< dsv::EspacioFase2D > faseOG = tablero.add<dsv::EspacioFase2D>("Espacio Fase (O, G)", dsv::Color::azul, "f1");
    dsv::Vista< dsv::EspacioFase2D > faseOR = tablero.add<dsv::EspacioFase2D>("Espacio Fase (O, R)", dsv::Color::azul, "f2");
    dsv::Vista< dsv::EspacioFase2D > faseRG = tablero.add<dsv::EspacioFase2D>("Espacio Fase (R, G)", dsv::Color::azul, "f3");

    faseOG -> configurarLimites( 0, 400, 0 , 200);
    faseOR -> configurarLimites( 0, 400, 0 , 200);
    faseRG -> configurarLimites( 0, 100, 0 , 200);

    // configuracion
    for( dsv::Vista<dsv::EspacioFase2D>  it : {faseOG, faseOR, faseRG }){
        it.objeto.configurarMaxPoints(500);
        it.objeto.ponerCabeza(false);
        it.objeto.activarSeguimiento(true);
    }


    // paneles de tiempo
    const int MAXP = 8;
    std::vector<  Vista<dsv::GraficaTiempo> > panelesT;
    for(int i = 1; i <= MAXP; ++i ){
        auto g = tablero.add<dsv::GraficaTiempo>("Simulacion " + std::to_string(i), dsv::Color::naranja, "g" + std::to_string(i));
        g->agregarSerie("Obreras",  sf::Color(50,200,50));
        g->agregarSerie("Guerreras", sf::Color(200,50,50));
        g->agregarSerie("Recolectoras",  sf::Color(150,150,50));
        // g->configurarMaxPoints(1000);
        // g->configurarMaxLim(100);
        g.panel.setSizeTitulo(10);
        panelesT.push_back(g);
    }

    // el degradado se aplica a todos los paneles de una vez que esten dentro del tablero
    tablero.setPanelDegradado(  dsv::Color::panelUp, dsv::Color::panelDown);

    // ponerle colores
    for( int i = 0; i < numTotal; ++i ){
        std::string id = "C" + std::to_string(i);
        sf::Color col = dsv::Color::Cyberpunk(i, numTotal);

        // agregar series con colores
        faseOG->agregarSerie(id, col);
        faseOR->agregarSerie(id, col);
        faseRG->agregarSerie(id, col);
    }

    
     // --- --- --- --- --- --- --- -- --- --- --- --- --- --- ---
    // --- --- --- --- --- --- MENU DE TIEMPO --- --- --- --- ---
    // --- --- --- --- --- --- --- -- --- --- --- --- --- --- ---
    // eventos
    bool pausa = true;           
    float timeScale = 1.0f;       
    // fps e info
    sf::Clock fpsClock;        
    long long frameCount = 0;
    float fps = 0;
    float tiempo=0.0f;
    
    // PANEL
    sf::Color colorMENU = dsv::Color::naranja;
    // sf::Color colorMenu = dsv::Color::oro;
    dsv::PanelFlotante PanelMenuTiempo(
        window, 
        "Menu Tiempo", 
        dsv::HandleOrientacion::Horizontal,
        dsv::DespliegueDir::Abajo,
        colorMENU          
    );
    
    PanelMenuTiempo.setDegradado( dsv::Color::naranja_dd % 230,   dsv::Color::naranja_dd % 230  );  // con alpha 230
    PanelMenuTiempo.positionAbsoluta( dsv::Ubicacion::ArribaCentro );
    

    // MENU
    dsv::MenuFlotante* menu = PanelMenuTiempo.crearContenido<dsv::MenuFlotante>(6.f, 12.f);

    // COTENIDO
    dsv::CampoControl& menu_fila1 = menu->agregarFila();
    menu_fila1.agregar<dsv::CampoTexto>(" - - - - - CONTROLADOR - - - - - ");

    dsv::CampoControl& menu_fila2 = menu->agregarFila();
    menu_fila2.agregar<dsv::CampoVariable>("fps",      &fps);
    menu_fila2.agregar<dsv::CampoVariable>("tiempo",   &tiempo);

    dsv::CampoControl& menu_fila3 = menu->agregarFila();
    menu_fila3.agregar<dsv::CampoBoton>("<<", [&]{ timeScale = std::max(0.1f, timeScale - 0.1f); },  colorMENU);
    menu_fila3.agregar<dsv::CampoToggleTexto>("", &pausa, "Pausa", "Play", colorMENU);
    menu_fila3.agregar<dsv::CampoBoton>(">>", [&]{ timeScale += 0.1f; }, colorMENU);

    dsv::CampoControl& menu_fila4 = menu->agregarFila();
    menu_fila4.agregar<dsv::CampoVariable>("Velocidad:  x",    &timeScale);

    dsv::CampoControl& menu_fila5 = menu->agregarFila();
    menu_fila5.agregar<dsv::CampoBoton>("Reset Vel", [&]{ timeScale = 1.0; });
    menu_fila5.agregar<dsv::CampoToggle>("Pausa", &pausa,  colorMENU);

    
    
    // --- --- --- --- --- --- --- -- --- --- --- --- --- --- ---
    // --- --- --- --- --- --- SIMULACION --- --- --- --- --- ---
    // --- --- --- --- --- --- --- -- --- --- --- --- --- --- ---

    // --- --- --- CONTROL TIEMPO --- --- ---
    sf::Clock clock;
    sf::Time accumulator = sf::Time::Zero;
    sf::Time ups = sf::seconds(0.005f); // Update por segundo
    
     while( window.isOpen() ){
        // --- --- --- ---  ECVENTOS --- --- --- --- 
        sf::Event event;
        while( window.pollEvent(event) ){
            if( event.type == sf::Event::Closed) window.close();

            PanelMenuTiempo.gestionarEvento(event);
            PanelMenuHormigas.gestionarEvento(event);

            // extra
            if( event.type == sf::Event::KeyPressed ){ 
                if( event.key.code == sf::Keyboard::Space ){ 
                    pausa = !pausa; 
                }
                if( event.key.code == sf::Keyboard::Up ){ 
                    timeScale += 0.2f; // Acelerar
                }
                if( event.key.code == sf::Keyboard::Down ){ 
                    timeScale = std::max(0.1f, timeScale - 0.2f); // Desacelerar (mínimo 0.1)
                }
            }
        }

        // --- --- --- ---  SIMULACION --- --- --- --- 
        sf::Time elapsed = clock.restart();

        //  Si no está pausado, lo sumamos al acumulador afectado por la escala
        if( !pausa  ){ 
            accumulator += elapsed * timeScale;
        }


        while( accumulator >= ups ){
             // --- --- --- --- ACTUALIZAR MODELO --- --- --- ---
            float dt = ups.asSeconds();

            for(int i = 0; i < numTotal; ++i ){
                // la libreria ya tiene un integrador 
                dsv::sim::step(arregloColonias[i], dt);
            }
            tiempo = arregloColonias[0].t;  // muestra
            accumulator -= ups;
        }


        // --- --- agregar datos a graficas --- --- 
        if( !pausa ){
            for( int i = 0; i < numTotal; ++i ){
                float O = arregloColonias[i].state[Ants::O];
                float R = arregloColonias[i].state[Ants::R];
                float G = arregloColonias[i].state[Ants::G];
                float t = arregloColonias[i].t;

                std::string id = "C" + std::to_string(i);
                // Todas las 20 van a los diagramas de fase
                faseOG->push_back(O, G, id  );
                faseOR->push_back(O, R,  id);
                faseRG->push_back(R, G,  id);

                // Solo las primeras 6 van a los paneles individuales
                if( i < MAXP ){
                    panelesT[i]->push_back(O, t, "Obreras");
                    panelesT[i]->push_back(G, t, "Guerreras");
                    panelesT[i]->push_back(R, t, "Recolectoras");
                }
            }
        }


        // --- --- --- --- --- calcular FPS --- --- --- --- --- 
        frameCount++;
        if( fpsClock.getElapsedTime().asSeconds() >= 1.0f ){ 
            fps = frameCount;
            frameCount = 0;
            fpsClock.restart();
        }


        // --- --- --- ---  Renderizado --- --- --- --- 
        // paneles de layout
        tablero.draw();
        // paneles flotantes
        PanelMenuTiempo.draw();
        PanelMenuHormigas.draw();
        
        window.display();
        
    }
    return 0;
}