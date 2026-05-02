/*
 * proyecto: DynSysVis RT -- Dashboard SEIARD (Pandemia Completa)
 *
 * Paleta de compartimentos (identidad visual compartida con SIS/SIR/SEIR/SIRS):
 *   S (Susceptibles)   --> celeste
 *   E (Expuestos)      --> naranja
 *   I (Infectados)     --> rojo
 *   A (Asintomaticos)  --> magenta
 *   R (Recuperados)    --> verde
 *   D (Fallecidos)     --> gris_oscuro
 *
 * 5 instancias: I0 varia +/- 1% para ver sensibilidad al brote inicial.
 * La divergencia entre instancias mide la robustez del pico epidemico.
 *
 * Layout:
 *   +----------+----------+-----+-----+
 *   |          |          |     |     |
 *   |  evol    |  evol    | SI  | ID  |
 *   |  tiempo  |  tiempo  | fas | fas |
 *   |  (grande)|          |     |     |
 *   +----------+----------+-----+-----+
 *   |  pie     |  pie     | AI  | pie |
 *   +----------+----------+-----+-----+
 *
 * Paneles flotantes:
 *   ArribaCentro : Menu Tiempo (play/pausa/velocidad)
 *   CentroIzq    : Panel Parametros (beta_I, beta_A, epsilon, p, mu, gammas)
 *   CentroDer    : Panel Condiciones Iniciales (I0, E0 por instancia)
 */

#include <SFML/Graphics.hpp>
#include <cmath>
#include <algorithm>
#include <string>
#include <vector>

#include "DynSysVis.hpp"
// dsv::mod::SEIARD_Model ya declarado en Epidemiologicos.hpp incluido via DynSysVis


// =============================================================================
//  PALETA COMPARTIMENTADA (identica en todos los dashboards epidemicos)
// =============================================================================
namespace EpiColor {
    inline sf::Color S()  { return dsv::Color::celeste;     }
    inline sf::Color E()  { return dsv::Color::naranja;     }
    inline sf::Color I()  { return dsv::Color::rojo;        }
    inline sf::Color A()  { return dsv::Color::magenta;     }
    inline sf::Color R()  { return dsv::Color::verde;       }
    inline sf::Color D()  { return dsv::Color::gris_oscuro; }
}


// =============================================================================
//  CONSTANTES
// =============================================================================
static constexpr int   NUM_SIM = 5;
static constexpr float N_TOTAL = 10000.0f;  // poblacion total

// Condiciones iniciales base
static constexpr float S0_BASE = 9980.0f;
static constexpr float E0_BASE =   10.0f;
static constexpr float I0_BASE =   10.0f;
static constexpr float A0_BASE =    0.0f;
static constexpr float R0_BASE =    0.0f;
static constexpr float D0_BASE =    0.0f;

// Perturbacion de I0 entre instancias: +/- 1% de I0_BASE / NUM_SIM
static constexpr float DELTA_I0 = I0_BASE * 0.01f;


// =============================================================================
//  MAIN
// =============================================================================
int main() {

    dsv::Color::cargar("assets/config/colores.txt");
    sf::RenderWindow window;
    dsv::Sistema::inicializarVentana(window,
        "DynSysVis RT -- Dashboard SEIARD: Pandemia Completa");


    // =========================================================================
    //  MODELO BASE
    // =========================================================================
    dsv::mod::SEIARD_Model modeloBase;
    modeloBase.beta_I  = 0.30f;
    modeloBase.beta_A  = 0.15f;
    modeloBase.epsilon = 1.0f / 5.0f;
    modeloBase.p       = 0.60f;
    modeloBase.gamma_I = 0.05f;
    modeloBase.gamma_A = 0.08f;
    modeloBase.mu      = 0.005f;

    // Variables puente para condiciones iniciales ajustables
    float I0_ctrl = I0_BASE;
    float E0_ctrl = E0_BASE;


    // =========================================================================
    //  5 INSTANCIAS: I0 varia linealmente alrededor de I0_BASE
    //  instancia 0: I0 - 2*DELTA_I0
    //  instancia 2: I0 (base)
    //  instancia 4: I0 + 2*DELTA_I0
    // =========================================================================
    using M = dsv::mod::SEIARD_Model;
    using Inst = dsv::mod::Instance<M>;
    std::vector<Inst> sims(NUM_SIM);

    auto initSims = [&]() {
        for (int i = 0; i < NUM_SIM; ++i) {
            float dI = (i - NUM_SIM / 2) * DELTA_I0;
            float I_init = std::max(1.0f, I0_ctrl + dI);
            float S_init = N_TOTAL - E0_ctrl - I_init;

            sims[i].state[M::S] = std::max(0.0f, S_init);
            sims[i].state[M::E] = E0_ctrl;
            sims[i].state[M::I] = I_init;
            sims[i].state[M::A] = A0_BASE;
            sims[i].state[M::R] = R0_BASE;
            sims[i].state[M::D] = D0_BASE;
            sims[i].t           = 0.0f;
            sims[i].model_ref.vincular(modeloBase);
        }
    };
    initSims();

    // Paleta Viridis para distinguir las 5 instancias
    std::vector<sf::Color> paletaMC(NUM_SIM);
    for (int i = 0; i < NUM_SIM; ++i)
        paletaMC[i] = dsv::Color::Viridis(i, NUM_SIM);


    // =========================================================================
    //  LAYOUT
    // =========================================================================
    dsv::Layout miLayout = {
        "ev ev ev ev fSI fSI fID fID",
        "ev ev ev ev fSI fSI fID fID",
        "ev ev ev ev fAI fAI pie pie",
        "ev ev ev ev fAI fAI pie pie",
    };
    dsv::Tablero tablero(window, miLayout,
                         dsv::Color::panelUp, dsv::Color::panelDown);


    // =========================================================================
    //  GRAFICA DE TIEMPO -- evolucion de los 6 compartimentos (instancia 0)
    //  + banda de incertidumbre de las 5 instancias para S e I
    // =========================================================================
    dsv::Vista<dsv::GraficaTiempo> grafEv =
        tablero.add<dsv::GraficaTiempo>(
            "Evolucion SEIARD -- S E I A R D vs t",
            dsv::Color::celeste, "ev");

    grafEv->agregarSerie("S Susceptibles",  EpiColor::S());
    grafEv->agregarSerie("E Expuestos",     EpiColor::E());
    grafEv->agregarSerie("I Infectados",    EpiColor::I());
    grafEv->agregarSerie("A Asintomaticos", EpiColor::A());
    grafEv->agregarSerie("R Recuperados",   EpiColor::R());
    grafEv->agregarSerie("D Fallecidos",    EpiColor::D());

    // Banda MC: I de las otras 4 instancias (transparentes)
    for (int i = 1; i < NUM_SIM; ++i) {
        sf::Color c = EpiColor::I(); c.a = 60;
        grafEv->agregarSerie("I_mc" + std::to_string(i), c);
    }

    grafEv.objeto.configurarVentanaTiempo(400.0f);
    grafEv.objeto.ponerSombreado(false);


    // =========================================================================
    //  FASE S vs I -- agotamiento de susceptibles
    //  La trayectoria empieza en (S0, I0) y termina en (S_inf, 0).
    //  El punto S_inf (donde I=0 de nuevo) es el tamano final de la epidemia.
    //  Cuanto mas a la izquierda cae S_inf, mayor fue el impacto total.
    // =========================================================================
    dsv::Vista<dsv::EspacioFase2D> faseSI =
        tablero.add<dsv::EspacioFase2D>(
            "Fase S vs I -- agotamiento de susceptibles",
            EpiColor::S(), "fSI", EpiColor::I());

    faseSI->activarAutoescalado(true);
    faseSI->ponerDesvanecido(false);
    faseSI->ponerCabeza(true);
    faseSI->configurarMaxPoints(8000);
    for (int i = 0; i < NUM_SIM; ++i)
        faseSI->agregarSerie("SI" + std::to_string(i), paletaMC[i]);


    // =========================================================================
    //  FASE I vs D -- trayectoria de letalidad
    //  La pendiente local dD/dI = mu/(gamma_I+mu) = CFR.
    //  Si la curva sube rapido en D con poco I: alta mortalidad por caso.
    //  El area bajo esta curva es proporcional a la carga total de muertes.
    // =========================================================================
    dsv::Vista<dsv::EspacioFase2D> faseID =
        tablero.add<dsv::EspacioFase2D>(
            "Fase I vs D -- trayectoria de letalidad (CFR visible)",
            EpiColor::I(), "fID", EpiColor::D());

    faseID->activarAutoescalado(true);
    faseID->ponerDesvanecido(false);
    faseID->ponerCabeza(true);
    faseID->configurarMaxPoints(8000);
    for (int i = 0; i < NUM_SIM; ++i)
        faseID->agregarSerie("ID" + std::to_string(i), paletaMC[i]);


    // =========================================================================
    //  FASE A vs I -- frente silencioso vs visible
    //  Muestra la proporcion de contagio invisible (A) vs visible (I).
    //  Un A/I alto implica que la epidemia real es mucho mayor que la notificada.
    // =========================================================================
    dsv::Vista<dsv::EspacioFase2D> faseAI =
        tablero.add<dsv::EspacioFase2D>(
            "Fase A vs I -- contagio silencioso vs notificado",
            EpiColor::A(), "fAI", EpiColor::I());

    faseAI->activarAutoescalado(true);
    faseAI->ponerDesvanecido(true);
    faseAI->ponerCabeza(true);
    faseAI->configurarMaxPoints(4000);
    for (int i = 0; i < NUM_SIM; ++i)
        faseAI->agregarSerie("AI" + std::to_string(i), paletaMC[i]);


    // =========================================================================
    //  PIE CHART -- distribucion actual S:E:I:A:R:D
    //  Una "piramide" sana tiene S dominante.
    //  En el pico: I+A maximizan su sector. Al final: R+D dominan.
    // =========================================================================
    dsv::Vista<dsv::GraficoCircular> pie =
        tablero.add<dsv::GraficoCircular>(
            "Distribucion Actual [S:E:I:A:R:D]", dsv::Color::celeste, "pie");

    pie->personalizarColores({
        EpiColor::S(), EpiColor::E(), EpiColor::I(),
        EpiColor::A(), EpiColor::R(), EpiColor::D()
    });

    tablero.setPanelDegradado(dsv::Color::panelUp, dsv::Color::panelDown);


    // =========================================================================
    //  PANEL PARAMETROS (izquierda)
    // =========================================================================
    dsv::PanelFlotante PanelParams(
        window, "Params",
        dsv::HandleOrientacion::Vertical,
        dsv::DespliegueDir::Der,
        dsv::Color::rojo
    );
    PanelParams.setDegradado(sf::Color(45, 8, 8, 245), sf::Color(22, 4, 4, 245));
    PanelParams.positionAbsoluta(dsv::Ubicacion::CentroIzq);

    dsv::MenuFlotante* menuP = PanelParams.crearContenido<dsv::MenuFlotante>(6.f, 14.f);

    dsv::CampoControl& pp_t = menuP->agregarFila();
    pp_t.agregar<dsv::CampoTexto>("-- Parametros SEIARD --");

    // Barras de estado (instancia 0)
    auto& s0 = sims[0];
    dsv::CampoControl& pp_bS = menuP->agregarFila();
    pp_bS.agregar<dsv::CampoBarra>("S", &s0.state[M::S], 0.f, N_TOTAL, EpiColor::S());
    dsv::CampoControl& pp_bE = menuP->agregarFila();
    pp_bE.agregar<dsv::CampoBarra>("E", &s0.state[M::E], 0.f, N_TOTAL, EpiColor::E());
    dsv::CampoControl& pp_bI = menuP->agregarFila();
    pp_bI.agregar<dsv::CampoBarra>("I", &s0.state[M::I], 0.f, N_TOTAL, EpiColor::I());
    dsv::CampoControl& pp_bA = menuP->agregarFila();
    pp_bA.agregar<dsv::CampoBarra>("A", &s0.state[M::A], 0.f, N_TOTAL, EpiColor::A());
    dsv::CampoControl& pp_bR = menuP->agregarFila();
    pp_bR.agregar<dsv::CampoBarra>("R", &s0.state[M::R], 0.f, N_TOTAL, EpiColor::R());
    dsv::CampoControl& pp_bD = menuP->agregarFila();
    pp_bD.agregar<dsv::CampoBarra>("D", &s0.state[M::D], 0.f, N_TOTAL, EpiColor::D());

    dsv::CampoControl& pp_sep1 = menuP->agregarFila();
    pp_sep1.agregar<dsv::CampoTexto>("-- Tasas de Transmision --");

    dsv::CampoControl& pp_bI_sl = menuP->agregarFila();
    pp_bI_sl.agregar<dsv::CampoDeslizador>("beta_I  Sintom.",  &modeloBase.beta_I,  0.01f, 1.0f);
    dsv::CampoControl& pp_bA_sl = menuP->agregarFila();
    pp_bA_sl.agregar<dsv::CampoDeslizador>("beta_A  Asintom.", &modeloBase.beta_A,  0.01f, 1.0f);
    dsv::CampoControl& pp_eps = menuP->agregarFila();
    pp_eps.agregar<dsv::CampoDeslizador>("epsilon Incubac.",  &modeloBase.epsilon, 0.01f, 1.0f);
    dsv::CampoControl& pp_p = menuP->agregarFila();
    pp_p.agregar<dsv::CampoDeslizador>("p  Frac. Sintom.",   &modeloBase.p,       0.0f,  1.0f);

    dsv::CampoControl& pp_sep2 = menuP->agregarFila();
    pp_sep2.agregar<dsv::CampoTexto>("-- Recuperacion y Muerte --");

    dsv::CampoControl& pp_gI = menuP->agregarFila();
    pp_gI.agregar<dsv::CampoDeslizador>("gamma_I Recup.I",   &modeloBase.gamma_I, 0.001f, 0.5f);
    dsv::CampoControl& pp_gA = menuP->agregarFila();
    pp_gA.agregar<dsv::CampoDeslizador>("gamma_A Recup.A",   &modeloBase.gamma_A, 0.001f, 0.5f);
    dsv::CampoControl& pp_mu = menuP->agregarFila();
    pp_mu.agregar<dsv::CampoDeslizador>("mu  Mortalidad",    &modeloBase.mu,      0.0f,   0.1f);

    // R0 y CFR en vivo
    float R0_disp = 0.0f, CFR_disp = 0.0f;
    dsv::CampoControl& pp_R0 = menuP->agregarFila();
    pp_R0.agregar<dsv::CampoVariable>("R0 aprox =", &R0_disp);
    dsv::CampoControl& pp_CFR = menuP->agregarFila();
    pp_CFR.agregar<dsv::CampoVariable>("CFR  = mu/(mu+gI)", &CFR_disp);

    dsv::CampoControl& pp_res = menuP->agregarFila();
    pp_res.agregar<dsv::CampoBoton>("Reset Params", [&]{
        modeloBase.beta_I = 0.30f; modeloBase.beta_A = 0.15f;
        modeloBase.epsilon = 0.20f; modeloBase.p = 0.60f;
        modeloBase.gamma_I = 0.05f; modeloBase.gamma_A = 0.08f;
        modeloBase.mu = 0.005f;
    }, dsv::Color::rojo);


    // =========================================================================
    //  PANEL CONDICIONES INICIALES (derecha)
    // =========================================================================
    dsv::PanelFlotante PanelCI(
        window, "CI",
        dsv::HandleOrientacion::Vertical,
        dsv::DespliegueDir::Izq,
        dsv::Color::naranja
    );
    PanelCI.setDegradado(sf::Color(40, 20, 5, 245), sf::Color(20, 10, 3, 245));
    PanelCI.positionAbsoluta(dsv::Ubicacion::CentroDer);

    dsv::MenuFlotante* menuCI = PanelCI.crearContenido<dsv::MenuFlotante>(6.f, 14.f);

    dsv::CampoControl& ci_t = menuCI->agregarFila();
    ci_t.agregar<dsv::CampoTexto>("-- Condiciones Iniciales --");

    dsv::CampoControl& ci_sep1 = menuCI->agregarFila();
    ci_sep1.agregar<dsv::CampoTexto>("(5 instancias: I0 +/- 1%)");

    dsv::CampoControl& ci_I0 = menuCI->agregarFila();
    ci_I0.agregar<dsv::CampoDeslizador>("I0  Infectados ini.", &I0_ctrl, 1.0f, 500.0f);

    dsv::CampoControl& ci_E0 = menuCI->agregarFila();
    ci_E0.agregar<dsv::CampoVariable>("E0  Expuestos ini.",   &E0_ctrl);

    dsv::CampoControl& ci_sep2 = menuCI->agregarFila();
    ci_sep2.agregar<dsv::CampoTexto>("-- Estado Instancia 0 --");

    dsv::CampoControl& ci_t0 = menuCI->agregarFila();
    ci_t0.agregar<dsv::CampoVariable>("t  dias =", &sims[0].t);
    dsv::CampoControl& ci_I = menuCI->agregarFila();
    ci_I.agregar<dsv::CampoVariable>("I[0] =", &sims[0].state[M::I]);
    dsv::CampoControl& ci_D = menuCI->agregarFila();
    ci_D.agregar<dsv::CampoVariable>("D[0] =", &sims[0].state[M::D]);

    // Escenarios predefinidos
    dsv::CampoControl& ci_sep3 = menuCI->agregarFila();
    ci_sep3.agregar<dsv::CampoTexto>("-- Escenarios --");

    dsv::CampoControl& ci_leve = menuCI->agregarFila();
    ci_leve.agregar<dsv::CampoBoton>("Gripe leve", [&]{
        modeloBase.beta_I = 0.15f; modeloBase.mu = 0.001f;
        I0_ctrl = 5.0f; initSims();
    }, dsv::Color::verde);

    dsv::CampoControl& ci_covid = menuCI->agregarFila();
    ci_covid.agregar<dsv::CampoBoton>("Covid-like", [&]{
        modeloBase.beta_I = 0.25f; modeloBase.beta_A = 0.18f;
        modeloBase.mu = 0.005f; modeloBase.p = 0.50f;
        I0_ctrl = 10.0f; initSims();
    }, dsv::Color::naranja);

    dsv::CampoControl& ci_ebola = menuCI->agregarFila();
    ci_ebola.agregar<dsv::CampoBoton>("Alta letalidad", [&]{
        modeloBase.beta_I = 0.40f; modeloBase.mu = 0.05f;
        modeloBase.p = 0.95f; I0_ctrl = 3.0f; initSims();
    }, dsv::Color::rojo);

    dsv::CampoControl& ci_res = menuCI->agregarFila();
    ci_res.agregar<dsv::CampoBoton>("Reset CI [R]",
        [&]{ I0_ctrl = I0_BASE; E0_ctrl = E0_BASE; initSims(); },
        dsv::Color::naranja);


    // =========================================================================
    //  PANEL TIEMPO (arriba centro)
    // =========================================================================
    bool  pausa     = true;
    float timeScale = 1.0f;
    sf::Clock fpsClock;
    long long frameCount = 0;
    float fps = 0.0f, tiempo = 0.0f;

    const sf::Color colorMENU = dsv::Color::celeste;

    dsv::PanelFlotante PanelTiempo(
        window, "Menu Tiempo",
        dsv::HandleOrientacion::Horizontal,
        dsv::DespliegueDir::Abajo,
        colorMENU
    );
    PanelTiempo.setDegradado(sf::Color(8, 25, 45, 230), sf::Color(4, 12, 22, 230));
    PanelTiempo.positionAbsoluta(dsv::Ubicacion::ArribaCentro);

    dsv::MenuFlotante* menuT = PanelTiempo.crearContenido<dsv::MenuFlotante>(6.f, 12.f);

    dsv::CampoControl& mt1 = menuT->agregarFila();
    mt1.agregar<dsv::CampoTexto>("-- Dashboard SEIARD: Pandemia Completa --");
    dsv::CampoControl& mt2 = menuT->agregarFila();
    mt2.agregar<dsv::CampoVariable>("fps", &fps);
    mt2.agregar<dsv::CampoVariable>("dia =", &tiempo);
    dsv::CampoControl& mt3 = menuT->agregarFila();
    mt3.agregar<dsv::CampoBoton>("<<",
        [&]{ timeScale = std::max(0.1f, timeScale - 0.1f); }, colorMENU);
    mt3.agregar<dsv::CampoToggleTexto>("", &pausa, "Pausa", "Play", colorMENU);
    mt3.agregar<dsv::CampoBoton>(">>", [&]{ timeScale += 0.1f; }, colorMENU);
    dsv::CampoControl& mt4 = menuT->agregarFila();
    mt4.agregar<dsv::CampoVariable>("Speed x", &timeScale);
    dsv::CampoControl& mt5 = menuT->agregarFila();
    mt5.agregar<dsv::CampoBoton>("Reset Vel", [&]{ timeScale = 1.0f; });
    mt5.agregar<dsv::CampoToggle>("Pausa", &pausa, colorMENU);
    dsv::CampoControl& mt6 = menuT->agregarFila();
    mt6.agregar<dsv::CampoBoton>("Reset [R]",
        [&]{ initSims(); tiempo = 0.0f; }, colorMENU);


    // =========================================================================
    //  BUCLE PRINCIPAL
    // =========================================================================
    sf::Clock clock;
    sf::Time  accumulator = sf::Time::Zero;
    sf::Time  ups = sf::seconds(0.01f); // 1 step = 0.01 dias

    while (window.isOpen()) {

        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window.close();
            PanelTiempo.gestionarEvento(event);
            PanelParams.gestionarEvento(event);
            PanelCI.gestionarEvento(event);
            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Space) pausa = !pausa;
                if (event.key.code == sf::Keyboard::Up)    timeScale += 0.2f;
                if (event.key.code == sf::Keyboard::Down)  timeScale = std::max(0.1f, timeScale - 0.2f);
                if (event.key.code == sf::Keyboard::R)   { initSims(); tiempo = 0.0f; }
            }
        }

        // Metricas en vivo
        float dI = modeloBase.gamma_I + modeloBase.mu;
        R0_disp  = (dI > 0.0f)
                 ? (modeloBase.beta_I * modeloBase.p
                   + modeloBase.beta_A * (1.0f - modeloBase.p)) / modeloBase.gamma_I
                 : 0.0f;
        CFR_disp = (dI > 0.0f) ? modeloBase.mu / dI : 0.0f;

        sf::Time elapsed = clock.restart();
        if (!pausa) accumulator += elapsed * timeScale;

        while (accumulator >= ups) {
            for (auto& s : sims) dsv::sim::RK4_step(s, ups.asSeconds());
            tiempo = sims[0].t;
            accumulator -= ups;
        }

        if (!pausa) {
            float t = sims[0].t;

            // Instancia 0: curva principal (todas las series)
            grafEv->push_back(sims[0].state[M::S], t, "S Susceptibles");
            grafEv->push_back(sims[0].state[M::E], t, "E Expuestos");
            grafEv->push_back(sims[0].state[M::I], t, "I Infectados");
            grafEv->push_back(sims[0].state[M::A], t, "A Asintomaticos");
            grafEv->push_back(sims[0].state[M::R], t, "R Recuperados");
            grafEv->push_back(sims[0].state[M::D], t, "D Fallecidos");

            // Banda MC: I de instancias 1..4
            for (int i = 1; i < NUM_SIM; ++i)
                grafEv->push_back(sims[i].state[M::I], sims[i].t,
                                  "I_mc" + std::to_string(i));

            // Espacios de fase: todas las instancias
            for (int i = 0; i < NUM_SIM; ++i) {
                const std::string si = std::to_string(i);
                float S = sims[i].state[M::S];
                float I = sims[i].state[M::I];
                float A = sims[i].state[M::A];
                float D = sims[i].state[M::D];

                faseSI->push_back(S, I, "SI" + si);
                faseID->push_back(I, D, "ID" + si);
                faseAI->push_back(A, I, "AI" + si);
            }

            // Pie: instancia 0
            pie->push_back({
                sims[0].state[M::S], sims[0].state[M::E],
                sims[0].state[M::I], sims[0].state[M::A],
                sims[0].state[M::R], sims[0].state[M::D]
            });
        }

        frameCount++;
        if (fpsClock.getElapsedTime().asSeconds() >= 1.0f) {
            fps = static_cast<float>(frameCount);
            frameCount = 0; fpsClock.restart();
        }

        tablero.draw();
        PanelTiempo.draw(); PanelParams.draw(); PanelCI.draw();
        window.display();
    }
    return 0;
}