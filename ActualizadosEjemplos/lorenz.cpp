/*
 * proyecto: DynSysVis RT -- Atractor de Lorenz (La Mariposa)
 *
 * El sistema de Lorenz fue originalmente un modelo de conveccion atmosferica
 * simplificado (Lorenz, 1963). Su importancia historica es que fue el primer
 * ejemplo riguroso de caos deterministico: un sistema de solo 3 ecuaciones
 * no lineales con solucion no periodica y sensibilidad exponencial a CI.
 *
 * Ecuaciones:
 *   dx/dt = sigma * (y - x)            -- conveccion termica
 *   dy/dt = x * (rho - z) - y          -- calentamiento diferencial
 *   dz/dt = x * y - beta * z           -- disipacion vertical
 *
 * Parametros clasicos de Lorenz (1963):
 *   sigma = 10    (numero de Prandtl)
 *   rho   = 28    (numero de Rayleigh reducido)
 *   beta  = 8/3   (aspecto geometrico del dominio de conveccion)
 *   Con estos valores: atractor caotico con dos lobulos ("alas de mariposa").
 *
 * Puntos fijos del sistema:
 *   C0 = (0, 0, 0)                             -- inestable siempre
 *   C+/- = (+/-sqrt(beta*(rho-1)), ...)         -- estables si rho < 24.74
 *   Para rho=28 > 24.74: C+/- son inestables -> caos
 *
 * Exponente de Lyapunov maxima (parametros clasicos): lambda_1 ~= 0.906 [1/s]
 *   Tiempo de duplicacion: t_d = ln(2)/lambda_1 ~= 0.76 s
 *   10 trayectorias con delta=1e-4 seran visiblemente distintas en ~10 s.
 *
 * Topologia del atractor:
 *   El "estiramiento y plegado" del espacio de fase es el mecanismo del caos.
 *   Las trayectorias se estiran (divergen), se doblan y se comprimen
 *   indefinidamente sin repetirse y sin salir del atractor compacto.
 *   Dimension fractal de Lyapunov (Kaplan-Yorke): ~2.06 (casi 2D, un poco mas).
 *
 * Observacion clave en la proyeccion XZ:
 *   Los "saltos" entre lobulos son impredecibles individualmente pero
 *   estadisticamente regulares. La frecuencia de salto aumenta con rho.
 */

#include <SFML/Graphics.hpp>
#include <cmath>
#include <algorithm>
#include <string>
#include <vector>

#include "DynSysVis.hpp"


// =============================================================================
//  CONSTANTES
// =============================================================================
static constexpr int   NUM_SIM  = 15;
static constexpr float DELTA_CI = 1e-4f;  // perturbacion entre instancias

// Condicion inicial base -- punto cercano al atractor pero no en el
static constexpr float X0 = 0.1f;
static constexpr float Y0 = 0.0f;
static constexpr float Z0 = 0.0f;


// =============================================================================
//  MAIN
// =============================================================================
int main() {

    dsv::Color::cargar("assets/config/colores.txt");
    sf::RenderWindow window;
    dsv::Sistema::inicializarVentana(window, "DynSysVis RT -- Atractor de Lorenz");


    // =========================================================================
    //  MODELO BASE
    // =========================================================================
    dsv::mod::Lorenz_Model modeloBase;
    modeloBase.sigma = 10.0f;
    modeloBase.rho   = 28.0f;
    modeloBase.beta  = 8.0f / 3.0f;

    // Variables puente para los sliders (permiten reset coherente)
    float x0_base = X0, y0_base = Y0, z0_base = Z0;
    float delta_ci = DELTA_CI;


    // =========================================================================
    //  10 INSTANCIAS con perturbacion en x0
    //  La variacion es DELTA_CI = 1e-4 en x: invisible en espacio real pero
    //  el caos la amplifica hasta ser macroscopica en ~10 segundos.
    // =========================================================================
    using LorenzInst = dsv::mod::Instance<dsv::mod::Lorenz_Model>;
    std::vector<LorenzInst> sims(NUM_SIM);

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
    //  PALETA CYBERPUNK -- colores electricos para el atractor mas famoso
    // =========================================================================
    std::vector<sf::Color> paleta(NUM_SIM);
    for (int i = 0; i < NUM_SIM; ++i)
        paleta[i] = dsv::Color::Cyberpunk(i, NUM_SIM);


    // =========================================================================
    //  LAYOUT
    //
    //  +----------+----------+----------+----------+
    //  |          |          |          |          |
    //  |  3D      |  3D      |  XY      |  XZ      |
    //  |  (grande)|          |  (fase)  |  (fase)  |
    //  |          |          |          |          |
    //  +----------+----------+----------+----------+
    //  |  3D      |  3D      |  YZ      |  Z(t)    |
    //  |          |          |  (fase)  |  (tiempo)|
    //  +----------+----------+----------+----------+
    // =========================================================================
    dsv::Layout miLayout = {
        "3d 3d 3d 3d XY XY XZ XZ",
        "3d 3d 3d 3d XY XY XZ XZ",
        "3d 3d 3d 3d YZ YZ zt zt",
        "3d 3d 3d 3d YZ YZ zt zt",
    };
    dsv::Tablero tablero(window, miLayout,
                         sf::Color(8, 5, 18), sf::Color(4, 2, 9));


    // =========================================================================
    //  GRAFICA 3D -- el atractor completo con estelas dinamicas
    // =========================================================================
    dsv::Vista<dsv::Grafica3D> graf3D =
        tablero.add<dsv::Grafica3D>(
            "Atractor de Lorenz -- 10 trayectorias Cyberpunk",
            dsv::Color::violeta, "3d");

    graf3D.objeto.getEjes().setLimites(-25.0f, 25.0f,
                                       -30.0f, 30.0f,
                                        0.0f,  55.0f);
    graf3D.objeto.getEjes().colorX = sf::Color(255, 50, 150);  // magenta
    graf3D.objeto.getEjes().colorY = sf::Color(50, 200, 255);  // cian
    graf3D.objeto.getEjes().colorZ = sf::Color(180, 50, 255);  // violeta

    {
        auto& g = graf3D.objeto.getGestor();
        g.setMaxPointsSeries(150);    // estelas cortas: ver el movimiento
        g.setGrosorSeries(1.5f);
        g.setAdelgazadoSeries(true);  // grosor decrece en la cola
        g.setDifuminadoSeries(true);  // alpha decrece en la cola
        for (int i = 0; i < NUM_SIM; ++i)
            g.agregarSerie("L" + std::to_string(i), paleta[i]);
    }


    // =========================================================================
    //  PROYECCIONES 2D -- "sombras" del atractor en los 3 planos canonicos
    //
    //  XY: muestra los dos lobulos de la mariposa (vista desde arriba en Z)
    //  XZ: muestra los "saltos" entre lobulos (el movimiento caotico mas visible)
    //  YZ: vista lateral (menos informativa pero completa el analisis 3D)
    //
    //  Sin desvanecido y con muchos puntos: el atractor llena el espacio
    //  gradualmente revelando su estructura fractal.
    // =========================================================================

    dsv::Vista<dsv::EspacioFase2D> proyXY =
        tablero.add<dsv::EspacioFase2D>(
            "Proyeccion XY -- las dos alas de la mariposa",
            dsv::Color::violeta, "XY");
    proyXY->activarAutoescalado(false);
    proyXY->configurarLimites(-25.0f, 25.0f, -30.0f, 30.0f);
    proyXY->ponerDesvanecido(false);
    proyXY->ponerCabeza(false);
    proyXY->configurarMaxPoints(400);
    for (int i = 0; i < NUM_SIM; ++i)
        proyXY->agregarSerie("L" + std::to_string(i), paleta[i]);

    dsv::Vista<dsv::EspacioFase2D> proyXZ =
        tablero.add<dsv::EspacioFase2D>(
            "Proyeccion XZ -- saltos entre lobulos (caos visible)",
            dsv::Color::magenta, "XZ");
    proyXZ->activarAutoescalado(false);
    proyXZ->configurarLimites(-25.0f, 25.0f, 0.0f, 55.0f);
    proyXZ->ponerDesvanecido(false);
    proyXZ->ponerCabeza(false);
    proyXZ->configurarMaxPoints(400);
    for (int i = 0; i < NUM_SIM; ++i)
        proyXZ->agregarSerie("L" + std::to_string(i), paleta[i]);

    dsv::Vista<dsv::EspacioFase2D> proyYZ =
        tablero.add<dsv::EspacioFase2D>(
            "Proyeccion YZ -- vista lateral del atractor",
            dsv::Color::cian, "YZ");
    proyYZ->activarAutoescalado(false);
    proyYZ->configurarLimites(-30.0f, 30.0f, 0.0f, 55.0f);
    proyYZ->ponerDesvanecido(false);
    proyYZ->ponerCabeza(false);
    proyYZ->configurarMaxPoints(400);
    for (int i = 0; i < NUM_SIM; ++i)
        proyYZ->agregarSerie("L" + std::to_string(i), paleta[i]);


    // =========================================================================
    //  TIEMPO Z(t) -- la divergencia mas clara de las 10 trayectorias
    //  Z(t) muestra los "picos" asociados a cada salto entre lobulos.
    //  Las 10 curvas comienzan identicas y se desfasan exponencialmente.
    // =========================================================================
    dsv::Vista<dsv::GraficaTiempo> grafZt =
        tablero.add<dsv::GraficaTiempo>(
            "Z(t) -- divergencia exponencial (10 trayectorias)",
            dsv::Color::violeta, "zt");
    grafZt.objeto.configurarVentanaTiempo(30.0f);
    grafZt.objeto.ponerSombreado(false);
    for (int i = 0; i < NUM_SIM; ++i)
        grafZt->agregarSerie("Z" + std::to_string(i), paleta[i]);

    tablero.setPanelDegradado(sf::Color(8, 5, 18), sf::Color(4, 2, 9));


    // =========================================================================
    //  PANEL PARAMETROS (derecha)
    // =========================================================================
    dsv::PanelFlotante PanelParams(
        window, "Lorenz",
        dsv::HandleOrientacion::Vertical,
        dsv::DespliegueDir::Izq,
        dsv::Color::violeta
    );
    PanelParams.setDegradado(sf::Color(20, 8, 40, 245), sf::Color(10, 4, 20, 245));
    PanelParams.positionAbsoluta(dsv::Ubicacion::CentroDer);

    dsv::MenuFlotante* menuP = PanelParams.crearContenido<dsv::MenuFlotante>(6.f, 14.f);

    dsv::CampoControl& pp_t = menuP->agregarFila();
    pp_t.agregar<dsv::CampoTexto>("-- Parametros de Lorenz --");

    // sigma: velocidad de conveccion. Aumentar -> atractor mas grande en XY
    dsv::CampoControl& pp_sigma = menuP->agregarFila();
    pp_sigma.agregar<dsv::CampoDeslizador>("sigma  Conveccion", &modeloBase.sigma, 1.0f, 20.0f);

    // rho: forzamiento termico. rho < 24.74: espirales estables. rho > 24.74: caos
    dsv::CampoControl& pp_rho = menuP->agregarFila();
    pp_rho.agregar<dsv::CampoDeslizador>("rho    Forzamiento", &modeloBase.rho,   1.0f, 50.0f);

    // beta: disipacion geometrica. beta < 2: colapso rapido. beta > 4: expansion
    dsv::CampoControl& pp_beta = menuP->agregarFila();
    pp_beta.agregar<dsv::CampoDeslizador>("beta   Disipacion",  &modeloBase.beta,  0.1f, 6.0f);

    // Indicador de regimen
    float rho_critico = 0.0f;
    dsv::CampoControl& pp_rc = menuP->agregarFila();
    pp_rc.agregar<dsv::CampoVariable>("rho_crit = 24.74", &rho_critico);

    dsv::CampoControl& pp_sep = menuP->agregarFila();
    pp_sep.agregar<dsv::CampoTexto>("-- Puntos Fijos C+/- --");

    float C_plus = 0.0f;
    dsv::CampoControl& pp_cp = menuP->agregarFila();
    pp_cp.agregar<dsv::CampoVariable>("C+/- x = sqrt(b*(r-1))", &C_plus);

    dsv::CampoControl& pp_res = menuP->agregarFila();
    pp_res.agregar<dsv::CampoBoton>("Reset Params", [&]{
        modeloBase.sigma = 10.0f;
        modeloBase.rho   = 28.0f;
        modeloBase.beta  = 8.0f / 3.0f;
    }, dsv::Color::violeta);


    // =========================================================================
    //  PANEL CONDICIONES INICIALES (izquierda)
    // =========================================================================
    dsv::PanelFlotante PanelCI(
        window, "CI",
        dsv::HandleOrientacion::Vertical,
        dsv::DespliegueDir::Der,
        dsv::Color::magenta
    );
    PanelCI.setDegradado(sf::Color(35, 5, 35, 245), sf::Color(18, 3, 18, 245));
    PanelCI.positionAbsoluta(dsv::Ubicacion::CentroIzq);

    dsv::MenuFlotante* menuCI = PanelCI.crearContenido<dsv::MenuFlotante>(6.f, 14.f);

    dsv::CampoControl& ci_t = menuCI->agregarFila();
    ci_t.agregar<dsv::CampoTexto>("-- Condiciones Iniciales --");

    dsv::CampoControl& ci_x = menuCI->agregarFila();
    ci_x.agregar<dsv::CampoDeslizador>("x0  base", &x0_base, -20.0f, 20.0f);

    dsv::CampoControl& ci_y = menuCI->agregarFila();
    ci_y.agregar<dsv::CampoDeslizador>("y0  base", &y0_base, -30.0f, 30.0f);

    dsv::CampoControl& ci_z = menuCI->agregarFila();
    ci_z.agregar<dsv::CampoDeslizador>("z0  base", &z0_base,   0.0f, 50.0f);

    dsv::CampoControl& ci_d = menuCI->agregarFila();
    ci_d.agregar<dsv::CampoDeslizador>("delta CI", &delta_ci, 0.0f, 1.0f);

    dsv::CampoControl& ci_sep = menuCI->agregarFila();
    ci_sep.agregar<dsv::CampoTexto>("-- Estado actual [0] --");

    dsv::CampoControl& ci_cx = menuCI->agregarFila();
    ci_cx.agregar<dsv::CampoVariable>("x[0] =", &sims[0].state[0]);
    dsv::CampoControl& ci_cy = menuCI->agregarFila();
    ci_cy.agregar<dsv::CampoVariable>("y[0] =", &sims[0].state[1]);
    dsv::CampoControl& ci_cz = menuCI->agregarFila();
    ci_cz.agregar<dsv::CampoVariable>("z[0] =", &sims[0].state[2]);

    dsv::CampoControl& ci_res = menuCI->agregarFila();
    ci_res.agregar<dsv::CampoBoton>("Reset CI [R]",
        [&]{ initSims(); }, dsv::Color::magenta);


    // =========================================================================
    //  PANEL TIEMPO (arriba centro)
    // =========================================================================
    bool  pausa     = true;
    float timeScale = 1.0f;
    sf::Clock fpsClock;
    long long frameCount = 0;
    float fps    = 0.0f;
    float tiempo = 0.0f;

    const sf::Color colorMENU = dsv::Color::violeta_l;

    dsv::PanelFlotante PanelTiempo(
        window, "Menu",
        dsv::HandleOrientacion::Horizontal,
        dsv::DespliegueDir::Abajo,
        colorMENU
    );
    PanelTiempo.setDegradado(sf::Color(25, 10, 50, 230), sf::Color(12, 5, 25, 230));
    PanelTiempo.positionAbsoluta(dsv::Ubicacion::ArribaCentro);

    dsv::MenuFlotante* menuT = PanelTiempo.crearContenido<dsv::MenuFlotante>(6.f, 12.f);

    dsv::CampoControl& mt_f1 = menuT->agregarFila();
    mt_f1.agregar<dsv::CampoTexto>("-- Lorenz: Atractor de la Mariposa --");

    dsv::CampoControl& mt_f2 = menuT->agregarFila();
    mt_f2.agregar<dsv::CampoVariable>("fps", &fps);
    mt_f2.agregar<dsv::CampoVariable>("t =", &tiempo);

    dsv::CampoControl& mt_f3 = menuT->agregarFila();
    mt_f3.agregar<dsv::CampoBoton>("<<",
        [&]{ timeScale = std::max(0.1f, timeScale - 0.1f); }, colorMENU);
    mt_f3.agregar<dsv::CampoToggleTexto>("", &pausa, "Pausa", "Play", colorMENU);
    mt_f3.agregar<dsv::CampoBoton>(">>", [&]{ timeScale += 0.1f; }, colorMENU);

    dsv::CampoControl& mt_f4 = menuT->agregarFila();
    mt_f4.agregar<dsv::CampoVariable>("Speed x", &timeScale);

    dsv::CampoControl& mt_f5 = menuT->agregarFila();
    mt_f5.agregar<dsv::CampoBoton>("Reset Vel", [&]{ timeScale = 1.0f; });
    mt_f5.agregar<dsv::CampoToggle>("Pausa", &pausa, colorMENU);

    dsv::CampoControl& mt_f6 = menuT->agregarFila();
    mt_f6.agregar<dsv::CampoBoton>("Reset [R]",
        [&]{ initSims(); tiempo = 0.0f; }, colorMENU);


    // =========================================================================
    //  BUCLE PRINCIPAL
    // =========================================================================
    sf::Clock clock;
    sf::Time  accumulator = sf::Time::Zero;
    sf::Time  ups = sf::seconds(0.002f);

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
                if (event.key.code == sf::Keyboard::Down)  timeScale = std::max(0.1f, timeScale - 0.2f);
                if (event.key.code == sf::Keyboard::R)   { initSims(); tiempo = 0.0f; }
            }
        }

        // Metricas en vivo
        rho_critico = 24.74f;   // valor fijo, solo referencia
        float val_rho_m1 = modeloBase.rho - 1.0f;
        C_plus = (val_rho_m1 > 0.0f)
               ? std::sqrt(modeloBase.beta * val_rho_m1)
               : 0.0f;

        sf::Time elapsed = clock.restart();
        if (!pausa) accumulator += elapsed * timeScale;

        while (accumulator >= ups) {
            float dt = ups.asSeconds();
            // RK4: necesario para preservar la topologia del atractor.
            // Euler introduce un "engrosamiento" artificial de las orbitas.
            for (auto& s : sims) dsv::sim::RK4_step(s, dt);
            tiempo = sims[0].t;
            accumulator -= ups;
        }

        if (!pausa) {
            for (int i = 0; i < NUM_SIM; ++i) {
                const std::string id = "L" + std::to_string(i);
                float x = sims[i].state[0];
                float y = sims[i].state[1];
                float z = sims[i].state[2];
                float t = sims[i].t;

                graf3D.objeto.getGestor().push_back({x, y, z}, id);
                proyXY->push_back(x, y, id);
                proyXZ->push_back(x, z, id);
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
        PanelTiempo.draw();
        PanelParams.draw();
        PanelCI.draw();
        window.display();
    }

    return 0;
}
