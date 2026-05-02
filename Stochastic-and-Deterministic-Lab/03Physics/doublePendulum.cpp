/*
 * proyecto: DynSysVis RT -- Pendulo Doble: Caos Mecanico
 *
 * El pendulo doble es el sistema caotico clasico por excelencia.
 * Por que es caotico:
 *   El exponente de Lyapunov es positivo: dos trayectorias con
 *   condiciones iniciales separadas por epsilon divergen como e^(lambda*t).
 *   Para este sistema lambda ~= 3-5 [1/s] segun los parametros.
 *   Dos angulos que difieren en 0.0001 rad (~0.006 grados) seran
 *   completamente distintos en ~2-4 segundos de simulacion.
 *   Esto es incompatible con prediccion a largo plazo: es caos deterministico.
 *
 * Estrategia de visualizacion:
 *
 *   +---------------------------+-------------+--------+
 *   |                           |             |        |
 *   |  Posicion fisica (x2,y2)  |  T1 vs T2   | W1vsW2 |
 *   |  masa 2 -- el atractor    |  (fase ang) | (fase  |
 *   |  Estelas cortas que muestran como      |  vel)  |
 *   |  se separan las trayectorias  +---------|        |
 *   |                           |             |        |
 *   +---------------------------+-------------+--------+
 *   |  Angulos T1(t), T2(t)     |  Velocidades W1(t), W2(t)   |
 *   |  divergen visiblemente    |  explosion caotica           |
 *   +---------------------------+------------------------------+
 *
 *  -- Mapa fisico (fiz): las 6 trayectorias empiezan juntas en (x,y)
 *    y se separan rapidamente. La estela de 30 puntos con desvanecido
 *    crea el efecto visual de "cometas" que se dispersan.
 *
 *  -- Fase T1 vs T2: muestra el espacio de configuracion del sistema.
 *    Para el pendulo simple era una orbita cerrada; aqui es un
 *    "paseo" aparentemente aleatorio que jamas se repite.
 *
 *  -- Fase W1 vs W2: velocidades angulares. La "explosion" de las
 *    trayectorias es mas dramatica aqui que en los angulos.
 *
 *  -- Tiempo T1(t) y T2(t): las curvas empiezan identicas y se
 *    desfasan de forma exponencial. El momento de ruptura visual
 *    depende de la diferencia en condiciones iniciales y de lambda.
 *
 * Integracion: RK4 obligatorio.
 *   Las ecuaciones de Euler-Lagrange tienen terminos como sin(T2-T1)
 *   y cos(T2-T1) con alta curvatura. Euler produce un drift energetico
 *   que hace que el sistema "gane" energia artificialmente, produciendo
 *   caos numerico adicional al real. RK4 conserva la energia con error
 *   O(dt^4) por paso, suficiente para ups=0.001f.
 */

#include <SFML/Graphics.hpp>
#include <cmath>
#include <algorithm>
#include <string>
#include <vector>

#include "DynSysVis.hpp"
// dsv::mod::DoublePendulum_Model ya esta definido en la libreria


// =============================================================================
//  CONSTANTES
// =============================================================================
static constexpr int   NUM_PEND = 6;    // trayectorias para ver la divergencia

// Condicion inicial base (angulos en radianes)
// 1.5 rad ~= 86 grados -- regimen altamente caotico
static constexpr float T1_BASE  = 1.5f;
static constexpr float T2_BASE  = 1.5f;
static constexpr float W1_BASE  = 0.0f;
static constexpr float W2_BASE  = 0.0f;

// Perturbacion entre instancias consecutivas
// 1e-4 rad: diferencia de 0.006 grados -- invisible a ojo desnudo
static constexpr float DELTA_T1 = 1e-4f;


// =============================================================================
//  MAIN
// =============================================================================
int main() {

    // --- Init -----------------------------------------------------------------
    dsv::Color::cargar("assets/config/colores.txt");
    sf::RenderWindow window;
    dsv::Sistema::inicializarVentana(window,
        "DynSysVis RT -- Pendulo Doble: Caos Mecanico");


    // =========================================================================
    //  MODELO BASE (compartido por todas las instancias)
    //  Los sliders modifican este objeto; las instancias lo leen via vincular.
    // =========================================================================
    dsv::mod::DoublePendulum_Model modeloBase;
    modeloBase.m1 = 1.0f;
    modeloBase.m2 = 1.0f;
    modeloBase.l1 = 1.0f;
    modeloBase.l2 = 1.0f;
    modeloBase.g  = 9.81f;


    // =========================================================================
    //  INSTANCIAS -- condiciones iniciales con diferencia minima en T1
    // =========================================================================
    std::vector<dsv::mod::Instance<dsv::mod::DoublePendulum_Model>> sims(NUM_PEND);

    for (int i = 0; i < NUM_PEND; ++i) {
        // T1 varia en DELTA_T1 entre instancias consecutivas
        // T2, W1, W2 identicos para todos
        sims[i].state[dsv::mod::DoublePendulum_Model::T1] = T1_BASE + i * DELTA_T1;
        sims[i].state[dsv::mod::DoublePendulum_Model::T2] = T2_BASE;
        sims[i].state[dsv::mod::DoublePendulum_Model::W1] = W1_BASE;
        sims[i].state[dsv::mod::DoublePendulum_Model::W2] = W2_BASE;
        sims[i].model_ref.vincular(modeloBase);
    }

    // Lambda de reset: vuelve a condiciones iniciales originales
    auto resetTodo = [&]() {
        for (int i = 0; i < NUM_PEND; ++i) {
            sims[i].state[dsv::mod::DoublePendulum_Model::T1] = T1_BASE + i * DELTA_T1;
            sims[i].state[dsv::mod::DoublePendulum_Model::T2] = T2_BASE;
            sims[i].state[dsv::mod::DoublePendulum_Model::W1] = W1_BASE;
            sims[i].state[dsv::mod::DoublePendulum_Model::W2] = W2_BASE;
            sims[i].t = 0.0f;
        }
    };


    // =========================================================================
    //  PALETA DE COLORES POR TRAYECTORIA
    //  Cyberpunk: contraste alto, colores saturados -- facil de distinguir
    //  en un fondo oscuro incluso cuando las curvas se solapan.
    // =========================================================================
    std::vector<sf::Color> paleta(NUM_PEND);
    for (int i = 0; i < NUM_PEND; ++i)
        paleta[i] = dsv::Color::Cyberpunk(i, NUM_PEND);


    // =========================================================================
    //  LAYOUT Y TABLERO
    //
    //  6 columnas x 4 filas:
    //    Filas 0-2 (3/4): mapa fisico (grande) | fase angulos | fase velocidades
    //    Fila  3  (1/4): tiempo angulos        | tiempo velocidades
    // =========================================================================
    dsv::Layout miLayout = {
        "fiz fiz fiz fiz T1T2 T1T2 W1W2 W1W2",
        "fiz fiz fiz fiz T1T2 T1T2 W1W2 W1W2",
        "fiz fiz fiz fiz T1T2 T1T2 W1W2 W1W2",
        "tAng tAng tAng tAng tVel tVel tVel tVel",
    };
    dsv::Tablero tablero(window, miLayout,
                         sf::Color(10, 10, 14), sf::Color(6, 6, 9));


    // =========================================================================
    //  MAPA FISICO -- posicion cartesiana de masa 2 (la punta del pendulo)
    //
    //  x1 = l1 * sin(T1)
    //  y1 = -l1 * cos(T1)
    //  x2 = x1 + l2 * sin(T2)
    //  y2 = y1 - l2 * cos(T2)
    //
    //  El rango maximo es [-l1+l2, l1+l2] en x y [-l1-l2, l1+l2] en y.
    //  Con l1=l2=1: [-2, 2] x [-2, 2].
    //  Con configurarMaxPoints(30) y desvanecido: estela tipo cometa.
    //  La separacion de las 6 estelas es la visualizacion del caos.
    // =========================================================================
    dsv::Vista<dsv::EspacioFase2D> mapFiz =
        tablero.add<dsv::EspacioFase2D>(
            "Posicion masa 2 -- Caos: x2=x1+l2*sin(T2), y2=y1-l2*cos(T2)",
            dsv::Color::violeta, "fiz");

    mapFiz->activarSeguimiento(false);
    mapFiz->activarAutoescalado(false);
    // Limites para l1=l2=1; si el usuario cambia l, pueden salirse -- aceptable
    mapFiz->configurarLimites(-2.2f, 2.2f, -2.2f, 2.2f);
    mapFiz->ponerDesvanecido(true);
    mapFiz->ponerCabeza(true);
    mapFiz->configurarMaxPoints(80);  // estela visible pero corta

    for (int i = 0; i < NUM_PEND; ++i)
        mapFiz->agregarSerie("P" + std::to_string(i), paleta[i]);


    // =========================================================================
    //  ESPACIO DE FASE -- T1 vs T2 (configuracion angular)
    //
    //  Para un pendulo simple: orbitas cerradas (ellipses).
    //  Para el pendulo doble: trayectoria erratica que cubre el espacio
    //  [-pi, pi] x [-pi, pi] de forma aparentemente aleatoria.
    //  Las 6 curvas se separan exponencialmente.
    // =========================================================================
    dsv::Vista<dsv::EspacioFase2D> faseT =
        tablero.add<dsv::EspacioFase2D>(
            "Fase Angular: T1 vs T2",
            dsv::Color::cian, "T1T2");

    faseT->activarSeguimiento(false);
    faseT->activarAutoescalado(false);
    // Angulos tipicamente en [-pi, pi] pero pueden salirse en regimen caotico
    faseT->configurarLimites(-4.0f, 4.0f, -4.0f, 4.0f);
    faseT->ponerDesvanecido(true);
    faseT->ponerCabeza(true);
    faseT->configurarMaxPoints(500);

    for (int i = 0; i < NUM_PEND; ++i)
        faseT->agregarSerie("P" + std::to_string(i), paleta[i]);


    // =========================================================================
    //  ESPACIO DE FASE -- W1 vs W2 (velocidades angulares)
    //
    //  Las velocidades angulares son mas sensibles al caos que los angulos.
    //  La "explosion" de las trayectorias aqui es mas dramatica y rapida.
    //  En regimen caotico W1, W2 pueden alcanzar valores de +-20 rad/s.
    // =========================================================================
    dsv::Vista<dsv::EspacioFase2D> faseW =
        tablero.add<dsv::EspacioFase2D>(
            "Fase Velocidades: W1 vs W2",
            dsv::Color::naranja, "W1W2");

    faseW->activarSeguimiento(true);   // seguimiento: los limites se ajustan al rango real
    faseW->ponerDesvanecido(true);
    faseW->ponerCabeza(true);
    faseW->configurarMaxPoints(300);

    for (int i = 0; i < NUM_PEND; ++i)
        faseW->agregarSerie("P" + std::to_string(i), paleta[i]);


    // =========================================================================
    //  TIEMPO -- angulos T1(t) y T2(t)
    //  Solo mostramos las 3 primeras trayectorias para no saturar.
    //  La divergencia entre la curva 0 y la curva 2 (separacion 2*DELTA_T1)
    //  es la evidencia visual mas clara del caos.
    // =========================================================================
    dsv::Vista<dsv::GraficaTiempo> grafAng =
        tablero.add<dsv::GraficaTiempo>(
            "Angulos T1(t) y T2(t) -- divergencia exponencial",
            dsv::Color::celeste, "tAng");

    grafAng.objeto.configurarVentanaTiempo(15.0f);
    grafAng.objeto.ponerSombreado(false);

    // Solo trayectorias 0, 2, 5 (primera, media, ultima) para claridad
    for (int idx : {0, 2, NUM_PEND-1}) {
        sf::Color c = paleta[idx];
        // T1: color solido
        grafAng->agregarSerie("T1_" + std::to_string(idx), c);
        // T2: mismo color mas transparente para distinguir los dos angulos
        sf::Color c2 = c; c2.a = 140;
        grafAng->agregarSerie("T2_" + std::to_string(idx), c2);
    }


    // =========================================================================
    //  TIEMPO -- velocidades angulares W1(t) y W2(t)
    // =========================================================================
    dsv::Vista<dsv::GraficaTiempo> grafVel =
        tablero.add<dsv::GraficaTiempo>(
            "Velocidades W1(t) y W2(t)",
            dsv::Color::naranja, "tVel");

    grafVel.objeto.configurarVentanaTiempo(15.0f);
    grafVel.objeto.ponerSombreado(false);

    for (int idx : {0, 2, NUM_PEND-1}) {
        sf::Color c = paleta[idx];
        grafVel->agregarSerie("W1_" + std::to_string(idx), c);
        sf::Color c2 = c; c2.a = 140;
        grafVel->agregarSerie("W2_" + std::to_string(idx), c2);
    }


    // Degradado tipo noche profunda
    tablero.setPanelDegradado(sf::Color(10, 10, 14), sf::Color(6, 6, 9));


    // =========================================================================
    //  PANEL FLOTANTE -- MENU TIEMPO (arriba centro)
    // =========================================================================
    bool  pausa     = true;
    float timeScale = 1.0f;
    sf::Clock fpsClock;
    long long frameCount = 0;
    float fps    = 0.0f;
    float tiempo = 0.0f;

    const sf::Color colorMENU = dsv::Color::naranja;

    dsv::PanelFlotante PanelTiempo(
        window, "Menu Tiempo",
        dsv::HandleOrientacion::Horizontal,
        dsv::DespliegueDir::Abajo,
        colorMENU
    );
    PanelTiempo.setDegradado(dsv::Color::naranja_dd % 230,
                              dsv::Color::naranja_dd % 230);
    PanelTiempo.positionAbsoluta(dsv::Ubicacion::ArribaCentro);

    dsv::MenuFlotante* menuTiempo = PanelTiempo.crearContenido<dsv::MenuFlotante>(6.f, 12.f);

    dsv::CampoControl& mt_f1 = menuTiempo->agregarFila();
    mt_f1.agregar<dsv::CampoTexto>("-- Pendulo Doble: Caos Mecanico --");

    dsv::CampoControl& mt_f2 = menuTiempo->agregarFila();
    mt_f2.agregar<dsv::CampoVariable>("fps",  &fps);
    mt_f2.agregar<dsv::CampoVariable>("t =",  &tiempo);

    dsv::CampoControl& mt_f3 = menuTiempo->agregarFila();
    mt_f3.agregar<dsv::CampoBoton>("<<",
        [&]{ timeScale = std::max(0.1f, timeScale - 0.1f); }, colorMENU);
    mt_f3.agregar<dsv::CampoToggleTexto>("", &pausa, "Pausa", "Play", colorMENU);
    mt_f3.agregar<dsv::CampoBoton>(">>", [&]{ timeScale += 0.1f; }, colorMENU);

    dsv::CampoControl& mt_f4 = menuTiempo->agregarFila();
    mt_f4.agregar<dsv::CampoVariable>("Speed x", &timeScale);

    dsv::CampoControl& mt_f5 = menuTiempo->agregarFila();
    mt_f5.agregar<dsv::CampoBoton>("Reset Vel", [&]{ timeScale = 1.0f; });
    mt_f5.agregar<dsv::CampoToggle>("Pausa", &pausa, colorMENU);

    dsv::CampoControl& mt_f6 = menuTiempo->agregarFila();
    mt_f6.agregar<dsv::CampoBoton>("Reset [R]",
        [&]{ resetTodo(); tiempo = 0.0f; }, colorMENU);

    // Energia del primer pendulo (referencia)
    float energiaDisplay = 0.0f;
    dsv::CampoControl& mt_f7 = menuTiempo->agregarFila();
    mt_f7.agregar<dsv::CampoVariable>("E[0] aprox =", &energiaDisplay);


    // =========================================================================
    //  PANEL FLOTANTE -- PARAMETROS FISICOS (derecha)
    // =========================================================================
    dsv::PanelFlotante PanelParams(
        window, "Params",
        dsv::HandleOrientacion::Vertical,
        dsv::DespliegueDir::Izq,
        dsv::Color::violeta
    );
    PanelParams.setDegradado(sf::Color(25, 10, 40, 245), sf::Color(12, 5, 20, 245));
    PanelParams.positionAbsoluta(dsv::Ubicacion::CentroDer);

    dsv::MenuFlotante* menuParams = PanelParams.crearContenido<dsv::MenuFlotante>(6.f, 14.f);

    dsv::CampoControl& pp_t = menuParams->agregarFila();
    pp_t.agregar<dsv::CampoTexto>("-- Parametros Fisicos --");

    // Masas
    dsv::CampoControl& pp_m1 = menuParams->agregarFila();
    pp_m1.agregar<dsv::CampoDeslizador>("m1  Masa 1",   &modeloBase.m1, 0.1f, 5.0f);

    dsv::CampoControl& pp_m2 = menuParams->agregarFila();
    pp_m2.agregar<dsv::CampoDeslizador>("m2  Masa 2",   &modeloBase.m2, 0.1f, 5.0f);

    // Longitudes -- afectan el rango fisico del mapa cartesiano
    dsv::CampoControl& pp_l1 = menuParams->agregarFila();
    pp_l1.agregar<dsv::CampoDeslizador>("l1  Long. 1",  &modeloBase.l1, 0.1f, 2.0f);

    dsv::CampoControl& pp_l2 = menuParams->agregarFila();
    pp_l2.agregar<dsv::CampoDeslizador>("l2  Long. 2",  &modeloBase.l2, 0.1f, 2.0f);

    // Gravedad
    dsv::CampoControl& pp_g = menuParams->agregarFila();
    pp_g.agregar<dsv::CampoDeslizador>("g   Gravedad",  &modeloBase.g,  1.0f, 25.0f);

    // Condicion inicial del angulo base (aplica en el proximo Reset)
    dsv::CampoControl& pp_sep = menuParams->agregarFila();
    pp_sep.agregar<dsv::CampoTexto>("-- Estado Actual P0 --");

    dsv::CampoControl& pp_T1 = menuParams->agregarFila();
    pp_T1.agregar<dsv::CampoVariable>("T1 =", &sims[0].state[dsv::mod::DoublePendulum_Model::T1]);

    dsv::CampoControl& pp_T2 = menuParams->agregarFila();
    pp_T2.agregar<dsv::CampoVariable>("T2 =", &sims[0].state[dsv::mod::DoublePendulum_Model::T2]);

    dsv::CampoControl& pp_W1 = menuParams->agregarFila();
    pp_W1.agregar<dsv::CampoVariable>("W1 =", &sims[0].state[dsv::mod::DoublePendulum_Model::W1]);

    dsv::CampoControl& pp_W2 = menuParams->agregarFila();
    pp_W2.agregar<dsv::CampoVariable>("W2 =", &sims[0].state[dsv::mod::DoublePendulum_Model::W2]);

    dsv::CampoControl& pp_res = menuParams->agregarFila();
    pp_res.agregar<dsv::CampoBoton>("Reset C.I.", [&]{
        resetTodo(); tiempo = 0.0f;
    }, dsv::Color::violeta);


    // =========================================================================
    //  BUCLE PRINCIPAL
    // =========================================================================
    sf::Clock clock;
    sf::Time  accumulator = sf::Time::Zero;
    // ups=0.001f: 1000 updates/s -- necesario para RK4 preciso en regimen caotico
    // La inestabilidad del pendulo doble requiere pasos pequenos
    sf::Time  ups = sf::seconds(0.001f);

    while (window.isOpen()) {

        // --- Eventos ----------------------------------------------------------
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window.close();

            PanelTiempo.gestionarEvento(event);
            PanelParams.gestionarEvento(event);

            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Space)
                    pausa = !pausa;
                if (event.key.code == sf::Keyboard::Up)
                    timeScale += 0.2f;
                if (event.key.code == sf::Keyboard::Down)
                    timeScale = std::max(0.1f, timeScale - 0.2f);
                if (event.key.code == sf::Keyboard::R) {
                    resetTodo();
                    tiempo = 0.0f;
                }
            }
        }

        // --- Simulacion -------------------------------------------------------
        sf::Time elapsed = clock.restart();
        if (!pausa)
            accumulator += elapsed * timeScale;

        while (accumulator >= ups) {
            float dt = ups.asSeconds();
            // RK4 es obligatorio para el pendulo doble:
            // -- Euler introduce un drift de energia que produce caos numerico
            //    adicional al caos real del sistema, contaminando la simulacion.
            // -- Con RK4 y dt=0.001, la conservacion de energia es ~1e-6 relativa
            //    por periodo, suficiente para observar el caos fisico real.
            for (auto& s : sims)
                dsv::sim::RK4_step(s, dt);

            tiempo = sims[0].t;

            // Energia mecanica aproximada del pendulo 0 (referencia de precision)
            // E ~= cinetica + potencial
            // E_cin ~= 0.5*m1*(l1*W1)^2 + 0.5*m2*(...)^2  (simplificado)
            // Solo usamos la parte cinetica como indicador de deriva
            {
                float W1 = sims[0].state[dsv::mod::DoublePendulum_Model::W1];
                float W2 = sims[0].state[dsv::mod::DoublePendulum_Model::W2];
                float T1 = sims[0].state[dsv::mod::DoublePendulum_Model::T1];
                float T2 = sims[0].state[dsv::mod::DoublePendulum_Model::T2];
                float l1 = modeloBase.l1, l2 = modeloBase.l2;
                float m1 = modeloBase.m1, m2 = modeloBase.m2;
                float g  = modeloBase.g;
                // Potencial gravitatorio (relativo al pivote)
                float V = -m1*g*l1*std::cos(T1) - m2*g*(l1*std::cos(T1) + l2*std::cos(T2));
                // Cinetica simplificada (sin terminos cruzados)
                float K = 0.5f*m1*l1*l1*W1*W1
                        + 0.5f*m2*(l1*l1*W1*W1 + l2*l2*W2*W2);
                energiaDisplay = K + V;
            }

            accumulator -= ups;
        }

        // --- Alimentar graficas -----------------------------------------------
        if (!pausa) {
            const float l1 = modeloBase.l1;
            const float l2 = modeloBase.l2;

            using DP = dsv::mod::DoublePendulum_Model;

            for (int i = 0; i < NUM_PEND; ++i) {
                const std::string id = "P" + std::to_string(i);

                float T1 = sims[i].state[DP::T1];
                float T2 = sims[i].state[DP::T2];
                float W1 = sims[i].state[DP::W1];
                float W2 = sims[i].state[DP::W2];
                float t  = sims[i].t;

                // Posicion cartesiana
                // Masa 1 (pivote intermedio)
                float x1 =  l1 * std::sin(T1);
                float y1 = -l1 * std::cos(T1);
                // Masa 2 (punta del pendulo) -- la que se grafica
                float x2 = x1 + l2 * std::sin(T2);
                float y2 = y1 - l2 * std::cos(T2);

                // Mapa fisico: posicion de la punta (masa 2)
                mapFiz->push_back(x2, y2, id);

                // Espacios de fase
                faseT->push_back(T1, T2, id);
                faseW->push_back(W1, W2, id);

                // Tiempo: solo 3 instancias seleccionadas para claridad
                if (i == 0 || i == 2 || i == NUM_PEND-1) {
                    grafAng->push_back(T1, t, "T1_" + std::to_string(i));
                    grafAng->push_back(T2, t, "T2_" + std::to_string(i));
                    grafVel->push_back(W1, t, "W1_" + std::to_string(i));
                    grafVel->push_back(W2, t, "W2_" + std::to_string(i));
                }
            }
        }

        // --- FPS --------------------------------------------------------------
        frameCount++;
        if (fpsClock.getElapsedTime().asSeconds() >= 1.0f) {
            fps        = static_cast<float>(frameCount);
            frameCount = 0;
            fpsClock.restart();
        }

        // --- Renderizado ------------------------------------------------------
        tablero.draw();
        PanelTiempo.draw();
        PanelParams.draw();
        window.display();
    }

    return 0;
}