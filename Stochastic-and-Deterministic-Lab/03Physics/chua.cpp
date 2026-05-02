/*
 * proyecto: DynSysVis RT -- Atractor de Chua (Circuito Caotico)
 *
 * El circuito de Chua (1983) es el unico sistema electronico caotico que
 * se puede construir con componentes fisicos estandar. Su importancia es
 * que el caos NO es una abstraccion matematica: es medible en un osciloscopio.
 *
 * Estructura del circuito:
 *   C1 (condensador 1) -- voltaje x
 *   C2 (condensador 2) -- voltaje y
 *   L  (inductor)      -- corriente z
 *   Diodo de Chua (NR) -- elemento no lineal pasivo negativo
 *
 * Ecuaciones normalizadas:
 *   dx/dt = alpha * (y - x - h(x))
 *   dy/dt = x - y + z
 *   dz/dt = -beta * y
 *
 * Donde h(x) es la caracteristica SECCIONALMENTE LINEAL del diodo de Chua:
 *   h(x) = m1*x + 0.5*(m0-m1)*(|x+1| - |x-1|)
 *
 * h(x) tiene tres regiones:
 *   x < -1:  h(x) = m0 * x + (m0 - m1)    -- region exterior izquierda
 *  -1<x<1:   h(x) = m1 * x                -- region interior
 *   x >  1:  h(x) = m0 * x - (m0 - m1)    -- region exterior derecha
 *
 * Esta funcion crea la caracteristica de resistencia NEGATIVA del diodo.
 * Sin esta no-linealidad el circuito seria un oscilador de RLC normal.
 *
 * Parametros tipicos (doble espiral):
 *   alpha = 15.6,  beta = 28.0,  m0 = -1.143,  m1 = -0.714
 *   --> Doble espiral de Chua (double scroll attractor)
 *
 * El "doble scroll" tiene dos espirales simétricas centradas en:
 *   P+ = (1, 0, -1)  y  P- = (-1, 0, 1)
 * Las trayectorias rotan alrededor de uno, luego saltan al otro.
 * El salto es deterministico pero aparentemente aleatorio: CAOS.
 *
 * A diferencia de Lorenz (simetria de reflexion en Z),
 * Chua tiene simetria de INVERSION: (x,y,z) -> (-x,-y,-z)
 *
 * Dimension fractal (Kaplan-Yorke): ~2.05
 * Exponente de Lyapunov: lambda_1 ~= 0.32 [1/s] (entre Lorenz y Rossler)
 */

#include <SFML/Graphics.hpp>
#include <cmath>
#include <algorithm>
#include <string>
#include <vector>

#include "DynSysVis.hpp"
// dsv::mod::Chua_Model ya declarado en la libreria


static constexpr int   NUM_SIM  = 10;
static constexpr float DELTA_CI = 1e-4f;

// CI base: punto cerca del atractor (no en el origen, que es inestable)
static constexpr float X0 =  0.7f;
static constexpr float Y0 =  0.0f;
static constexpr float Z0 = -0.5f;


int main() {

    dsv::Color::cargar("assets/config/colores.txt");
    sf::RenderWindow window;
    dsv::Sistema::inicializarVentana(window, "DynSysVis RT -- Atractor de Chua: Doble Scroll");


    // =========================================================================
    //  MODELO BASE
    // =========================================================================
    dsv::mod::Chua_Model modeloBase;
    modeloBase.alpha = 15.6f;
    modeloBase.beta  = 28.0f;
    modeloBase.m0    = -1.143f;
    modeloBase.m1    = -0.714f;

    float x0_base = X0, y0_base = Y0, z0_base = Z0;
    float delta_ci = DELTA_CI;


    // =========================================================================
    //  10 INSTANCIAS
    // =========================================================================
    using ChuaInst = dsv::mod::Instance<dsv::mod::Chua_Model>;
    std::vector<ChuaInst> sims(NUM_SIM);

    auto initSims = [&]() {
        for (int i = 0; i < NUM_SIM; ++i) {
            sims[i].state[0] = x0_base + i * delta_ci;
            sims[i].state[1] = y0_base;
            sims[i].state[2] = z0_base;
            sims[i].t        = 0.0f;
            sims[i].model_ref.vincular(modeloBase);
        }
    };
    initSims();


    // =========================================================================
    //  PALETA OCEANO -- azules y cianes evocan el mundo electronico/digital
    //  Tambien referencia al laboratorio de Chua (electronica de circuitos)
    // =========================================================================
    std::vector<sf::Color> paleta(NUM_SIM);
    for (int i = 0; i < NUM_SIM; ++i)
        paleta[i] = dsv::Color::Oceano(i, NUM_SIM);


    // =========================================================================
    //  LAYOUT -- el "doble scroll" es mas claro en proyeccion XZ
    //  XZ: muestra los DOS lobulos simetricos (la "doble espiral")
    //  XY: muestra la estructura de bandas
    //  YZ: complementaria
    // =========================================================================
    dsv::Layout miLayout = {
        "3d 3d 3d 3d XZ XZ XZ XZ",
        "3d 3d 3d 3d XZ XZ XZ XZ",
        "3d 3d 3d 3d XY XY YZ YZ",
        "3d 3d 3d 3d zt zt YZ YZ",
    };
    dsv::Tablero tablero(window, miLayout,
                         sf::Color(3, 8, 20), sf::Color(2, 4, 10));


    // =========================================================================
    //  GRAFICA 3D
    // =========================================================================
    dsv::Vista<dsv::Grafica3D> graf3D =
        tablero.add<dsv::Grafica3D>(
            "Doble Scroll de Chua -- 10 trayectorias Oceano",
            dsv::Color::cian, "3d");

    // Limites del doble scroll: x in [-3,3], y in [-0.4,0.4], z in [-4,4]
    graf3D.objeto.getEjes().setLimites(-3.5f, 3.5f,
                                       -0.5f, 0.5f,
                                       -4.5f, 4.5f);
    graf3D.objeto.getEjes().colorX = sf::Color(0, 200, 255);   // cian electrico
    graf3D.objeto.getEjes().colorY = sf::Color(0, 100, 200);   // azul
    graf3D.objeto.getEjes().colorZ = sf::Color(100, 220, 255); // cian claro

    {
        auto& g = graf3D.objeto.getGestor();
        g.setMaxPointsSeries(150);
        g.setGrosorSeries(1.5f);
        g.setAdelgazadoSeries(true);
        g.setDifuminadoSeries(true);
        for (int i = 0; i < NUM_SIM; ++i)
            g.agregarSerie("C" + std::to_string(i), paleta[i]);
    }


    // =========================================================================
    //  PROYECCION XZ -- la vista clasica del "doble scroll"
    //  Los dos lobulos simetricos son la imagen iconica del circuito de Chua.
    //  Cada lobulo corresponde a uno de los puntos de equilibrio P+ y P-.
    //  Los saltos XZ son el resultado de que h(x) cambia de pendiente en |x|=1.
    // =========================================================================
    dsv::Vista<dsv::EspacioFase2D> proyXZ =
        tablero.add<dsv::EspacioFase2D>(
            "Proyeccion XZ -- DOBLE SCROLL: dos lobulos simetricos P+/P-",
            dsv::Color::cian, "XZ");
    proyXZ->activarAutoescalado(false);
    proyXZ->configurarLimites(-3.5f, 3.5f, -4.5f, 4.5f);
    proyXZ->ponerDesvanecido(false);
    proyXZ->ponerCabeza(false);
    proyXZ->configurarMaxPoints(12000);
    for (int i = 0; i < NUM_SIM; ++i)
        proyXZ->agregarSerie("C" + std::to_string(i), paleta[i]);

    // =========================================================================
    //  PROYECCION XY -- la estructura de bandas del diodo de Chua
    //  Las 3 regiones de h(x) crean 3 "zonas" en el plano XY con
    //  comportamiento cualitativamente diferente.
    // =========================================================================
    dsv::Vista<dsv::EspacioFase2D> proyXY =
        tablero.add<dsv::EspacioFase2D>(
            "Proyeccion XY -- zonas del diodo (3 regiones de h(x))",
            dsv::Color::azul, "XY");
    proyXY->activarAutoescalado(false);
    proyXY->configurarLimites(-3.5f, 3.5f, -0.5f, 0.5f);
    proyXY->ponerDesvanecido(false);
    proyXY->ponerCabeza(false);
    proyXY->configurarMaxPoints(12000);
    for (int i = 0; i < NUM_SIM; ++i)
        proyXY->agregarSerie("C" + std::to_string(i), paleta[i]);

    // =========================================================================
    //  PROYECCION YZ
    // =========================================================================
    dsv::Vista<dsv::EspacioFase2D> proyYZ =
        tablero.add<dsv::EspacioFase2D>(
            "Proyeccion YZ",
            dsv::Color::celeste, "YZ");
    proyYZ->activarAutoescalado(false);
    proyYZ->configurarLimites(-0.5f, 0.5f, -4.5f, 4.5f);
    proyYZ->ponerDesvanecido(false);
    proyYZ->ponerCabeza(false);
    proyYZ->configurarMaxPoints(12000);
    for (int i = 0; i < NUM_SIM; ++i)
        proyYZ->agregarSerie("C" + std::to_string(i), paleta[i]);


    // =========================================================================
    //  TIEMPO Z(t) -- la corriente del inductor
    //  En el circuito fisico, z es la corriente que oscila entre los dos estados.
    //  Los picos positivos = lobulo P+, negativos = lobulo P-.
    //  La secuencia de lobulos es aperiodica: caos electrico medible.
    // =========================================================================
    dsv::Vista<dsv::GraficaTiempo> grafZt =
        tablero.add<dsv::GraficaTiempo>(
            "Z(t) = corriente del inductor -- secuencia caotica de lobulos",
            dsv::Color::cian, "zt");
    grafZt.objeto.configurarVentanaTiempo(50.0f);
    grafZt.objeto.ponerSombreado(false);
    for (int i = 0; i < NUM_SIM; ++i)
        grafZt->agregarSerie("Z" + std::to_string(i), paleta[i]);

    tablero.setPanelDegradado(sf::Color(3, 8, 20), sf::Color(2, 4, 10));


    // =========================================================================
    //  PANEL PARAMETROS (derecha)
    //  alpha y beta controlan las velocidades de los nodos C1, C2 y L.
    //  m0 y m1 controlan la forma de h(x) -- el "diodo" virtual.
    // =========================================================================
    dsv::PanelFlotante PanelParams(
        window, "Chua",
        dsv::HandleOrientacion::Vertical,
        dsv::DespliegueDir::Izq,
        dsv::Color::cian
    );
    PanelParams.setDegradado(sf::Color(5, 20, 45, 245), sf::Color(3, 10, 22, 245));
    PanelParams.positionAbsoluta(dsv::Ubicacion::CentroDer);

    dsv::MenuFlotante* menuP = PanelParams.crearContenido<dsv::MenuFlotante>(6.f, 14.f);

    dsv::CampoControl& pp_t = menuP->agregarFila();
    pp_t.agregar<dsv::CampoTexto>("-- Parametros del Circuito --");

    // alpha = C2/C1: relacion de condensadores. Mayor alpha -> dinamica mas rapida en x
    dsv::CampoControl& pp_alpha = menuP->agregarFila();
    pp_alpha.agregar<dsv::CampoDeslizador>("alpha  C2/C1", &modeloBase.alpha, 5.0f, 25.0f);

    // beta = C2/(L*G^2): relacion condensador/inductor
    dsv::CampoControl& pp_beta = menuP->agregarFila();
    pp_beta.agregar<dsv::CampoDeslizador>("beta   C2/LG2", &modeloBase.beta, 10.0f, 50.0f);

    dsv::CampoControl& pp_sep1 = menuP->agregarFila();
    pp_sep1.agregar<dsv::CampoTexto>("-- Diodo de Chua h(x) --");

    // m0: pendiente de h(x) en la region EXTERIOR |x|>1 (debe ser negativo para caos)
    dsv::CampoControl& pp_m0 = menuP->agregarFila();
    pp_m0.agregar<dsv::CampoDeslizador>("m0  Pendiente ext", &modeloBase.m0, -2.0f, 0.0f);

    // m1: pendiente de h(x) en la region INTERIOR |x|<1 (debe ser negativo)
    dsv::CampoControl& pp_m1 = menuP->agregarFila();
    pp_m1.agregar<dsv::CampoDeslizador>("m1  Pendiente int", &modeloBase.m1, -2.0f, 0.0f);

    dsv::CampoControl& pp_sep2 = menuP->agregarFila();
    pp_sep2.agregar<dsv::CampoTexto>("-- Presets --");

    // Preset clasico "Doble Scroll"
    dsv::CampoControl& pp_ds = menuP->agregarFila();
    pp_ds.agregar<dsv::CampoBoton>("Doble Scroll", [&]{
        modeloBase.alpha = 15.6f; modeloBase.beta = 28.0f;
        modeloBase.m0 = -1.143f; modeloBase.m1 = -0.714f;
    }, dsv::Color::cian);

    // Preset "Scroll Simple" (solo un lobulo activo)
    dsv::CampoControl& pp_ss = menuP->agregarFila();
    pp_ss.agregar<dsv::CampoBoton>("Scroll Simple", [&]{
        modeloBase.alpha = 9.0f; modeloBase.beta = 14.286f;
        modeloBase.m0 = -1.143f; modeloBase.m1 = -0.714f;
    }, dsv::Color::azul);

    // Preset periodico (sin caos)
    dsv::CampoControl& pp_per = menuP->agregarFila();
    pp_per.agregar<dsv::CampoBoton>("Periodico", [&]{
        modeloBase.alpha = 8.0f; modeloBase.beta = 14.0f;
        modeloBase.m0 = -1.143f; modeloBase.m1 = -0.714f;
    }, dsv::Color::verde);

    dsv::CampoControl& pp_res = menuP->agregarFila();
    pp_res.agregar<dsv::CampoBoton>("Reset Params", [&]{
        modeloBase.alpha = 15.6f; modeloBase.beta = 28.0f;
        modeloBase.m0 = -1.143f; modeloBase.m1 = -0.714f;
    }, dsv::Color::celeste);


    // =========================================================================
    //  PANEL CI (izquierda)
    // =========================================================================
    dsv::PanelFlotante PanelCI(
        window, "CI",
        dsv::HandleOrientacion::Vertical,
        dsv::DespliegueDir::Der,
        dsv::Color::azul
    );
    PanelCI.setDegradado(sf::Color(5, 15, 40, 245), sf::Color(3, 8, 20, 245));
    PanelCI.positionAbsoluta(dsv::Ubicacion::CentroIzq);

    dsv::MenuFlotante* menuCI = PanelCI.crearContenido<dsv::MenuFlotante>(6.f, 14.f);

    dsv::CampoControl& ci_t = menuCI->agregarFila();
    ci_t.agregar<dsv::CampoTexto>("-- Condiciones Iniciales --");

    // x = voltaje C1 inicial
    dsv::CampoControl& ci_x = menuCI->agregarFila();
    ci_x.agregar<dsv::CampoDeslizador>("x0 (V_C1)", &x0_base, -3.0f, 3.0f);

    // y = voltaje C2 inicial
    dsv::CampoControl& ci_y = menuCI->agregarFila();
    ci_y.agregar<dsv::CampoDeslizador>("y0 (V_C2)", &y0_base, -0.4f, 0.4f);

    // z = corriente L inicial
    dsv::CampoControl& ci_z = menuCI->agregarFila();
    ci_z.agregar<dsv::CampoDeslizador>("z0 (I_L)",  &z0_base, -4.0f, 4.0f);

    dsv::CampoControl& ci_d = menuCI->agregarFila();
    ci_d.agregar<dsv::CampoDeslizador>("delta CI", &delta_ci, 0.0f, 0.5f);

    dsv::CampoControl& ci_sep = menuCI->agregarFila();
    ci_sep.agregar<dsv::CampoTexto>("-- h(x) actual (diodo) --");

    // Mostrar el valor de h(x) en el punto x[0] de la primera instancia
    // h(x) = m1*x + 0.5*(m0-m1)*(|x+1| - |x-1|)
    float hx_display = 0.0f;
    dsv::CampoControl& ci_hx = menuCI->agregarFila();
    ci_hx.agregar<dsv::CampoVariable>("h(x[0]) =", &hx_display);

    dsv::CampoControl& ci_sep2 = menuCI->agregarFila();
    ci_sep2.agregar<dsv::CampoTexto>("-- Estado [0] --");
    dsv::CampoControl& ci_v0 = menuCI->agregarFila();
    ci_v0.agregar<dsv::CampoVariable>("x =", &sims[0].state[0]);
    dsv::CampoControl& ci_v1 = menuCI->agregarFila();
    ci_v1.agregar<dsv::CampoVariable>("y =", &sims[0].state[1]);
    dsv::CampoControl& ci_v2 = menuCI->agregarFila();
    ci_v2.agregar<dsv::CampoVariable>("z =", &sims[0].state[2]);

    dsv::CampoControl& ci_res = menuCI->agregarFila();
    ci_res.agregar<dsv::CampoBoton>("Reset CI [R]",
        [&]{ initSims(); }, dsv::Color::azul);


    // =========================================================================
    //  PANEL TIEMPO
    // =========================================================================
    bool  pausa     = true;
    float timeScale = 1.0f;
    sf::Clock fpsClock;
    long long frameCount = 0;
    float fps = 0.0f, tiempo = 0.0f;

    const sf::Color colorMENU = dsv::Color::cian;

    dsv::PanelFlotante PanelTiempo(
        window, "Menu",
        dsv::HandleOrientacion::Horizontal,
        dsv::DespliegueDir::Abajo,
        colorMENU
    );
    PanelTiempo.setDegradado(sf::Color(5, 20, 45, 230), sf::Color(3, 10, 22, 230));
    PanelTiempo.positionAbsoluta(dsv::Ubicacion::ArribaCentro);

    dsv::MenuFlotante* menuT = PanelTiempo.crearContenido<dsv::MenuFlotante>(6.f, 12.f);
    dsv::CampoControl& mt1 = menuT->agregarFila();
    mt1.agregar<dsv::CampoTexto>("-- Chua: Doble Scroll Electrico --");
    dsv::CampoControl& mt2 = menuT->agregarFila();
    mt2.agregar<dsv::CampoVariable>("fps", &fps);
    mt2.agregar<dsv::CampoVariable>("t =", &tiempo);
    dsv::CampoControl& mt3 = menuT->agregarFila();
    mt3.agregar<dsv::CampoBoton>("<<", [&]{ timeScale = std::max(0.1f, timeScale-0.1f); }, colorMENU);
    mt3.agregar<dsv::CampoToggleTexto>("", &pausa, "Pausa", "Play", colorMENU);
    mt3.agregar<dsv::CampoBoton>(">>", [&]{ timeScale += 0.1f; }, colorMENU);
    dsv::CampoControl& mt4 = menuT->agregarFila();
    mt4.agregar<dsv::CampoVariable>("Speed x", &timeScale);
    dsv::CampoControl& mt5 = menuT->agregarFila();
    mt5.agregar<dsv::CampoBoton>("Reset Vel", [&]{ timeScale = 1.0f; });
    mt5.agregar<dsv::CampoToggle>("Pausa", &pausa, colorMENU);
    dsv::CampoControl& mt6 = menuT->agregarFila();
    mt6.agregar<dsv::CampoBoton>("Reset [R]", [&]{ initSims(); tiempo = 0.0f; }, colorMENU);


    // =========================================================================
    //  BUCLE PRINCIPAL
    // =========================================================================
    sf::Clock clock;
    sf::Time  accumulator = sf::Time::Zero;
    // ups = 0.001f: Chua requiere pasos pequenos para la funcion h(x)
    // La discontinuidad de derivada en |x|=1 necesita pasos finos con RK4.
    sf::Time  ups = sf::seconds(0.001f);

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window.close();
            PanelTiempo.gestionarEvento(event);
            PanelParams.gestionarEvento(event);
            PanelCI.gestionarEvento(event);
            graf3D->gestionarEvento(event, window);
            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Space) pausa = !pausa;
                if (event.key.code == sf::Keyboard::Up)    timeScale += 0.2f;
                if (event.key.code == sf::Keyboard::Down)  timeScale = std::max(0.1f, timeScale-0.2f);
                if (event.key.code == sf::Keyboard::R)   { initSims(); tiempo = 0.0f; }
            }
        }

        // h(x) en vivo para la primera instancia (diagnostico del diodo)
        {
            float x = sims[0].state[0];
            hx_display = modeloBase.m1 * x
                       + 0.5f * (modeloBase.m0 - modeloBase.m1)
                         * (std::abs(x + 1.0f) - std::abs(x - 1.0f));
        }

        sf::Time elapsed = clock.restart();
        if (!pausa) accumulator += elapsed * timeScale;

        while (accumulator >= ups) {
            // RK4 para Chua: la funcion h(x) tiene discontinuidad de derivada
            // en x = +/-1. RK4 no es exacto en discontinuidades pero con
            // dt=0.001 la aproximacion es excelente (error < 1e-8 por paso).
            for (auto& s : sims) dsv::sim::RK4_step(s, ups.asSeconds());
            tiempo = sims[0].t;
            accumulator -= ups;
        }

        if (!pausa) {
            for (int i = 0; i < NUM_SIM; ++i) {
                const std::string id = "C" + std::to_string(i);
                float x = sims[i].state[0];
                float y = sims[i].state[1];
                float z = sims[i].state[2];
                float t = sims[i].t;
                graf3D.objeto.getGestor().push_back({x, y, z}, id);
                proyXZ->push_back(x, z, id);
                proyXY->push_back(x, y, id);
                proyYZ->push_back(y, z, id);
                grafZt->push_back(z, t, "Z" + std::to_string(i));
            }
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
