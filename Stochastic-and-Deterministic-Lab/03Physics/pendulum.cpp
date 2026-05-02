/*
 * proyecto: DynSysVis RT -- Pendulo Real vs Aproximacion Lineal
 *
 * Objetivo: demostrar visualmente donde falla sin(theta) ~= theta.
 *
 * Por que falla la aproximacion lineal en angulos grandes:
 *   El error de la aproximacion es: sin(theta) = theta - theta^3/6 + ...
 *   Para theta = 0.3 rad (~17 deg), error ~= 0.3^3/6 = 0.0045  (~1.5%)  OK
 *   Para theta = 1.5 rad (~86 deg), error ~= 1.5^3/6 = 0.5625  (~56%)   MAL
 *   La consecuencia practica: el pendulo lineal oscila con periodo fijo
 *   T_lineal = 2*pi*sqrt(L/g), mientras que el real tiene un periodo mayor
 *   que depende de la amplitud. A angulos grandes, el real es MAS LENTO.
 *   Esto se ve como un desfase creciente entre las dos curvas theta(t).
 *
 * Estrategia de visualizacion:
 *
 *   +------------------+------------------+------------------+------------------+
 *   |                  |                  |                  |                  |
 *   |  Espacio fase    |  Espacio fase    | Posicion (x,y)   | Posicion (x,y)   |
 *   |  REAL (theta,w)  |  LINEAL(theta,w) | REAL (animacion) | LINEAL(animacion)|
 *   |  espiral abierta |  espiral cerrada | trayectoria arco | trayectoria arco |
 *   |                  |                  |                  |                  |
 *   +------------------+------------------+------------------+------------------+
 *   |  theta_R(t)   vs    theta_L(t)  -- divergen con el tiempo                |
 *   +-------------------------------------------------------------------+-------+
 *
 *  -- Espacio de fase: el pendulo real con friccion espirala al origen.
 *    El lineal forma una espiral perfectamente simetrica (es un oscilador
 *    armonico amortiguado exacto). Con friccion=0, el real forma orbitas
 *    cerradas deformadas (NO circulos), el lineal forma elipses perfectas.
 *
 *  -- Posicion cartesiana x=L*sin(theta), y=-L*cos(theta):
 *    Muestra el movimiento fisico real del extremo del pendulo.
 *    Con configurarMaxPoints corto y desvanecido, el punto "se mueve"
 *    como una animacion dentro del panel de la libreria.
 *    El rango fisico es [-L, L] en x y [-L, 0] en y (semiesfera inferior).
 *
 *  -- Tiempo theta(t): la curva roja y azul empiezan iguales y divergen.
 *    El momento de divergencia visible depende del angulo inicial: a
 *    mayor theta0, mas rapido se separan.
 *
 * Instancias multiples: 5 pendulos por modelo con theta0 ligeramente
 *   distinto (1.2 a 1.8 rad). Sirve para ver el campo de trayectorias
 *   y confirmar que la diferencia es sistematica, no solo de una condicion.
 */

#include <SFML/Graphics.hpp>
#include <cmath>
#include <algorithm>
#include <string>
#include <vector>

#include "DynSysVis.hpp"
// Los modelos Pendulum_Model y Pendulum_Model_Simple ya estan en dsv::mod


// =============================================================================
//  CONSTANTES
// =============================================================================
static constexpr int   NUM_PEND   = 5;
static constexpr float THETA0_MIN = 1.2f;  // rad -- angulo grande para ver el error
static constexpr float THETA0_MAX = 1.8f;  // rad -- casi 103 grados
static constexpr float OMEGA0     = 0.0f;  // velocidad angular inicial (reposo)
static constexpr float L_ESCALA   = 1.0f;  // longitud para las coordenadas cartesianas


// =============================================================================
//  MAIN
// =============================================================================
int main() {

    // --- Init -----------------------------------------------------------------
    dsv::Color::cargar("assets/config/colores.txt");
    sf::RenderWindow window;
    dsv::Sistema::inicializarVentana(window,
        "DynSysVis RT -- Pendulo Real vs Lineal");


    // =========================================================================
    //  MODELOS BASE (compartidos por todas las instancias de cada tipo)
    //  Los parametros g, L, gamma se mantienen sincronizados en el bucle.
    // =========================================================================
    dsv::mod::Pendulum_Model        modeloReal;
    dsv::mod::Pendulum_Model_Simple modeloLineal;

    // Parametros comunes -- el slider modifica estos y se copian cada tick
    float g_param     = 9.81f;
    float L_param     = 1.0f;
    float gamma_param = 0.05f; // friccion baja para ver la divergencia claramente

    modeloReal.g     = g_param;
    modeloReal.L     = L_param;
    modeloReal.gamma = gamma_param;

    modeloLineal.g     = g_param;
    modeloLineal.L     = L_param;
    modeloLineal.gamma = gamma_param;


    // =========================================================================
    //  INSTANCIAS MONTE CARLO (N pendulos con theta0 distintos)
    // =========================================================================

    // Condiciones iniciales: theta0 distribuido entre THETA0_MIN y THETA0_MAX
    auto theta0_i = [&](int i) -> float {
        if (NUM_PEND == 1) return (THETA0_MIN + THETA0_MAX) * 0.5f;
        return THETA0_MIN + i * (THETA0_MAX - THETA0_MIN) / (NUM_PEND - 1);
    };

    // Pendulos reales
    std::vector<dsv::mod::Instance<dsv::mod::Pendulum_Model>> simsR(NUM_PEND);
    for (int i = 0; i < NUM_PEND; ++i) {
        simsR[i].state[dsv::mod::Pendulum_Model::THETA] = theta0_i(i);
        simsR[i].state[dsv::mod::Pendulum_Model::OMEGA] = OMEGA0;
        simsR[i].model_ref.vincular(modeloReal);
    }

    // Pendulos lineales (mismas condiciones iniciales para comparacion justa)
    std::vector<dsv::mod::Instance<dsv::mod::Pendulum_Model_Simple>> simsL(NUM_PEND);
    for (int i = 0; i < NUM_PEND; ++i) {
        simsL[i].state[dsv::mod::Pendulum_Model_Simple::THETA] = theta0_i(i);
        simsL[i].state[dsv::mod::Pendulum_Model_Simple::OMEGA] = OMEGA0;
        simsL[i].model_ref.vincular(modeloLineal);
    }

    // Lambda para reiniciar todas las instancias
    auto resetTodo = [&]() {
        for (int i = 0; i < NUM_PEND; ++i) {
            simsR[i].state[0] = theta0_i(i);
            simsR[i].state[1] = OMEGA0;
            simsR[i].t        = 0.0f;
            simsL[i].state[0] = theta0_i(i);
            simsL[i].state[1] = OMEGA0;
            simsL[i].t        = 0.0f;
        }
    };


    // =========================================================================
    //  LAYOUT Y TABLERO
    //
    //  Fila 0-2 (3/4 de la pantalla):
    //    fR  fL  mapR  mapL   -- fase real | fase lineal | mapa real | mapa lineal
    //  Fila 3 (1/4 inferior):
    //    tRL tRL tRL   tRL    -- theta(t) ambos superpuestos para ver desfase
    //    + tR individual y tL individual
    // =========================================================================
    dsv::Layout miLayout = {
        "fR  fR  fL  fL  mapR mapR mapL mapL",
        "fR  fR  fL  fL  mapR mapR mapL mapL",
        "fR  fR  fL  fL  mapR mapR mapL mapL",
        "tRL tRL tRL tRL tR   tR   tL   tL  ",
    };
    dsv::Tablero tablero(window, miLayout,
                         sf::Color(18, 18, 22), sf::Color(10, 10, 14));


    // =========================================================================
    //  PALETAS DE COLOR
    //  Real   --> tonos calidos (rojo, naranja, coral)
    //  Lineal --> tonos frios  (azul, cian, celeste)
    // =========================================================================
    auto colorReal   = [](int i, int n) { return dsv::Color::Magma(i, n);  };
    auto colorLineal = [](int i, int n) { return dsv::Color::Oceano(i, n); };


    // =========================================================================
    //  ESPACIO DE FASE (theta, omega) -- espiral amortigua al origen
    //  Con gamma=0: orbitas cerradas (real) vs elipses perfectas (lineal)
    // =========================================================================

    // Pendulo real -- espiral asimetrica (periodo depende de amplitud)
    dsv::Vista<dsv::EspacioFase2D> faseR =
        tablero.add<dsv::EspacioFase2D>(
            "Fase REAL (theta, omega) -- sin(theta)",
            dsv::Color::rojo, "fR");

    faseR->activarSeguimiento(false);
    faseR->activarAutoescalado(true);
    faseR->ponerDesvanecido(true);
    faseR->ponerCabeza(true);
    faseR->configurarMaxPoints(300); // cola media: se ve la espiral pero no satura

    for (int i = 0; i < NUM_PEND; ++i)
        faseR->agregarSerie("R" + std::to_string(i), colorReal(i, NUM_PEND));

    // Pendulo lineal -- espiral perfecta (oscilador armonico exacto)
    dsv::Vista<dsv::EspacioFase2D> faseL =
        tablero.add<dsv::EspacioFase2D>(
            "Fase LINEAL (theta, omega) -- theta (aprox.)",
            dsv::Color::celeste, "fL");

    faseL->activarSeguimiento(false);
    faseL->activarAutoescalado(true);
    faseL->ponerDesvanecido(true);
    faseL->ponerCabeza(true);
    faseL->configurarMaxPoints(300);

    for (int i = 0; i < NUM_PEND; ++i)
        faseL->agregarSerie("L" + std::to_string(i), colorLineal(i, NUM_PEND));


    // =========================================================================
    //  MAPA CARTESIANO (x, y) -- ANIMACION DEL MOVIMIENTO FISICO
    //
    //  x = L * sin(theta)   y = -L * cos(theta)
    //
    //  El extremo del pendulo recorre un arco de circunferencia de radio L.
    //  Con cola muy corta (MaxPoints=60) y desvanecido, el punto "se mueve"
    //  como una animacion. El panel muestra la fisica en el espacio real.
    //
    //  Limites: x in [-L, L],  y in [-L, 0]  (semiesfera inferior)
    //  Con angulo inicial de 1.5 rad: x_max = L*sin(1.5) ~= 0.997*L (casi horizontal)
    // =========================================================================

    // Mapa real
    dsv::Vista<dsv::EspacioFase2D> mapR =
        tablero.add<dsv::EspacioFase2D>(
            "Posicion Fisica REAL  x=L*sin(t)  y=-L*cos(t)",
            dsv::Color::rojo, "mapR");

    mapR->activarSeguimiento(false);
    mapR->activarAutoescalado(false);
    // Espacio fisico: [-L-margen, L+margen] x [-L-margen, 0.2]
    mapR->configurarLimites(-1.3f * L_ESCALA,  1.3f * L_ESCALA,
                            -1.3f * L_ESCALA,  0.3f * L_ESCALA);
    mapR->ponerDesvanecido(true);
    mapR->ponerCabeza(true);
    mapR->configurarMaxPoints(60); // cola muy corta -- efecto "bola que se mueve"

    for (int i = 0; i < NUM_PEND; ++i)
        mapR->agregarSerie("R" + std::to_string(i), colorReal(i, NUM_PEND));

    // Mapa lineal
    dsv::Vista<dsv::EspacioFase2D> mapL =
        tablero.add<dsv::EspacioFase2D>(
            "Posicion Fisica LINEAL  x=L*sin(t)  y=-L*cos(t)",
            dsv::Color::celeste, "mapL");

    mapL->activarSeguimiento(false);
    mapL->activarAutoescalado(false);
    mapL->configurarLimites(-1.3f * L_ESCALA,  1.3f * L_ESCALA,
                            -1.3f * L_ESCALA,  0.3f * L_ESCALA);
    mapL->ponerDesvanecido(true);
    mapL->ponerCabeza(true);
    mapL->configurarMaxPoints(60);

    for (int i = 0; i < NUM_PEND; ++i)
        mapL->agregarSerie("L" + std::to_string(i), colorLineal(i, NUM_PEND));


    // =========================================================================
    //  TIEMPO theta(t) -- la divergencia mas evidente
    //
    //  tRL: ambas curvas superpuestas en el mismo panel.
    //       Las curvas comienzan identicas y se desfasan visualmente.
    //       Solo se muestra el pendulo i=0 (el mas extremo: theta0=1.8 rad).
    //       A mayor angulo inicial, mayor velocidad de divergencia.
    //
    //  tR / tL: individuales para ver el periodo propio de cada modelo.
    //           El real tiene periodo mas largo (se ve como curva mas "ancha").
    // =========================================================================

    // Superposicion: Real (rojo) vs Lineal (azul) en el mismo panel
    dsv::Vista<dsv::GraficaTiempo> grafTRL =
        tablero.add<dsv::GraficaTiempo>(
            "theta(t): REAL vs LINEAL -- divergencia visible",
            dsv::Color::blanco, "tRL");

    grafTRL->agregarSerie("Real",   dsv::Color::rojo);
    grafTRL->agregarSerie("Lineal", dsv::Color::celeste);
    grafTRL.objeto.configurarVentanaTiempo(60.0f);
    grafTRL.objeto.ponerSombreado(false);

    // Individual real
    dsv::Vista<dsv::GraficaTiempo> grafTR =
        tablero.add<dsv::GraficaTiempo>(
            "theta(t) REAL",
            dsv::Color::rojo, "tR", dsv::Color::rojo);

    grafTR.objeto.configurarVentanaTiempo(30.0f);
    grafTR.objeto.ponerSombreado(true);

    // Individual lineal
    dsv::Vista<dsv::GraficaTiempo> grafTL =
        tablero.add<dsv::GraficaTiempo>(
            "theta(t) LINEAL",
            dsv::Color::celeste, "tL", dsv::Color::celeste);

    grafTL.objeto.configurarVentanaTiempo(30.0f);
    grafTL.objeto.ponerSombreado(true);

    // Degradado oscuro tipo pizarron
    tablero.setPanelDegradado(sf::Color(18, 18, 22), sf::Color(10, 10, 14));


    // =========================================================================
    //  PANEL FLOTANTE -- CONTROL DE TIEMPO (arriba centro)
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
    mt_f1.agregar<dsv::CampoTexto>("-- Pendulo Real vs Lineal --");

    dsv::CampoControl& mt_f2 = menuTiempo->agregarFila();
    mt_f2.agregar<dsv::CampoVariable>("fps",    &fps);
    mt_f2.agregar<dsv::CampoVariable>("t =",    &tiempo);

    dsv::CampoControl& mt_f3 = menuTiempo->agregarFila();
    mt_f3.agregar<dsv::CampoBoton>("<<",
        [&]{ timeScale = std::max(0.1f, timeScale - 0.1f); }, colorMENU);
    mt_f3.agregar<dsv::CampoToggleTexto>("", &pausa, "Pausa", "Play", colorMENU);
    mt_f3.agregar<dsv::CampoBoton>(">>",
        [&]{ timeScale += 0.1f; }, colorMENU);

    dsv::CampoControl& mt_f4 = menuTiempo->agregarFila();
    mt_f4.agregar<dsv::CampoVariable>("Speed x", &timeScale);

    dsv::CampoControl& mt_f5 = menuTiempo->agregarFila();
    mt_f5.agregar<dsv::CampoBoton>("Reset Vel", [&]{ timeScale = 1.0f; });
    mt_f5.agregar<dsv::CampoToggle>("Pausa", &pausa, colorMENU);

    dsv::CampoControl& mt_f6 = menuTiempo->agregarFila();
    mt_f6.agregar<dsv::CampoBoton>("Reset [R]", [&]{ resetTodo(); tiempo = 0.0f; }, colorMENU);


    // =========================================================================
    //  PANEL FLOTANTE -- FISICA (derecha)
    //  Controla g, L, gamma en AMBOS modelos simultaneamente.
    //  El slider modifica las variables puente (g_param, etc.) y el bucle
    //  las copia a los dos modelos cada tick.
    // =========================================================================
    dsv::PanelFlotante PanelFisica(
        window, "Fisica",
        dsv::HandleOrientacion::Vertical,
        dsv::DespliegueDir::Izq,
        dsv::Color::verde
    );
    PanelFisica.setDegradado(sf::Color(12, 35, 18, 245), sf::Color(8, 18, 10, 245));
    PanelFisica.positionAbsoluta(dsv::Ubicacion::CentroDer);

    dsv::MenuFlotante* menuFisica = PanelFisica.crearContenido<dsv::MenuFlotante>(6.f, 14.f);

    dsv::CampoControl& pf_t = menuFisica->agregarFila();
    pf_t.agregar<dsv::CampoTexto>("-- Parametros Fisicos --");

    dsv::CampoControl& pf_sep1 = menuFisica->agregarFila();
    pf_sep1.agregar<dsv::CampoTexto>("(aplican a ambos modelos)");

    // Gravedad
    dsv::CampoControl& pf_g = menuFisica->agregarFila();
    pf_g.agregar<dsv::CampoDeslizador>("g  Gravedad",  &g_param,     1.0f, 25.0f);

    // Longitud -- afecta el periodo T = 2*pi*sqrt(L/g) y los limites del mapa
    dsv::CampoControl& pf_L = menuFisica->agregarFila();
    pf_L.agregar<dsv::CampoDeslizador>("L  Longitud",  &L_param,     0.1f, 3.0f);

    // Friccion -- con gamma=0 se ven orbitas cerradas (conservativo)
    //            con gamma>0 la espiral converge al origen
    dsv::CampoControl& pf_gam = menuFisica->agregarFila();
    pf_gam.agregar<dsv::CampoDeslizador>("gamma  Friccion", &gamma_param, 0.0f, 2.0f);

    // Periodo teorico del lineal (exacto) y del real (aproximado para theta pequeno)
    // T_lineal = 2*pi*sqrt(L/g) -- independiente de amplitud
    // T_real ~= T_lineal * (1 + theta0^2/16 + ...) para theta0 moderado
    float T_lineal_display = 0.0f;
    float T_real_aprox     = 0.0f;
    dsv::CampoControl& pf_Tl = menuFisica->agregarFila();
    pf_Tl.agregar<dsv::CampoVariable>("T lineal [s] =", &T_lineal_display);

    dsv::CampoControl& pf_Tr = menuFisica->agregarFila();
    pf_Tr.agregar<dsv::CampoVariable>("T real aprox =", &T_real_aprox);

    // Angulo inicial del pendulo mas inclinado
    dsv::CampoControl& pf_t0 = menuFisica->agregarFila();
    pf_t0.agregar<dsv::CampoTexto>("theta0 max = 1.8 rad (103 deg)");

    dsv::CampoControl& pf_res = menuFisica->agregarFila();
    pf_res.agregar<dsv::CampoBoton>("Reset C.I.", [&]{
        resetTodo(); tiempo = 0.0f;
    }, dsv::Color::verde);


    // =========================================================================
    //  BUCLE PRINCIPAL
    // =========================================================================
    sf::Clock clock;
    sf::Time  accumulator = sf::Time::Zero;
    // ups pequeno para suavidad -- el pendulo rapido (g=25, L=0.1) tiene
    // periodo T ~= 0.4 s, necesitamos al menos 50 pasos por periodo.
    sf::Time  ups = sf::seconds(0.002f);

    while (window.isOpen()) {

        // --- Eventos ----------------------------------------------------------
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window.close();

            PanelTiempo.gestionarEvento(event);
            PanelFisica.gestionarEvento(event);

            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Space)
                    pausa = !pausa;
                if (event.key.code == sf::Keyboard::Up)
                    timeScale += 0.2f;
                if (event.key.code == sf::Keyboard::Down)
                    timeScale = std::max(0.1f, timeScale - 0.2f);
                // R: reiniciar posiciones de todos los pendulos
                if (event.key.code == sf::Keyboard::R) {
                    resetTodo();
                    tiempo = 0.0f;
                }
            }
        }

        // --- Sincronizar parametros comunes a ambos modelos ------------------
        // El slider modifica g_param, L_param, gamma_param.
        // Aqui se propagan a los dos structs de modelo.
        modeloReal.g     = g_param;
        modeloReal.L     = L_param;
        modeloReal.gamma = gamma_param;
        modeloLineal.g     = g_param;
        modeloLineal.L     = L_param;
        modeloLineal.gamma = gamma_param;

        // Metricas de periodo en vivo
        T_lineal_display = 2.0f * 3.14159f * std::sqrt(L_param / g_param);
        // Correccion de primer orden para angulo grande: T_real ~= T0*(1 + theta0^2/16)
        float theta_ref = (THETA0_MIN + THETA0_MAX) * 0.5f;
        T_real_aprox = T_lineal_display * (1.0f + theta_ref * theta_ref / 16.0f);

        // --- Simulacion -------------------------------------------------------
        sf::Time elapsed = clock.restart();
        if (!pausa)
            accumulator += elapsed * timeScale;

        while (accumulator >= ups) {
            float dt = ups.asSeconds();
            // RK4 para precision en el pendulo no lineal:
            // el termino sin(theta) tiene curvatura alta cerca de theta=pi/2
            for (auto& s : simsR) dsv::sim::RK4_step(s, dt);
            for (auto& s : simsL) dsv::sim::RK4_step(s, dt);
            tiempo = simsR[0].t;
            accumulator -= ups;
        }

        // --- Alimentar graficas -----------------------------------------------
        if (!pausa) {
            for (int i = 0; i < NUM_PEND; ++i) {
                const std::string idR = "R" + std::to_string(i);
                const std::string idL = "L" + std::to_string(i);

                const float thetaR = simsR[i].state[dsv::mod::Pendulum_Model::THETA];
                const float omegaR = simsR[i].state[dsv::mod::Pendulum_Model::OMEGA];
                const float thetaL = simsL[i].state[dsv::mod::Pendulum_Model_Simple::THETA];
                const float omegaL = simsL[i].state[dsv::mod::Pendulum_Model_Simple::OMEGA];
                const float t_sim  = simsR[i].t;

                // Espacio de fase (theta, omega)
                faseR->push_back(thetaR, omegaR, idR);
                faseL->push_back(thetaL, omegaL, idL);

                // Posicion cartesiana -- animacion fisica
                // x = L*sin(theta),  y = -L*cos(theta)
                // El extremo del pendulo describe un arco de radio L.
                float xR = L_param * std::sin(thetaR);
                float yR = -L_param * std::cos(thetaR);
                float xL = L_param * std::sin(thetaL);
                float yL = -L_param * std::cos(thetaL);

                mapR->push_back(xR, yR, idR);
                mapL->push_back(xL, yL, idL);

                // Series de tiempo -- solo el pendulo central (i=2) para claridad
                // en el panel de superposicion; todos en los individuales
                if (i == NUM_PEND / 2) {
                    grafTRL->push_back(thetaR, t_sim, "Real");
                    grafTRL->push_back(thetaL, t_sim, "Lineal");
                }
                grafTR->push_back(thetaR, t_sim);
                grafTL->push_back(thetaL, t_sim);
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
        PanelFisica.draw();
        window.display();
    }

    return 0;
}