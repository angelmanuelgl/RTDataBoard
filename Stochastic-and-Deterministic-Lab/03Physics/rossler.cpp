/*
 * proyecto: DynSysVis RT -- Atractor de Rossler
 *
 * El sistema de Rossler (1976) fue disenado como la version minimalista
 * del caos: una sola no-linealidad (x[2]*x[0]) frente a las dos de Lorenz.
 * Esto lo hace mas facil de analizar matematicamente y visualmente mas limpio.
 *
 * Ecuaciones:
 *   dx/dt = -y - z
 *   dy/dt =  x + a*y
 *   dz/dt =  b + z*(x - c)
 *
 * La unica no-linealidad esta en dz/dt: el termino z*x.
 * Cuando x < c, z tiende a 0 (espiral en XY).
 * Cuando x > c, z "explota" hacia arriba (salto en Z).
 * El sistema baja de nuevo cuando z crece y arrastra x hacia valores menores.
 *
 * Parametros tipicos (c es el mas importante):
 *   a = 0.1, b = 0.1, c = 14.0: atractor caotico bien desarrollado
 *   c < 4.0: orbitas periodicas (no caotico)
 *   c ~ 4.2: primera bifurcacion (periodo 2)
 *   c ~ 6.0: cascada de bifurcaciones hacia el caos
 *   c > 12:  caos completamente desarrollado con "bandas"
 *
 * Topologia del atractor de Rossler:
 *   En la proyeccion XY se ve como una espiral plana que se ENROLLA
 *   hacia el exterior. Periodicamente un "salto" en Z interrumpe la espiral,
 *   la trayectoria regresa desde arriba y reanuda el enroll desde un punto
 *   ligeramente distinto. Esta mezcla crea el "plegado" caotico.
 *   A diferencia de Lorenz (dos lobulos simetricos), Rossler tiene UN solo
 *   lobulo con estructura de banda.
 *
 * Dimension fractal: ~2.01 (casi plano, mas simple que Lorenz ~2.06)
 * Exponente de Lyapunov (c=14): lambda_1 ~= 0.13 [1/s] (mas lento que Lorenz)
 */

#include <SFML/Graphics.hpp>
#include <cmath>
#include <algorithm>
#include <string>
#include <vector>

#include "DynSysVis.hpp"
// dsv::mod::Rossler_Model ya declarado en la libreria


static constexpr int   NUM_SIM  = 10;
static constexpr float DELTA_CI = 1e-4f;
static constexpr float X0 =  0.1f;
static constexpr float Y0 =  0.0f;
static constexpr float Z0 =  0.0f;


int main() {

    dsv::Color::cargar("assets/config/colores.txt");
    sf::RenderWindow window;
    dsv::Sistema::inicializarVentana(window, "DynSysVis RT -- Atractor de Rossler");


    // =========================================================================
    //  MODELO BASE
    // =========================================================================
    dsv::mod::Rossler_Model modeloBase;
    modeloBase.a = 0.1f;
    modeloBase.b = 0.1f;
    modeloBase.c = 14.0f;

    float x0_base = X0, y0_base = Y0, z0_base = Z0;
    float delta_ci = DELTA_CI;


    // =========================================================================
    //  10 INSTANCIAS -- perturbacion en x0
    // =========================================================================
    using RosslerInst = dsv::mod::Instance<dsv::mod::Rossler_Model>;
    std::vector<RosslerInst> sims(NUM_SIM);

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
    //  PALETA MAGMA -- de negro/morado a amarillo/blanco (calor y tension)
    // =========================================================================
    std::vector<sf::Color> paleta(NUM_SIM);
    for (int i = 0; i < NUM_SIM; ++i)
        paleta[i] = dsv::Color::Magma(i, NUM_SIM);


    // =========================================================================
    //  LAYOUT -- Rossler tiene proyeccion XY especialmente importante
    //
    //  La espiral en XY es la "firma visual" del atractor de Rossler.
    //  Se le da mas espacio que a XZ y YZ.
    // =========================================================================
    dsv::Layout miLayout = {
        "3d 3d 3d XY XY XY XZ XZ",
        "3d 3d 3d XY XY XY XZ XZ",
        "3d 3d 3d XY XY XY YZ YZ",
        "3d 3d 3d zt zt zt YZ YZ",
    };
    dsv::Tablero tablero(window, miLayout,
                         sf::Color(12, 5, 5), sf::Color(6, 2, 2));


    // =========================================================================
    //  GRAFICA 3D
    // =========================================================================
    dsv::Vista<dsv::Grafica3D> graf3D =
        tablero.add<dsv::Grafica3D>(
            "Atractor de Rossler -- 10 trayectorias Magma",
            dsv::Color::naranja, "3d");

    // Limites del atractor de Rossler con c=14
    // XY: [-12, 12],  Z: [0, 25] aprox
    graf3D.objeto.getEjes().setLimites(-15.0f, 15.0f,
                                       -15.0f, 15.0f,
                                         0.0f, 30.0f);
    graf3D.objeto.getEjes().colorX = sf::Color(255, 120, 20);  // naranja
    graf3D.objeto.getEjes().colorY = sf::Color(200, 80, 0);    // naranja oscuro
    graf3D.objeto.getEjes().colorZ = sf::Color(255, 200, 50);  // amarillo

    {
        auto& g = graf3D.objeto.getGestor();
        g.setMaxPointsSeries(150);
        g.setGrosorSeries(1.5f);
        g.setAdelgazadoSeries(true);
        g.setDifuminadoSeries(true);
        for (int i = 0; i < NUM_SIM; ++i)
            g.agregarSerie("R" + std::to_string(i), paleta[i]);
    }


    // =========================================================================
    //  PROYECCION XY -- la espiral de Rossler
    //  Esta es la vista mas icononica del atractor de Rossler.
    //  La espiral se ENROLLA desde el centro hacia afuera, luego salta en Z
    //  y regresa (el "plegado"). Sin desvanecido se ve como el atractor se
    //  "construye" banda por banda con el tiempo.
    // =========================================================================
    dsv::Vista<dsv::EspacioFase2D> proyXY =
        tablero.add<dsv::EspacioFase2D>(
            "Proyeccion XY -- espiral del atractor (la firma de Rossler)",
            dsv::Color::naranja, "XY");
    proyXY->activarAutoescalado(true);
    proyXY->configurarLimites(-5.0f, 5.0f, -5.0f, 5.0f);
    proyXY->ponerDesvanecido(false);
    proyXY->ponerCabeza(false);
    proyXY->configurarMaxPoints(1000);
    for (int i = 0; i < NUM_SIM; ++i)
        proyXY->agregarSerie("R" + std::to_string(i), paleta[i]);

    // =========================================================================
    //  PROYECCION XZ -- los "saltos" en Z
    //  Muestra los momentos en que x > c y z se dispara hacia arriba.
    //  La frecuencia de saltos es irregular: eso es el caos.
    // =========================================================================
    dsv::Vista<dsv::EspacioFase2D> proyXZ =
        tablero.add<dsv::EspacioFase2D>(
            "Proyeccion XZ -- saltos caoticos en Z (no-linealidad)",
            dsv::Color::rojo, "XZ");
    proyXZ->activarAutoescalado(true);
    proyXZ->configurarLimites(-5.0f, 5.0f, 0.0f, 10.0f);
    proyXZ->ponerDesvanecido(false);
    proyXZ->ponerCabeza(false);
    proyXZ->configurarMaxPoints(1000);
    for (int i = 0; i < NUM_SIM; ++i)
        proyXZ->agregarSerie("R" + std::to_string(i), paleta[i]);

    // =========================================================================
    //  PROYECCION YZ
    // =========================================================================
    dsv::Vista<dsv::EspacioFase2D> proyYZ =
        tablero.add<dsv::EspacioFase2D>(
            "Proyeccion YZ",
            dsv::Color::oro, "YZ");
    proyYZ->activarAutoescalado(true);
    proyYZ->configurarLimites(-5.0f, 5.0f, 0.0f, 10.0f);
    proyYZ->ponerDesvanecido(false);
    proyYZ->ponerCabeza(false);
    proyYZ->configurarMaxPoints(1000);
    for (int i = 0; i < NUM_SIM; ++i)
        proyYZ->agregarSerie("R" + std::to_string(i), paleta[i]);


    // =========================================================================
    //  TIEMPO Z(t) -- los saltos de Z revelan el caracter caotico
    //  Cada pico corresponde a una vuelta de la espiral que "salta".
    //  El tiempo entre picos es irregularmente irregular: eso es caos.
    // =========================================================================
    dsv::Vista<dsv::GraficaTiempo> grafZt =
        tablero.add<dsv::GraficaTiempo>(
            "Z(t) -- picos irregulares (saltos de la espiral)",
            dsv::Color::naranja, "zt");
    grafZt.objeto.configurarVentanaTiempo(100.0f);
    grafZt.objeto.ponerSombreado(false);
    for (int i = 0; i < NUM_SIM; ++i)
        grafZt->agregarSerie("Z" + std::to_string(i), paleta[i]);

    tablero.setPanelDegradado(sf::Color(12, 5, 5), sf::Color(6, 2, 2));


    // =========================================================================
    //  PANEL PARAMETROS (derecha) -- c es el mas importante
    // =========================================================================
    dsv::PanelFlotante PanelParams(
        window, "Rossler",
        dsv::HandleOrientacion::Vertical,
        dsv::DespliegueDir::Izq,
        dsv::Color::naranja
    );
    PanelParams.setDegradado(sf::Color(40, 15, 5, 245), sf::Color(20, 8, 3, 245));
    PanelParams.positionAbsoluta(dsv::Ubicacion::CentroDer);

    dsv::MenuFlotante* menuP = PanelParams.crearContenido<dsv::MenuFlotante>(6.f, 14.f);

    dsv::CampoControl& pp_t = menuP->agregarFila();
    pp_t.agregar<dsv::CampoTexto>("-- Parametros de Rossler --");

    // a: controla la velocidad de espiral en Y. a=0: sin espiral
    dsv::CampoControl& pp_a = menuP->agregarFila();
    pp_a.agregar<dsv::CampoDeslizador>("a  Espiral Y",   &modeloBase.a, 0.0f, 0.5f);

    // b: escala del origen fijo en Z. b=0: z=0 es fijo (sin saltos)
    dsv::CampoControl& pp_b = menuP->agregarFila();
    pp_b.agregar<dsv::CampoDeslizador>("b  Origen Z",    &modeloBase.b, 0.0f, 0.5f);

    // c: el parametro de caos. c < 4: periodico. c > 12: caos fuerte
    // Es el umbral de la no-linealidad: cuando x > c, z explota
    dsv::CampoControl& pp_c = menuP->agregarFila();
    pp_c.agregar<dsv::CampoDeslizador>("c  Umbral Caos", &modeloBase.c, 2.0f, 20.0f);

    dsv::CampoControl& pp_sep = menuP->agregarFila();
    pp_sep.agregar<dsv::CampoTexto>("-- Escenarios Rapidos --");

    // Periodico: sin caos, orbita cerrada en XY
    dsv::CampoControl& pp_per = menuP->agregarFila();
    pp_per.agregar<dsv::CampoBoton>("Periodico (c=4)", [&]{
        modeloBase.c = 4.0f;
    }, dsv::Color::verde_dd, dsv::Color::blanco, 150.f);

    // Bifurcacion: periodo 2 (la orbita XY tiene dos "lazos")
    dsv::CampoControl& pp_bif = menuP->agregarFila();
    pp_bif.agregar<dsv::CampoBoton>("Bifurcacion (c=6)", [&]{
        modeloBase.c = 6.0f;
    }, dsv::Color::oro_dd, dsv::Color::blanco, 150.f);

    // Caos clasico
    dsv::CampoControl& pp_caos = menuP->agregarFila();
    pp_caos.agregar<dsv::CampoBoton>("Caos Clasico (c=14)", [&]{
        modeloBase.a = 0.1f; modeloBase.b = 0.1f; modeloBase.c = 14.0f;
    }, dsv::Color::naranja_dd, dsv::Color::blanco, 150.f);

    dsv::CampoControl& pp_res = menuP->agregarFila();
    pp_res.agregar<dsv::CampoBoton>("Reset Params", [&]{
        modeloBase.a = 0.1f; modeloBase.b = 0.1f; modeloBase.c = 14.0f;
    }, dsv::Color::rojo_dd, dsv::Color::blanco, 150.f);


    // =========================================================================
    //  PANEL CI (izquierda)
    // =========================================================================
    dsv::PanelFlotante PanelCI(
        window, "CI",
        dsv::HandleOrientacion::Vertical,
        dsv::DespliegueDir::Der,
        dsv::Color::rojo
    );
    PanelCI.setDegradado(sf::Color(35, 8, 3, 245), sf::Color(18, 4, 2, 245));
    PanelCI.positionAbsoluta(dsv::Ubicacion::CentroIzq);

    dsv::MenuFlotante* menuCI = PanelCI.crearContenido<dsv::MenuFlotante>(6.f, 14.f);

    dsv::CampoControl& ci_t = menuCI->agregarFila();
    ci_t.agregar<dsv::CampoTexto>("-- Condiciones Iniciales --");

    dsv::CampoControl& ci_x = menuCI->agregarFila();
    ci_x.agregar<dsv::CampoDeslizador>("x0", &x0_base, -10.0f, 10.0f);
    dsv::CampoControl& ci_y = menuCI->agregarFila();
    ci_y.agregar<dsv::CampoDeslizador>("y0", &y0_base, -10.0f, 10.0f);
    dsv::CampoControl& ci_z = menuCI->agregarFila();
    ci_z.agregar<dsv::CampoDeslizador>("z0", &z0_base,   0.0f, 20.0f);
    dsv::CampoControl& ci_d = menuCI->agregarFila();
    ci_d.agregar<dsv::CampoDeslizador>("delta CI", &delta_ci, 0.0f, 1.0f);

    dsv::CampoControl& ci_sep = menuCI->agregarFila();
    ci_sep.agregar<dsv::CampoTexto>("-- Estado [0] --");
    dsv::CampoControl& ci_v0 = menuCI->agregarFila();
    ci_v0.agregar<dsv::CampoVariable>("x =", &sims[0].state[0]);
    dsv::CampoControl& ci_v1 = menuCI->agregarFila();
    ci_v1.agregar<dsv::CampoVariable>("y =", &sims[0].state[1]);
    dsv::CampoControl& ci_v2 = menuCI->agregarFila();
    ci_v2.agregar<dsv::CampoVariable>("z =", &sims[0].state[2]);

    dsv::CampoControl& ci_res = menuCI->agregarFila();
    ci_res.agregar<dsv::CampoBoton>("Reset CI [R]",
        [&]{ initSims(); }, dsv::Color::rojo);


    // =========================================================================
    //  PANEL TIEMPO
    // =========================================================================
    bool  pausa     = true;
    float timeScale = 1.0f;
    sf::Clock fpsClock;
    long long frameCount = 0;
    float fps = 0.0f, tiempo = 0.0f;

    const sf::Color colorMENU = dsv::Color::naranja;

    dsv::PanelFlotante PanelTiempo(
        window, "Menu",
        dsv::HandleOrientacion::Horizontal,
        dsv::DespliegueDir::Abajo,
        colorMENU
    );
    PanelTiempo.setDegradado(dsv::Color::naranja_dd % 230, dsv::Color::naranja_dd % 230);
    PanelTiempo.positionAbsoluta(dsv::Ubicacion::ArribaCentro);

    dsv::MenuFlotante* menuT = PanelTiempo.crearContenido<dsv::MenuFlotante>(6.f, 12.f);
    dsv::CampoControl& mt1 = menuT->agregarFila();
    mt1.agregar<dsv::CampoTexto>("-- Rossler: Espiral Caotica --");
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
    // Rossler es mas lento que Lorenz (lambda ~0.13 vs 0.9)
    // ups=0.005 da buena suavidad en la espiral
    sf::Time  ups = sf::seconds(0.005f);

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

        sf::Time elapsed = clock.restart();
        if (!pausa) accumulator += elapsed * timeScale;

        while (accumulator >= ups) {
            for (auto& s : sims) dsv::sim::RK4_step(s, ups.asSeconds());
            tiempo = sims[0].t;
            accumulator -= ups;
        }

        if (!pausa) {
            for (int i = 0; i < NUM_SIM; ++i) {
                const std::string id = "R" + std::to_string(i);
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
        PanelTiempo.draw(); PanelParams.draw(); PanelCI.draw();
        window.display();
    }
    return 0;
}
