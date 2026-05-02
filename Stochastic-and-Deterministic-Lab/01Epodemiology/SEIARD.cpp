/*
 * proyecto: DynSysVis RT -- Modelo SEIARD (Pandemia Completa)
 *
 * Compartimentos:
 *   S  Susceptibles
 *   E  Expuestos (incubando, no contagiosos)
 *   I  Infectados sintomaticos
 *   A  Asintomaticos (contagiosos silenciosos)
 *   R  Recuperados
 *   D  Muertos (compartimento absorbente)
 *
 * Layout justificado para saturacion del sistema de salud:
 *
 *   +-------------------------------+-------+
 *   |                               |       |
 *   |  Evolucion global S,E,I,A,R,D | I vs D|
 *   |  (curva epidemica completa)   | fase  |
 *   |                               |       |
 *   +-------+-------+-------+-------+-------+
 *   | I(t)  | A(t)  | D(t)  |  pie chart    |
 *   +-------+-------+-------+---------------+
 *
 *  -- El panel grande "all" muestra CUANDO se satura la capacidad (pico I+A).
 *  -- El espacio de fase I vs D revela la trayectoria de letalidad:
 *     si la curva sube rapido en D con poco I, la tasa de mortalidad mu es alta.
 *  -- I(t) y A(t) individuales permiten ver los dos frentes de contagio.
 *  -- D(t) es el indicador de impacto real del sistema de salud.
 *  -- El pie chart muestra en tiempo real como se redistribuye la poblacion.
 */

#include <SFML/Graphics.hpp>
#include <cmath>
#include <algorithm>
#include <string>

#include "DynSysVis.hpp"



// =============================================================================
//  MAIN
// =============================================================================
int main() {

    // --- Init -----------------------------------------------------------------
    dsv::Color::cargar("assets/config/colores.txt");
    sf::RenderWindow window;
    dsv::Sistema::inicializarVentana(window, "DynSysVis RT -- Modelo SEIARD Pandemia");


    // --- Modelo ---------------------------------------------------------------
    using Mod = dsv::mod::SEIARD_Model;
    dsv::mod::Instance<Mod> pandemia;

    // Condiciones iniciales -- poblacion total N = 10 000
    pandemia.state[Mod::S] = 9990.0f;
    pandemia.state[Mod::E] =    5.0f;
    pandemia.state[Mod::I] =    5.0f;
    pandemia.state[Mod::A] =    0.0f;
    pandemia.state[Mod::R] =    0.0f;
    pandemia.state[Mod::D] =    0.0f;

    auto& m = pandemia.getModel();


    // --- Paleta de colores ----------------------------------------------------
    //  S azul/celeste  E oro/amarillo  I rojo
    //  A naranja       R verde         D gris
    const sf::Color colS = dsv::Color::celeste;
    const sf::Color colE = dsv::Color::oro;
    const sf::Color colI = dsv::Color::rojo;
    const sf::Color colA = dsv::Color::naranja;
    const sf::Color colR = dsv::Color::verde;
    const sf::Color colD = dsv::Color::gris;


    // --- Layout ---------------------------------------------------------------
    dsv::Layout miLayout = {
        "all all all all fID",
        "all all all all fID",
        "tI  tA  tD  cir cir",
    };
    dsv::Tablero tablero(window, miLayout, dsv::Color::panelUp, dsv::Color::panelDown);


    // --- GraficaTiempo multiserie -- curva epidemica completa -----------------
    //  Panel principal. Aqui se ve CUANDO el pico de I+A supera la capacidad
    //  hospitalaria y cuanto tarda D en crecer tras ese pico.
    dsv::Vista<dsv::GraficaTiempo> todas =
        tablero.add<dsv::GraficaTiempo>("Evolucion SEIARD", dsv::Color::celeste, "all");

    todas->agregarSerie("Susceptibles",  colS);
    todas->agregarSerie("Expuestos",     colE);
    todas->agregarSerie("Infectados",    colI);
    todas->agregarSerie("Asintomaticos", colA);
    todas->agregarSerie("Recuperados",   colR);
    todas->agregarSerie("Muertos",       colD);

    todas.objeto.configurarVentanaTiempo(400.0f);
    todas.objeto.ponerSombreado(false);


    // --- EspacioFase2D -- I vs D (trayectoria de letalidad) ------------------
    //  Si la curva sube rapido en el eje D con poco I, mu es alta.
    //  Si la curva se dobla pronto hacia el eje D, el pico de infeccion fue corto.
    //  El area bajo esta curva es proporcional a la carga total de mortalidad.
    dsv::Vista<dsv::EspacioFase2D> faseID =
        tablero.add<dsv::EspacioFase2D>("Fase: Infectados vs Muertos", dsv::Color::rojo, "fID", colI);

    faseID->activarAutoescalado(true);
    faseID->ponerDesvanecido(false);
    faseID->ponerCabeza(true);
    faseID->configurarMaxPoints(6000);


    // --- GraficaTiempo -- I(t) individual ------------------------------------
    //  Curva de infectados sintomaticos aislada con sombra.
    //  El pico de esta curva define la saturacion hospitalaria.
    dsv::Vista<dsv::GraficaTiempo> curvaI =
        tablero.add<dsv::GraficaTiempo>("Infectados I(t)", dsv::Color::rojo, "tI", colI);

    curvaI.objeto.configurarVentanaTiempo(400.0f);
    curvaI.objeto.ponerSombreado(true);


    // --- GraficaTiempo -- A(t) individual ------------------------------------
    //  Asintomaticos: el frente silencioso de la pandemia.
    //  Su pico suele ser mas alto que I pero invisible sin pruebas.
    dsv::Vista<dsv::GraficaTiempo> curvaA =
        tablero.add<dsv::GraficaTiempo>("Asintomaticos A(t)", dsv::Color::naranja, "tA", colA);

    curvaA.objeto.configurarVentanaTiempo(400.0f);
    curvaA.objeto.ponerSombreado(true);


    // --- GraficaTiempo -- D(t) individual ------------------------------------
    //  Muertos acumulados. Compartimento absorbente: solo crece.
    //  La pendiente de D(t) en el pico de I es la tasa de mortalidad observable.
    dsv::Vista<dsv::GraficaTiempo> curvaD =
        tablero.add<dsv::GraficaTiempo>("Muertos D(t)", dsv::Color::gris, "tD", colD);

    curvaD.objeto.configurarVentanaTiempo(400.0f);
    curvaD.objeto.ponerSombreado(false);


    // --- GraficoCircular -- distribucion actual de los 6 compartimentos ------
    dsv::Vista<dsv::GraficoCircular> pie =
        tablero.add<dsv::GraficoCircular>("Distribucion Actual", dsv::Color::aqua, "cir");

    pie->personalizarColores({colS, colE, colI, colA, colR, colD});


    // --- Degradado global al tablero -----------------------------------------
    tablero.setPanelDegradado(dsv::Color::panelUp, dsv::Color::panelDown);


    // =========================================================================
    //  PANEL FLOTANTE -- PARAMETROS DEL MODELO
    // =========================================================================
    dsv::PanelFlotante PanelModelo(
        window,
        "SEIARD",
        dsv::HandleOrientacion::Horizontal,
        dsv::DespliegueDir::Arriba,
        dsv::Color::rojo
    );
    PanelModelo.setDegradado(sf::Color(50, 15, 15, 245), sf::Color(15, 10, 10, 245));
    PanelModelo.positionAbsoluta(dsv::Ubicacion::AbajoCentro);

    dsv::MenuFlotante* menuModelo = PanelModelo.crearContenido<dsv::MenuFlotante>(6.f, 14.f);

    // Titulo
    dsv::CampoControl& pm_t = menuModelo->agregarFila();
    pm_t.agregar<dsv::CampoTexto>("--- Parametros SEIARD ---");

    // Barras de estado (solo lectura)
    dsv::CampoControl& pm_bS = menuModelo->agregarFila();
    pm_bS.agregar<dsv::CampoBarra>("S", &pandemia.state[Mod::S], 0.f, 10000.f, colS);

    dsv::CampoControl& pm_bE = menuModelo->agregarFila();
    pm_bE.agregar<dsv::CampoBarra>("E", &pandemia.state[Mod::E], 0.f, 10000.f, colE);

    dsv::CampoControl& pm_bI = menuModelo->agregarFila();
    pm_bI.agregar<dsv::CampoBarra>("I", &pandemia.state[Mod::I], 0.f, 10000.f, colI);

    dsv::CampoControl& pm_bA = menuModelo->agregarFila();
    pm_bA.agregar<dsv::CampoBarra>("A", &pandemia.state[Mod::A], 0.f, 10000.f, colA);

    dsv::CampoControl& pm_bR = menuModelo->agregarFila();
    pm_bR.agregar<dsv::CampoBarra>("R", &pandemia.state[Mod::R], 0.f, 10000.f, colR);

    dsv::CampoControl& pm_bD = menuModelo->agregarFila();
    pm_bD.agregar<dsv::CampoBarra>("D", &pandemia.state[Mod::D], 0.f, 10000.f, colD);

    // Separador
    dsv::CampoControl& pm_sep = menuModelo->agregarFila();
    pm_sep.agregar<dsv::CampoTexto>("--- Constantes ---");

    // Sliders de parametros
    dsv::CampoControl& pm_bI_sl = menuModelo->agregarFila();
    pm_bI_sl.agregar<dsv::CampoDeslizador>("beta_I  Sintom.",  &m.beta_I,  0.01f, 1.0f);

    dsv::CampoControl& pm_bA_sl = menuModelo->agregarFila();
    pm_bA_sl.agregar<dsv::CampoDeslizador>("beta_A  Asintom.", &m.beta_A,  0.01f, 1.0f);

    dsv::CampoControl& pm_p_sl = menuModelo->agregarFila();
    pm_p_sl.agregar<dsv::CampoDeslizador>("p  Frac.Sintom.",   &m.p,       0.0f,  1.0f);

    dsv::CampoControl& pm_mu_sl = menuModelo->agregarFila();
    pm_mu_sl.agregar<dsv::CampoDeslizador>("mu  Mortalidad",   &m.mu,      0.0f,  0.1f);

    dsv::CampoControl& pm_gI_sl = menuModelo->agregarFila();
    pm_gI_sl.agregar<dsv::CampoDeslizador>("gamma_I  Recup.I", &m.gamma_I, 0.001f, 0.5f);

    dsv::CampoControl& pm_gA_sl = menuModelo->agregarFila();
    pm_gA_sl.agregar<dsv::CampoDeslizador>("gamma_A  Recup.A", &m.gamma_A, 0.001f, 0.5f);

    // R0 efectivo y CFR en vivo
    float R0_display  = 0.0f;
    float CFR_display = 0.0f; // Case Fatality Rate = mu / (gamma_I + mu)

    dsv::CampoControl& pm_R0 = menuModelo->agregarFila();
    pm_R0.agregar<dsv::CampoVariable>("R0 aprox =", &R0_display);

    dsv::CampoControl& pm_cfr = menuModelo->agregarFila();
    pm_cfr.agregar<dsv::CampoVariable>("CFR  =", &CFR_display);

    // Boton reset
    dsv::CampoControl& pm_res = menuModelo->agregarFila();
    pm_res.agregar<dsv::CampoBoton>("Reset C.I.", [&]{
        pandemia.state[Mod::S] = 9990.0f;
        pandemia.state[Mod::E] =    5.0f;
        pandemia.state[Mod::I] =    5.0f;
        pandemia.state[Mod::A] =    0.0f;
        pandemia.state[Mod::R] =    0.0f;
        pandemia.state[Mod::D] =    0.0f;
        pandemia.t = 0.0f;
    }, dsv::Color::rojo);


    // =========================================================================
    //  PANEL FLOTANTE -- CONTROL DE TIEMPO
    // =========================================================================
    bool  pausa     = true;
    float timeScale = 1.0f;

    sf::Clock fpsClock;
    long long frameCount = 0;
    float fps    = 0.0f;
    float tiempo = 0.0f;

    const sf::Color colorMENU = dsv::Color::naranja;

    dsv::PanelFlotante PanelTiempo(
        window,
        "Menu Tiempo",
        dsv::HandleOrientacion::Horizontal,
        dsv::DespliegueDir::Abajo,
        colorMENU
    );
    PanelTiempo.setDegradado(dsv::Color::naranja_dd % 230, dsv::Color::naranja_dd % 230);
    PanelTiempo.positionAbsoluta(dsv::Ubicacion::ArribaIzq);

    dsv::MenuFlotante* menuTiempo = PanelTiempo.crearContenido<dsv::MenuFlotante>(6.f, 12.f);

    dsv::CampoControl& mt_f1 = menuTiempo->agregarFila();
    mt_f1.agregar<dsv::CampoTexto>("- - - - CONTROLADOR - - - -");

    dsv::CampoControl& mt_f2 = menuTiempo->agregarFila();
    mt_f2.agregar<dsv::CampoVariable>("fps",    &fps);
    mt_f2.agregar<dsv::CampoVariable>("tiempo", &tiempo);

    dsv::CampoControl& mt_f3 = menuTiempo->agregarFila();
    mt_f3.agregar<dsv::CampoBoton>("<<", [&]{ timeScale = std::max(0.1f, timeScale - 0.1f); }, colorMENU);
    mt_f3.agregar<dsv::CampoToggleTexto>("", &pausa, "Pausa", "Play", colorMENU);
    mt_f3.agregar<dsv::CampoBoton>(">>", [&]{ timeScale += 0.1f; }, colorMENU);

    dsv::CampoControl& mt_f4 = menuTiempo->agregarFila();
    mt_f4.agregar<dsv::CampoVariable>("Velocidad: x", &timeScale);

    dsv::CampoControl& mt_f5 = menuTiempo->agregarFila();
    mt_f5.agregar<dsv::CampoBoton>("Reset Vel", [&]{ timeScale = 1.0f; });
    mt_f5.agregar<dsv::CampoToggle>("Pausa", &pausa, colorMENU);


    // =========================================================================
    //  BUCLE PRINCIPAL
    // =========================================================================
    sf::Clock clock;
    sf::Time  accumulator = sf::Time::Zero;
    sf::Time  ups         = sf::seconds(0.005f); // 200 updates/s de simulacion

    while (window.isOpen()) {

        // --- Eventos ----------------------------------------------------------
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window.close();

            PanelTiempo.gestionarEvento(event);
            PanelModelo.gestionarEvento(event);

            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Space)
                    pausa = !pausa;
                if (event.key.code == sf::Keyboard::Up)
                    timeScale += 0.2f;
                if (event.key.code == sf::Keyboard::Down)
                    timeScale = std::max(0.1f, timeScale - 0.2f);
            }
        }

        // --- Simulacion -- acumulador de tiempo -------------------------------
        sf::Time elapsed = clock.restart();
        if (!pausa)
            accumulator += elapsed * timeScale;

        while (accumulator >= ups) {
            // RK4 para mayor precision cerca del pico de infeccion
            dsv::sim::RK4_step(pandemia, ups.asSeconds());
            tiempo = pandemia.t;

            // Metricas epidemiologicas calculadas en vivo
            // R0 aproximado: (beta_I * p + beta_A * (1-p)) / gamma_I
            R0_display  = (m.beta_I * m.p + m.beta_A * (1.0f - m.p)) / m.gamma_I;
            // CFR: fraccion de infectados que mueren
            CFR_display = m.mu / (m.gamma_I + m.mu);

            accumulator -= ups;
        }

        // --- Alimentar graficas (solo si no esta en pausa) --------------------
        if (!pausa) {
            const float S = pandemia.state[Mod::S];
            const float E = pandemia.state[Mod::E];
            const float I = pandemia.state[Mod::I];
            const float A = pandemia.state[Mod::A];
            const float R = pandemia.state[Mod::R];
            const float D = pandemia.state[Mod::D];
            const float t = pandemia.t;

            // Curva epidemica completa
            todas->push_back(S, t, "Susceptibles");
            todas->push_back(E, t, "Expuestos");
            todas->push_back(I, t, "Infectados");
            todas->push_back(A, t, "Asintomaticos");
            todas->push_back(R, t, "Recuperados");
            todas->push_back(D, t, "Muertos");

            // Curvas individuales
            curvaI->push_back(I, t);
            curvaA->push_back(A, t);
            curvaD->push_back(D, t);

            // Espacio de fase I vs D
            // -- el eje X es I (infectados activos)
            // -- el eje Y es D (muertos acumulados)
            // La pendiente local dD/dI = mu / (gamma_I + mu) = CFR
            faseID->push_back(I, D);

            // Distribucion actual en el pie chart
            pie->push_back({S, E, I, A, R, D});
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
        PanelModelo.draw();
        window.display();
    }

    return 0;
}