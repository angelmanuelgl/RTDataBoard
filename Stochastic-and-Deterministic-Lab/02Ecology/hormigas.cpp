/*  * autor: Angel Manuel Gonzalez Lopez 
    * github: https://github.com/angelmanuelgl
    * web: https://angelmanuelgl.github.io/
    * 
    * proyecto: DynSysVis - - - - - - - - - - - - - - - - - - - 
 */
/*  MAIN.cpp
    ejemplo para usar mi libreria 
*/

#include <SFML/Graphics.hpp>
#include <cmath>
#include <vector>
#include <algorithm>


#include "DynSysVis.hpp"

// using namespace dsv;



int main( ){
    // --- cargar colores --- DSV
    dsv::Color::cargar("assets/config/colores.txt");

    // --- configurar ventana --- SFML
    sf::RenderWindow window;
    dsv::Sistema::inicializarVentana(window, "DynSysVis RT - Colonia de Hormigas");



    // --- --- --- --- --- --- --- -- --- --- --- --- --- --- ---
    // --- --- --- --- --- --- MODELO --- --- --- --- --- --- ---
    // --- --- --- --- --- --- --- -- --- --- --- --- --- --- ---

    // --- INSTANCIA DEL MODELO ---
    using Ants = dsv::mod::AntColony_Model;
    dsv::mod::Instance<Ants> hormigas;
    
    // Condiciones iniciales
    hormigas.state[Ants::O] = 60.0f;
    hormigas.state[Ants::G] = 20.0f;
    hormigas.state[Ants::R] = 10.0f;

    // Referencia corta al modelo para los menús
    auto& m = hormigas.getModel();

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
    dsv::CampoControl& mH_fila1 = menuHormigas->agregarFila();
    mH_fila1.agregar<dsv::CampoBarra>("Obreras ", &hormigas.state[Ants::O], 0.f, 500.f, sf::Color(80,240,100));
    
    dsv::CampoControl& mH_fila2 = menuHormigas->agregarFila();
    mH_fila2.agregar<dsv::CampoDeslizador>("Nacimiento Obreras ", &m.bO, 0.0f, 0.5f);

    // Recolectoras
    dsv::CampoControl& mH_fila3 = menuHormigas->agregarFila();
    mH_fila3.agregar<dsv::CampoBarra>("Recolectoras ", &hormigas.state[Ants::R], 0.f, 500.f, dsv::Color::oro);
    
    dsv::CampoControl& mH_fila4 = menuHormigas->agregarFila();
    mH_fila4.agregar<dsv::CampoDeslizador>("Muerte Recolectoras ", &m.dR, 0.0f, 0.5f);

    // Guerreras
    dsv::CampoControl& mH_fila5 = menuHormigas->agregarFila();
    mH_fila5.agregar<dsv::CampoBarra>("Guerreras ", &hormigas.state[Ants::G], 0.f, 500.f, dsv::Color::rojo);
    
    dsv::CampoControl& mH_fila6 = menuHormigas->agregarFila();
    mH_fila6.agregar<dsv::CampoDeslizador>("Muerte Guerreras ", &m.dG, 0.0f, 0.5f);


   
    // --- --- --- --- --- --- --- -- --- --- --- --- --- --- ---
    // --- --- --- --- --- --- PANELES --- --- --- --- --- --- --
    // --- --- --- --- --- --- --- -- --- --- --- --- --- --- ---
    
    // --- tablero con datos --- DSV
    // dsv::Layout miLayout = {
    //     "fa1 fa1 FA1  o o o",
    //     "fa1 fa1 FA1  r r r",
    //     "fa2 fa2 FA2  g g g",
    //     "fa2 fa2 FA2  t t t",
    //     "fa3 fa3 FA3  all all all",
    //     "fa3 fa3 FA3  .  . cir",
    // };
    dsv::Layout miLayout = {
        "fa1 fa1 FA1 fa3 fa3 FA3",
        "fa2 fa2 FA2 all all all",
        "t   t   cir all all all",
        "o   o   g   g   r   r",
    };
    dsv::Tablero tablero(window, miLayout, dsv::Color::panelUp, dsv::Color::panelDown);


    /// GraficaTiempo
    dsv::Vista<dsv::GraficaTiempo> obreras      = tablero.add<dsv::GraficaTiempo>("obreras", dsv::Color::verde, "o", dsv::Color::verde);
    dsv::Vista<dsv::GraficaTiempo> guerreras    = tablero.add<dsv::GraficaTiempo>("guerreras", dsv::Color::rojo, "g", dsv::Color::rojo);
    dsv::Vista<dsv::GraficaTiempo> recolectoras = tablero.add<dsv::GraficaTiempo>("recolectoras", dsv::Color::oro, "r", dsv::Color::oro);
    
    dsv::Vista<dsv::GraficaTiempo> total = tablero.add<dsv::GraficaTiempo>("Poblaciones Total", dsv::Color::celeste, "t" );

    /// EspacioFase2D
    dsv::Vista<dsv::EspacioFase2D> OG = tablero.add<dsv::EspacioFase2D>("Espacio Fase Vista General (O, G)", dsv::Color::cian, "fa1", dsv::Color::cian );
    dsv::Vista<dsv::EspacioFase2D> OR = tablero.add<dsv::EspacioFase2D>("Espacio Fase Vista General (O, R)", dsv::Color::cian, "fa2",   dsv::Color::cian );
    dsv::Vista<dsv::EspacioFase2D> RG = tablero.add<dsv::EspacioFase2D>("Espacio Fase Vista General (R, G)", dsv::Color::cian, "fa3",   dsv::Color::cian );

    dsv::Vista<dsv::EspacioFase2D> OG2 = tablero.add<dsv::EspacioFase2D>("Seguimiento(O, G)", dsv::Color::aqua, "FA1",   dsv::Color::azul );
    dsv::Vista<dsv::EspacioFase2D> OR2 = tablero.add<dsv::EspacioFase2D>("Seguimiento(R, G)", dsv::Color::aqua, "FA2",   dsv::Color::azul );
    dsv::Vista<dsv::EspacioFase2D> RG2 = tablero.add<dsv::EspacioFase2D>("Seguimiento(R, G)", dsv::Color::aqua, "FA3",   dsv::Color::azul );

    /// GraficoCircular
    dsv::Vista<dsv::GraficoCircular> pie = tablero.add<dsv::GraficoCircular>("Poblacion de Hormigas", dsv::Color::aqua, "cir");

    pie -> personalizarColores( { dsv::Color::verde ,  dsv::Color::oro , dsv::Color::rojo} );

    /// GraficaTiempo // tambien admite multiserie
    dsv::Vista<dsv::GraficaTiempo> triple = tablero.add<dsv::GraficaTiempo>("Poblaciones de Hormigas", dsv::Color::celeste, "all" );

    triple -> agregarSerie("recolectoras", dsv::Color::oro);
    triple -> agregarSerie("obreras"     , dsv::Color::verde);
    triple -> agregarSerie("guerreras"   , dsv::Color::rojo);

    // --- --- CONFIGURACION (OCPIONAL) --- --- 
    for( dsv::Vista<dsv::GraficaTiempo> graf : {obreras, guerreras, recolectoras,total}  ){
        graf.objeto.configurarVentanaTiempo(40); // 40s
   
    }

    triple.objeto.configurarVentanaTiempo(100); // 100s
    triple.objeto.configurarLimitesY(0,100); // 

    for( dsv::Vista<dsv::EspacioFase2D> graf : {OG, OR, RG}  ){
        graf.objeto.activarAutoescalado(true); // false = los ejes seran fijos // true =  los ejes se escalan automaticamente
        graf.objeto.configurarLimites(0.0f, 500.0f, 0.0f, 500.0f); //limites de eje x // limites eje y 
        graf.objeto.ponerDesvanecido(false); // la cola de los datos no se desvanece
        graf.objeto.ponerCabeza(false); // quitamos el punto que por defecto se pone en valor actual
    }

    for( dsv::Vista<dsv::EspacioFase2D> graf : {OG2, OR2, RG2}  ){
        graf.objeto.activarSeguimiento(true); // el encuadre se ajusta a los datos
        graf.objeto.ponerDesvanecido(true); // la cola de los datos se desvanece
        graf.objeto.configurarMaxPoints(200); // limitar la cantidad de puntos en la cola de datos
        
    }



    // el degradado se aplica a todos los paneles de una vez que esten dentro del tablero
    tablero.setPanelDegradado(  dsv::Color::panelUp, dsv::Color::panelDown);


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

        while( accumulator >= ups  ){
            // --- --- --- --- ACTUALIZAR MODELO --- --- --- ---
            float dt = ups.asSeconds();
            
            // la libreria ya tiene un integrador
            dsv::sim::step(hormigas, dt);

            tiempo = hormigas.t;  // muestra
            accumulator -= ups;
        }

        // --- --- agregar datos a graficas --- --- 
        if( !pausa ){
            float O_val = hormigas.state[Ants::O];
            float G_val = hormigas.state[Ants::G];
            float R_val = hormigas.state[Ants::R];
            float t_val = hormigas.t;

            obreras->push_back(O_val, t_val);
            guerreras->push_back(G_val, t_val);
            recolectoras->push_back(R_val, t_val);
            total->push_back(O_val + G_val + R_val, t_val);

            triple->push_back(O_val, t_val, "obreras");
            triple->push_back(G_val, t_val, "guerreras");
            triple->push_back(R_val, t_val, "recolectoras");

            OG->push_back(O_val, G_val);
            OR->push_back(O_val, R_val);
            RG->push_back(R_val, G_val);
            
            OG2->push_back(O_val, G_val);
            OR2->push_back(O_val, R_val);
            RG2->push_back(R_val, G_val);

            pie->push_back({O_val, G_val, R_val});
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