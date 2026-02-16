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
    sistema dinamicos de hormigas
*/
#include <SFML/Graphics.hpp>
#include <cmath>
#include <vector>
#include <algorithm>


#include "DynSysVis.hpp"
// #include "GraficoCircular.hpp"


int main( ){
    sf::Font roboto;
    roboto.loadFromFile("assets/fonts/Roboto.ttf");

    // --- cargar colores ---
    std::cout << "1. Cargando temas..." << std::endl;
    Tema::cargar("assets/config/colores.txt");

    // --- configurar ventana ---
    std::cout << "2. Inicializando ventana..." << std::endl;
    sf::RenderWindow window;
    Sistema::inicializarVentana(window, "Simulacion de Hormigas");


    // --- parametros para simular lo de hormigas ---

   
    // poblaciones iniciales
// --- ESCENARIO 2: oscilaciones amortiguadas ---

float O = 60.0f;
float G = 20.0f;
float R = 10.0f;

const float bO = 0.05f;
const float betaOG = 4.0f;
const float betaGR = 2.5f;
const float betaRG = 1.2f;
const float kG = 50.0f;
const float kR = 50.0f;
const float dG = 0.005f;
const float dR = 0.005f;

    std::cout << "3. Creando paneles..." << std::endl;
    // --- paneles derehca----
    Panel panelG(window, Tema::c("guerreras"), "Poblacion de Guerreras G(t)", 3,3 );
    panelG.positionAbsoluta(Ubicacion::ArribaDer);

    Panel panelR(window,  Tema::c("recolectoras"), "Poblacion de Recolectoras R(t)", 3,3  );
    panelR.positionRelativa(RelativoA::Abajo, panelG);

    Panel panelO(window,  Tema::c("obreras"), "Poblacion de Obreras O(t)", 3,3 );
    panelO.positionRelativa(RelativoA::Abajo, panelR);
    
     // --- paneles izquieda----
    Panel panelfOG(window, Tema::c("color1"), "Espacio Fase (O, G)", 6, 4);
    panelfOG.positionAbsoluta(Ubicacion::ArribaIzq);

    Panel panelfOR(window,  Tema::c("color2"), "Espacio Fase (O, R)", 6, 4);
    panelfOR.positionRelativa(RelativoA::Abajo  , panelfOG);

    Panel panelfRG(window,  Tema::c("color3"), "Espacio Fase (R, G)", 6, 4);
    panelfRG.positionRelativa(RelativoA::Der  , panelfOR);

    Panel panelCirc(window,  Tema::c("rojo"), "Poblacion de Hormigas", 3, 2);
    panelCirc.positionRelativa(RelativoA::Abajo, panelfOR);

    // --- paneles centro----
    Panel panelTriple(window, Tema::c("celeste"),"Poblaciones de Hormigas", 3, 3); 
    panelTriple.positionRelativa(RelativoA::Izq  , panelG);
    
    Panel panelP(window, Tema::c("celeste"),"Poblacion Total de Hormigas", 3, 3); 
    panelP.positionRelativa(RelativoA::Abajo  , panelTriple);
    
    // --- graficas respecto a tiempo y respecot a fase ---
    auto* graphG = panelG.crearContenido<GraficaTiempo>(Tema::c("guerreras"));
    auto* graphR = panelR.crearContenido<GraficaTiempo>(Tema::c("recolectoras"));
    auto* graphO = panelO.crearContenido<GraficaTiempo>(Tema::c("obreras"));
    auto* graphP = panelP.crearContenido<GraficaTiempo>(Tema::c("white"));

    auto* graphTriple = panelTriple.crearContenido<GraficaTiempo>(Tema::c("guerreras"));
    graphTriple -> agregarSerie("guerreras",Tema::c("guerreras"));
    graphTriple -> agregarSerie("recolectoras",Tema::c("recolectoras"));
    graphTriple -> agregarSerie("obreras",Tema::c("obreras"));
    
    graphTriple -> configurarMaxPoints(5000);
    graphG -> configurarMaxPoints(5000);
    graphR -> configurarMaxPoints(5000);
    graphO -> configurarMaxPoints(5000);
    graphP -> configurarMaxPoints(5000);

    auto* faseOG = panelfOG.crearContenido<GraficaEspacioFase>(Tema::c("color1"));
    auto* faseOR = panelfOR.crearContenido<GraficaEspacioFase>(Tema::c("color2"));
    auto* faseRG = panelfRG.crearContenido<GraficaEspacioFase>(Tema::c("color3"));

    faseOG -> configurarMaxPoints(5000);
    faseOR -> configurarMaxPoints(5000);  
    faseRG -> configurarMaxPoints(5000);


    std::vector<sf::Color> colores = { Tema::c("obreras"), Tema::c("guerreras"), Tema::c("recolectoras") };
    auto* circular = panelCirc.crearContenido<GraficoCircular>();
    circular -> personalizarColores( colores );



    // --- IMPORTANTE: control del tiempo --
    //  (Paso fijo de 0.1s)
    sf::Clock clock;
    sf::Time accumulator = sf::Time::Zero;
    sf::Time ups = sf::seconds(0.005f); // Update por segundo
    // ups = sf::seconds(0.5f); // Update por segundo

    std::cout << "5. Inicializando simulacion   ..." << std::endl;
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
            graphO -> addValue(O);
            graphG -> addValue(G);
            graphR -> addValue(R);
            graphP -> addValue(O+G+R);

            graphTriple -> addValue(O, "obreras");
            graphTriple -> addValue(G, "guerreras");
            graphTriple -> addValue(R, "recolectoras");

            faseOG -> addValue(O,G);
            faseOR -> addValue(O,R);
            faseRG -> addValue(R,G);

            circular -> addValues( {O, G, R} );


            accumulator -= ups;
        }

        // --- RENDERIZADO ---
        window.clear(sf::Color(40, 40, 40)); // fondo oscuro tipo Sci-Fi

        // Dibujar paneles y sus graficas internas
        panelG.draw();
        panelR.draw();
        panelO.draw();
        panelP.draw();
        panelfOG.draw();
        panelfOR.draw();
        panelfRG.draw();

        panelCirc.draw();

        panelTriple.draw();

        window.display();
    }

    return 0;
}