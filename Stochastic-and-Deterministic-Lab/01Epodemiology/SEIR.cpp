/*
 * autor: Angel Manuel Gonzalez Lopez
 * proyecto: DynSysVis — Simulacion SEIR en Tiempo Real
 *
 *  Modelo:  Susceptible → Expuesto → Infectado → Recuperado
 *  d=4, noise_dim=0 → integrador automatico E_step (Euler)
 *  Usamos RK4_step explicito para mayor precision en el pico de infeccion.
 */

#include <SFML/Graphics.hpp>
#include <cmath>
#include <algorithm>
#include <string>

#include "DynSysVis.hpp"



// ══════════════════════════════════════════════════════════════════════════════
//  MAIN
// ══════════════════════════════════════════════════════════════════════════════
int main() {

    // ── Inicializacion ────────────────────────────────────────────────────────
    dsv::Color::cargar("assets/config/colores.txt");
    sf::RenderWindow window;
    dsv::Sistema::inicializarVentana(window, "DynSysVis RT — Modelo SEIR");


    // ── Modelo ────────────────────────────────────────────────────────────────
    using SEIR = dsv::mod::SEIR_Model;
    dsv::mod::Instance<SEIR> seir;

    // Condiciones iniciales — poblacion total N = 1 000
    seir.state[SEIR::S] = 990.0f;
    seir.state[SEIR::E] =   5.0f;
    seir.state[SEIR::I] =   5.0f;
    seir.state[SEIR::R] =   0.0f;

    auto& m = seir.getModel();


    // ── Paleta epidemiologica ─────────────────────────────────────────────────
    //  S (Susceptibles) → celeste   E (Expuestos) → oro
    //  I (Infectados)   → rojo      R (Recuperados) → verde
    const sf::Color colS = dsv::Color::celeste;
    const sf::Color colE = dsv::Color::oro;
    const sf::Color colI = dsv::Color::rojo;
    const sf::Color colR = dsv::Color::verde;


    // ══════════════════════════════════════════════════════════════════════════
    //  LAYOUT
    //
    //  Diseño justificado para un epidemiologo:
    //
    //   ┌──────────────┬─────────┬─────────┐
    //   │              │  S vs I │  E vs I │  Diagramas de fase clave:
    //   │   3D (S,I,R) │         │         │  S vs I → umbral epidemico
    //   │   Atractor   ├─────────┴─────────┤  E vs I → latencia→contagio
    //   │              │  S,E,I,R vs t     │
    //   ├──────────────┤  Curva multiserie │  Curva multiserie → el "cuadro
    //   │  I(t)  │ Pie │  (panel ancho)    │  clinico" completo
    //   └────────┴─────┴───────────────────┘
    //
    //  El panel 3D ocupa 3 filas x 2 columnas (izquierda) para que el atractor
    //  tenga espacio para rotar comodamente con el mouse.
    // ══════════════════════════════════════════════════════════════════════════
    dsv::Layout miLayout = {
        "3d  3d  fSI fEI",
        "3d  3d  all all",
        "3d  3d  all all",
        "ti  ti  cir  .  ",
    };
    dsv::Tablero tablero(window, miLayout, dsv::Color::panelUp, dsv::Color::panelDown);


    // ── Grafica3D — Atractor SEIR en (S, I, R) ───────────────────────────────
    //  Usamos S, I, R como las 3 coordenadas (E queda implicita).
    //  La trayectoria parte de (990, 5, 0) y converge al equilibrio endemico
    //  o al punto libre de enfermedad segun R0.
    dsv::Vista<dsv::Grafica3D> fase3D =
        tablero.add<dsv::Grafica3D>("Atractor SEIR  (S, I, R)", dsv::Color::violeta, "3d");

    fase3D.objeto.getEjes().setLimites(-50.0f, 1100.0f);

    {
        auto& g = fase3D.objeto.getGestor();
        g.agregarSerie("SEIR", dsv::Color::violeta);
        g.setGrosor(2.0f, "SEIR");
        g.setDifuminadoSeries(true);
        g.setAdelgazadoSeries(false);
        g.setMaxPointsSeries(5000);
    }


    // ── EspacioFase2D — S vs I ────────────────────────────────────────────────
    //  El plano (S, I) es el diagrama clinico mas informativo:
    //  la trayectoria muestra el numero de reproduccion efectivo Rt = β·S/(γ·N).
    //  Cuando la curva cruza el eje I descendiendo, la epidemia termina.
    dsv::Vista<dsv::EspacioFase2D> faseSI =
        tablero.add<dsv::EspacioFase2D>("Fase: S vs I  (umbral epidemico)", dsv::Color::cian, "fSI", colS);

    faseSI->activarSeguimiento(false);
    faseSI->activarAutoescalado(true);
    faseSI->ponerDesvanecido(false);
    faseSI->ponerCabeza(true);
    faseSI->configurarMaxPoints(5000);


    // ── EspacioFase2D — E vs I ────────────────────────────────────────────────
    //  Muestra la relacion entre el reservorio latente (E) y el contagioso (I).
    //  El pico de E siempre precede al pico de I por 1/epsilon dias.
    dsv::Vista<dsv::EspacioFase2D> faseEI =
        tablero.add<dsv::EspacioFase2D>("Fase: E vs I  (latencia -> contagio)", dsv::Color::oro, "fEI", colE);

    faseEI->activarSeguimiento(true);
    faseEI->ponerDesvanecido(true);
    faseEI->ponerCabeza(true);
    faseEI->configurarMaxPoints(2000);


    // ── GraficaTiempo multiserie — S, E, I, R ────────────────────────────────
    //  La "curva epidemica" canonica. El epidemiologo busca:
    //  el pico de I (capacidad hospitalaria), el cruce S = N/R0 (herd immunity),
    //  y la velocidad de crecimiento de R.
    dsv::Vista<dsv::GraficaTiempo> todas =
        tablero.add<dsv::GraficaTiempo>("Evolucion SEIR — S, E, I, R", dsv::Color::celeste, "all");

    todas->agregarSerie("Susceptibles", colS);
    todas->agregarSerie("Expuestos",    colE);
    todas->agregarSerie("Infectados",   colI);
    todas->agregarSerie("Recuperados",  colR);
    todas.objeto.configurarVentanaTiempo(300.0f);
    todas.objeto.ponerSombreado(false);


    // ── GraficaTiempo — Curva de infeccion I(t) ──────────────────────────────
    //  Solo la curva I para detectar el pico con precision.
    dsv::Vista<dsv::GraficaTiempo> curvaI =
        tablero.add<dsv::GraficaTiempo>("Curva de Infeccion  I(t)", dsv::Color::rojo, "ti", colI);

    curvaI.objeto.configurarVentanaTiempo(300.0f);
    curvaI.objeto.configurarLimitesY(0.0f, 500.0f);
    curvaI.objeto.ponerSombreado(true);


    // ── GraficoCircular — Distribucion instantanea ───────────────────────────
    dsv::Vista<dsv::GraficoCircular> pie =
        tablero.add<dsv::GraficoCircular>("Distribucion Actual", dsv::Color::aqua, "cir");

    pie->personalizarColores({colS, colE, colI, colR});


    // ── Degradado global al tablero ───────────────────────────────────────────
    tablero.setPanelDegradado(dsv::Color::panelUp, dsv::Color::panelDown);


    // ══════════════════════════════════════════════════════════════════════════
    //  PANEL FLOTANTE — PARaMETROS DEL MODELO
    // ══════════════════════════════════════════════════════════════════════════
    dsv::PanelFlotante PanelModelo(
        window,
        "SEIR",
        dsv::HandleOrientacion::Vertical,
        dsv::DespliegueDir::Der,
        dsv::Color::violeta
    );
    PanelModelo.setDegradado(sf::Color(30, 20, 50, 245), sf::Color(12, 12, 18, 245));
    PanelModelo.positionAbsoluta(dsv::Ubicacion::CentroIzq);

    dsv::MenuFlotante* menuModelo = PanelModelo.crearContenido<dsv::MenuFlotante>(6.f, 14.f);

    // Titulo
    dsv::CampoControl& pm_t = menuModelo->agregarFila();
    pm_t.agregar<dsv::CampoTexto>(" - - Parametros SEIR - - ");

    // Barras de estado (solo lectura visual)
    dsv::CampoControl& pm_barS = menuModelo->agregarFila();
    pm_barS.agregar<dsv::CampoBarra>("S", &seir.state[SEIR::S], 0.f, 1000.f, colS);

    dsv::CampoControl& pm_barE = menuModelo->agregarFila();
    pm_barE.agregar<dsv::CampoBarra>("E", &seir.state[SEIR::E], 0.f, 1000.f, colE);

    dsv::CampoControl& pm_barI = menuModelo->agregarFila();
    pm_barI.agregar<dsv::CampoBarra>("I", &seir.state[SEIR::I], 0.f, 1000.f, colI);

    dsv::CampoControl& pm_barR = menuModelo->agregarFila();
    pm_barR.agregar<dsv::CampoBarra>("R", &seir.state[SEIR::R], 0.f, 1000.f, colR);

    // Separador
    dsv::CampoControl& pm_sep = menuModelo->agregarFila();
    pm_sep.agregar<dsv::CampoTexto>(" - - Constantes - - ");

    // Deslizadores de parametros
    dsv::CampoControl& pm_beta = menuModelo->agregarFila();
    pm_beta.agregar<dsv::CampoDeslizador>("BETA Transmision",    &m.beta,    0.01f, 1.0f);

    dsv::CampoControl& pm_gam = menuModelo->agregarFila();
    pm_gam.agregar<dsv::CampoDeslizador>("GAMA Recuperacion",   &m.gamma,   0.001f, 0.5f);

    dsv::CampoControl& pm_eps = menuModelo->agregarFila();
    pm_eps.agregar<dsv::CampoDeslizador>("SIGMA Incubacion",     &m.epsilon, 0.01f, 1.0f);

    // R0 calculado en vivo
    float R0_display = 0.0f;
    dsv::CampoControl& pm_R0 = menuModelo->agregarFila();
    pm_R0.agregar<dsv::CampoVariable>("R₀ = β/γ =", &R0_display);

    // Boton reset condiciones iniciales
    dsv::CampoControl& pm_reset = menuModelo->agregarFila();
    pm_reset.agregar<dsv::CampoBoton>("Reset C.I.", [&]{
        seir.state[SEIR::S] = 990.0f;
        seir.state[SEIR::E] =   5.0f;
        seir.state[SEIR::I] =   5.0f;
        seir.state[SEIR::R] =   0.0f;
        seir.t = 0.0f;
    }, dsv::Color::violeta);


    // ══════════════════════════════════════════════════════════════════════════
    //  PANEL FLOTANTE — CONTROL DE TIEMPO
    // ══════════════════════════════════════════════════════════════════════════
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
    PanelTiempo.positionAbsoluta(dsv::Ubicacion::ArribaCentro);

    dsv::MenuFlotante* menuTiempo = PanelTiempo.crearContenido<dsv::MenuFlotante>(6.f, 12.f);

    dsv::CampoControl& mt_f1 = menuTiempo->agregarFila();
    mt_f1.agregar<dsv::CampoTexto>(" - - - - CONTROLADOR - - - - ");

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


    // ══════════════════════════════════════════════════════════════════════════
    //  BUCLE PRINCIPAL
    // ══════════════════════════════════════════════════════════════════════════
    sf::Clock clock;
    sf::Time  accumulator = sf::Time::Zero;
    sf::Time  ups         = sf::seconds(0.005f); // 200 updates/s de simulacion

    while (window.isOpen()) {

        // ── Eventos ───────────────────────────────────────────────────────────
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window.close();

            PanelTiempo.gestionarEvento(event);
            PanelModelo.gestionarEvento(event);
            fase3D->gestionarEvento(event, window); // Grafica3D: requiere window

            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Space)
                    pausa = !pausa;
                if (event.key.code == sf::Keyboard::Up)
                    timeScale += 0.2f;
                if (event.key.code == sf::Keyboard::Down)
                    timeScale = std::max(0.1f, timeScale - 0.2f);
            }
        }

        // ── Simulacion — acumulador de tiempo ─────────────────────────────────
        sf::Time elapsed = clock.restart();
        if (!pausa)
            accumulator += elapsed * timeScale;

        while (accumulator >= ups) {
            // RK4 para mayor precision en el pico de infeccion
            dsv::sim::RK4_step(seir, ups.asSeconds());
            tiempo    = seir.t;
            R0_display = m.beta / m.gamma; // numero reproductivo basico
            accumulator -= ups;
        }

        // ── Alimentar graficas (solo cuando no esta en pausa) ─────────────────
        if (!pausa) {
            const float S = seir.state[SEIR::S];
            const float E = seir.state[SEIR::E];
            const float I = seir.state[SEIR::I];
            const float R = seir.state[SEIR::R];
            const float t = seir.t;

            // Series temporales
            todas->push_back(S, t, "Susceptibles");
            todas->push_back(E, t, "Expuestos");
            todas->push_back(I, t, "Infectados");
            todas->push_back(R, t, "Recuperados");

            curvaI->push_back(I, t);

            // Espacios de fase 2D
            faseSI->push_back(S, I);
            faseEI->push_back(E, I);

            // Espacio de fase 3D — coordenadas (S, I, R)
            fase3D.objeto.getGestor().push_back({S, I, R}, "SEIR");

            // Grafico circular — distribucion actual de los 4 compartimentos
            pie->push_back({S, E, I, R});
        }

        // ── FPS ───────────────────────────────────────────────────────────────
        frameCount++;
        if (fpsClock.getElapsedTime().asSeconds() >= 1.0f) {
            fps        = static_cast<float>(frameCount);
            frameCount = 0;
            fpsClock.restart();
        }

        // ── Renderizado ───────────────────────────────────────────────────────
        tablero.draw();         // graficas del layout
        PanelTiempo.draw();     // UI flotante (encima del tablero)
        PanelModelo.draw();
        window.display();
    }

    return 0;
}