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

#include "Libreria.hpp"
#include "GraficoCircular.hpp"


int main( ){
    // --- cargar colores ---
    std::cout << "1. Cargando temas..." << std::endl;
    Tema::cargar("assets/config/colores.txt");

    // --- configurar ventana ---
   std::cout << "2. Inicializando ventana..." << std::endl;
    sf::RenderWindow window;
    Sistema::inicializarVentana(window, "Simulacion de Hormigas - Tesis");


    // --- parametros para simular lo de hormigas ---

    // poblaciones iniciales
    float O = 30.0f; // Obreras
    float G = 30.0f;  // Guerreras
    float R = 40.0f;  // Recolectoras

    // transisciones y etc
    const float bO = 0.05f;     // natalidad obreras
    const float betaOG = 2.1f;   // transicion O->G
    const float betaGR = 0.9f;   // transicion G->R
    const float betaRG = 0.08f;  // transicion R->G
    const float kG = 80.0f;      // saturacion Guerreras
    const float kR = 80.0f;      // saturacion Recolectoras
    const float dG = 0.02f;      // muerte Guerreras
    const float dR = 0.04f;      // muerte Recolectoras

    std::cout << "3. Creando paneles..." << std::endl;
    // --- paneles----
    Panel panelG(window, Tema::c("guerreras"), 4,3 );
    panelG.positionAbsoluta(Ubicacion::ArribaDer);

    Panel panelR(window,  Tema::c("recolectoras"),  4,3 );
    panelR.positionRelativa(RelativoA::Abajo, panelG);

    Panel panelO(window,  Tema::c("obreras"), 4,3 );
    panelO.positionRelativa(RelativoA::Abajo, panelR);

    Panel panelf1(window, Tema::c("color1"), 5, 4);
    panelf1.positionAbsoluta(Ubicacion::ArribaIzq);

    Panel panelf2(window,  Tema::c("color2"), 5, 4);
    panelf2.positionRelativa(RelativoA::Abajo  , panelf1);

    Panel panelf3(window,  Tema::c("color3"), 5, 4);
    panelf3.positionRelativa(RelativoA::Abajo  , panelf2);

    Panel panelCirc(window,  Tema::c("rojo"), 5, 4);
    panelCirc.positionRelativa(RelativoA::Abajo, panelf3);

    std::cout << "4. Creando graficas..." << std::endl;
    
    
    // --- graficas respecto a tiempo y respecot a fase ---
    auto* graphG = panelG.crearContenido<GraficaTiempo>(Tema::c("guerreras"), "Poblacion de Guerreras G(t)");

    auto* graphR = panelR.crearContenido<GraficaTiempo>(Tema::c("recolectoras"), "Poblacion de Recolectoras R(t)");
    auto* graphO = panelO.crearContenido<GraficaTiempo>(Tema::c("obreras"), "Poblacion de Obreras O(t)");
    

    auto* fase1 = panelf1.crearContenido<GraficaEspacioFase>(Tema::c("color1"), "(Obreras, Guerreras)");
    auto* fase2 = panelf2.crearContenido<GraficaEspacioFase>(Tema::c("color2"), "(Obreras, Recolectoras)");
    auto* fase3 = panelf3.crearContenido<GraficaEspacioFase>(Tema::c("color3"), "(Recolectoras, Guerreras)");
    
    fase1 -> configurarMaxPoints(10000);
    fase2 -> configurarMaxPoints(10000);  
    fase3 -> configurarMaxPoints(10000);


    std::vector<sf::Color> colores = { Tema::c("obreras"), Tema::c("guerreras"), Tema::c("recolectoras") };
    auto* circular = panelCirc.crearContenido<GraficoCircular>();
    circular -> personalizarColores( colores );



    // --- IMPORTANTE: control del tiempo --
    //  (Paso fijo de 0.1s)
    sf::Clock clock;
    sf::Time accumulator = sf::Time::Zero;
    sf::Time ups = sf::seconds(0.01f); // Update por segundo
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

            fase1 -> addValue(O,G);
            fase2 -> addValue(O,R);
            fase3 -> addValue(R,G);

            circular -> addValues( {O, G, R} );


            accumulator -= ups;
        }

        // --- RENDERIZADO ---
        window.clear(sf::Color(15, 15, 15)); // Fondo oscuro tipo Sci-Fi

        // Dibujar paneles y sus graficas internas
        panelG.draw();
        panelR.draw();
        panelO.draw();
        panelf2.draw();
        panelf1.draw();
        panelf3.draw();

        panelCirc.draw();

        window.display();
    }

    return 0;
}