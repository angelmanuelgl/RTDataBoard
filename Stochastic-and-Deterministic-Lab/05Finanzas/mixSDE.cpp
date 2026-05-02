/*
 * proyecto: DynSysVis RT -- Laboratorio Financiero SDE
 *
 * Tres modelos fundamentales de finanzas matematicas:
 *   1. DriftDiffusion  -- Movimiento Browniano con Deriva (base de Black-Scholes)
 *   2. BlackScholes    -- Movimiento Browniano Geometrico (precio de activos)
 *   3. CIR             -- Cox-Ingersoll-Ross (tasas de interes)
 *
 * Estrategia Monte Carlo en Tiempo Real:
 *   20 trayectorias por modelo, todas compartiendo el mismo modelo base.
 *   El "abanico" de curvas visualiza la distribucion de probabilidad en vivo.
 *   Mover un slider cambia todas las trayectorias simultaneamente.
 *
 * Layout:
 *
 *   +-------------------+-------------------+-------------------+
 *   |                   |                   |                   |
 *   |  DriftDiffusion   |   BlackScholes    |      CIR          |
 *   |  x(t) = mu*dt     |   dS = mu*S*dt    |  dx = a(b-x)dt   |
 *   |       + sigma*dW  |       +sigma*S*dW |       +s*sqrt(x) |
 *   |                   |                   |                   |
 *   +-------------------+-------------------+-------------------+
 *
 * -- DriftDiffusion: trayectorias lineales + ruido aditivo gaussiano.
 *    El "abanico" se abre linealmente con sqrt(t) (difusion clasica).
 * -- BlackScholes: trayectorias exponenciales con ruido multiplicativo.
 *    Ninguna trayectoria cruza x=0 (propiedad del GBM). La correccion de Ito
 *    hace que la media de log(S) crezca a (mu - sigma^2/2)*t, no a mu*t.
 * -- CIR: todas las trayectorias revierten a la media b con velocidad a.
 *    La condicion de Feller (2*a*b >= sigma^2) garantiza x > 0 siempre.
 *    El abanico es acotado: a mayor 'a', mas apretado.
 */

#include <SFML/Graphics.hpp>
#include <cmath>
#include <algorithm>
#include <string>
#include <vector>
#include <random>

#include "DynSysVis.hpp"



// =============================================================================
//  CONSTANTES
// =============================================================================
static constexpr int NUM_SIMS  = 20;   // trayectorias Monte Carlo por modelo
static constexpr int NUM_SIMS_DD = 20;
static constexpr int NUM_SIMS_BS = 20;
static constexpr int NUM_SIMS_CIR = 20;


// =============================================================================
//  HELPERS
// =============================================================================

// Reinicializa condiciones iniciales de un vector de instancias
template<typename Model>
void resetInstancias(std::vector<dsv::mod::Instance<Model>>& sims,
                     float valor_inicial)
{
    for (auto& s : sims) {
        s.state[0] = valor_inicial;
        s.t        = 0.0f;
    }
}

// Registra todas las trayectorias de un modelo en su GraficaTiempo
// Se llama una vez tras declarar las series, para garantizar colores correctos.
void registrarSeriesFinancieras(dsv::Vista<dsv::GraficaTiempo>& graf,
                                int numSims,
                                std::function<sf::Color(int, int)> paleta)
{
    for (int i = 0; i < numSims; ++i) {
        graf->agregarSerie("T" + std::to_string(i), paleta(i, numSims));
    }
}


// =============================================================================
//  MAIN
// =============================================================================
int main() {

    // --- Init -----------------------------------------------------------------
    dsv::Color::cargar("assets/config/colores.txt");
    sf::RenderWindow window;
    dsv::Sistema::inicializarVentana(window,
        "DynSysVis RT -- Laboratorio Financiero SDE");


    // =========================================================================
    //  MODELOS BASE (compartidos por todas las instancias de cada tipo)
    // =========================================================================

    // --- DriftDiffusion -------------------------------------------------------
    // dx = mu*dt + sigma*dW
    // noise_dim=1 -> sim::step() elige EM_step_simple automaticamente
    dsv::mod::DriftDiffusion_Model modeloDD;
    modeloDD.mu    = 0.05f;
    modeloDD.sigma = 0.20f;

    // --- BlackScholes ---------------------------------------------------------
    // dS = mu*S*dt + sigma*S*dW
    // Correccion de Ito: E[S(t)] = S0 * exp(mu*t)  (no mu-sigma^2/2)
    // La media geometrica crece a (mu - sigma^2/2)*t -- esto es clave para
    // entender por que las opciones no se valuan con mu sino con la tasa libre.
    dsv::mod::BlackScholes_Model modeloBS;
    modeloBS.mu    = 0.08f;
    modeloBS.sigma = 0.20f;

    // --- CIR ------------------------------------------------------------------
    // dx = a*(b - x)*dt + sigma*sqrt(x)*dW
    // Condicion de Feller: 2*a*b >= sigma^2 garantiza x > 0
    // Con defaults: 2*0.5*0.05 = 0.05 >= 0.01 -- condicion satisfecha.
    // Al aumentar sigma en el slider podria violarse: el clamp en sqrt(max(0,x))
    // del modelo actua como barrera numerica de seguridad.
    dsv::mod::CIR_Model modeloCIR;
    modeloCIR.a     = 0.5f;
    modeloCIR.b     = 0.05f;
    modeloCIR.sigma = 0.08f;


    // =========================================================================
    //  INSTANCIAS MONTE CARLO (todas vinculadas al modelo compartido)
    // =========================================================================

    // Generador aleatorio para condiciones iniciales con pequena perturbacion
    std::mt19937 rng(std::random_device{}());
    std::uniform_real_distribution<float> pertDD(-0.5f, 0.5f);
    std::uniform_real_distribution<float> pertBS( 90.0f, 110.0f);
    std::uniform_real_distribution<float> pertCIR(0.03f, 0.08f);

    // DriftDiffusion -- condicion inicial: x0 = 0 +/- perturbacion
    std::vector<dsv::mod::Instance<dsv::mod::DriftDiffusion_Model>> simsDD(NUM_SIMS_DD);
    for (auto& s : simsDD) {
        s.state[0] = pertDD(rng);
        s.model_ref.vincular(modeloDD);
    }

    // BlackScholes -- condicion inicial: precio S0 ~ U(90, 110)
    std::vector<dsv::mod::Instance<dsv::mod::BlackScholes_Model>> simsBS(NUM_SIMS_BS);
    for (auto& s : simsBS) {
        s.state[0] = pertBS(rng);
        s.model_ref.vincular(modeloBS);
    }

    // CIR -- condicion inicial: tasa r0 ~ U(0.03, 0.08)
    std::vector<dsv::mod::Instance<dsv::mod::CIR_Model>> simsCIR(NUM_SIMS_CIR);
    for (auto& s : simsCIR) {
        s.state[0] = pertCIR(rng);
        s.model_ref.vincular(modeloCIR);
    }


    // =========================================================================
    //  LAYOUT Y TABLERO
    // =========================================================================
    dsv::Layout miLayout = {
        "dd ",
        "bs ",
        "cir"
    };
    dsv::Tablero tablero(window, miLayout,
                         sf::Color(20, 22, 30), sf::Color(12, 13, 18));


    // =========================================================================
    //  GRAFICAS DE TIEMPO -- una por modelo
    // =========================================================================

    // --- DriftDiffusion -- azul/celeste --------------------------------------
    // Trayectorias gaussianas. El abanico crece como sigma*sqrt(t).
    dsv::Vista<dsv::GraficaTiempo> grafDD =
        tablero.add<dsv::GraficaTiempo>(
            "Drift Diffusion  dx = mu*dt + sigma*dW",
            dsv::Color::celeste, "dd");

    grafDD.objeto.configurarVentanaTiempo(20.0f);
    grafDD.objeto.ponerSombreado(false);
    for (int i = 0; i < NUM_SIMS_DD; ++i) {
        sf::Color col = dsv::Color::Oceano(i, NUM_SIMS_DD);
        grafDD->agregarSerie("T" + std::to_string(i), col);
    }


    // --- BlackScholes -- verde / logaritmico ---------------------------------
    // Trayectorias exponenciales. Ninguna cruza S=0.
    // La dispersion porcentual es constante (volatilidad proporcional al precio).
    dsv::Vista<dsv::GraficaTiempo> grafBS =
        tablero.add<dsv::GraficaTiempo>(
            "Black-Scholes GBM  dS = mu*S*dt + sigma*S*dW",
            dsv::Color::verde, "bs");

    grafBS.objeto.configurarVentanaTiempo(20.0f);
    grafBS.objeto.ponerSombreado(false);
    for (int i = 0; i < NUM_SIMS_BS; ++i) {
        sf::Color col = dsv::Color::Magma(i, NUM_SIMS_BS);
        grafBS->agregarSerie("T" + std::to_string(i), col);
    }


    // --- CIR -- naranja / reversion a la media -------------------------------
    // El abanico es acotado: las trayectorias oscilan alrededor de b.
    // A mayor 'a', el abanico es mas estrecho (reversion mas fuerte).
    dsv::Vista<dsv::GraficaTiempo> grafCIR =
        tablero.add<dsv::GraficaTiempo>(
            "CIR Rate  dx = a*(b-x)*dt + sigma*sqrt(x)*dW",
            dsv::Color::naranja, "cir");

    grafCIR.objeto.configurarVentanaTiempo(20.0f);
    grafCIR.objeto.ponerSombreado(false);
    for (int i = 0; i < NUM_SIMS_CIR; ++i) {
        sf::Color col = dsv::Color::Cyberpunk(i, NUM_SIMS_CIR);
        grafCIR->agregarSerie("T" + std::to_string(i), col);
    }

    // Degradado oscuro tipo terminal financiera
    tablero.setPanelDegradado(sf::Color(20, 22, 30), sf::Color(12, 13, 18));


    // =========================================================================
    //  PANEL FLOTANTE -- DRIFT DIFFUSION (izquierda)
    // =========================================================================
    dsv::PanelFlotante PanelDD(
        window, "Drift Diffusion",
        dsv::HandleOrientacion::Vertical,
        dsv::DespliegueDir::Der,
        dsv::Color::celeste
    );
    PanelDD.setDegradado(sf::Color(10, 25, 45, 240), sf::Color(8, 15, 25, 240));
    PanelDD.positionAbsoluta(dsv::Ubicacion::CentroIzq);

    dsv::MenuFlotante* menuDD = PanelDD.crearContenido<dsv::MenuFlotante>(6.f, 14.f);

    dsv::CampoControl& dd_t = menuDD->agregarFila();
    dd_t.agregar<dsv::CampoTexto>("--- Drift Diffusion ---");

    dsv::CampoControl& dd_mu = menuDD->agregarFila();
    dd_mu.agregar<dsv::CampoDeslizador>("mu  Deriva",     &modeloDD.mu,    -0.5f, 0.5f);

    dsv::CampoControl& dd_sig = menuDD->agregarFila();
    dd_sig.agregar<dsv::CampoDeslizador>("sigma  Volat.",  &modeloDD.sigma,  0.01f, 1.0f);

    // Mostrar x[0] de la primera trayectoria como referencia
    dsv::CampoControl& dd_var = menuDD->agregarFila();
    dd_var.agregar<dsv::CampoVariable>("x[0] =", &simsDD[0].state[0]);

    // Reset con R
    dsv::CampoControl& dd_res = menuDD->agregarFila();
    dd_res.agregar<dsv::CampoBoton>("Reset DD", [&]{
        std::uniform_real_distribution<float> p2(-0.5f, 0.5f);
        for (auto& s : simsDD) { s.state[0] = p2(rng); s.t = 0.0f; }
    }, dsv::Color::celeste);


    // =========================================================================
    //  PANEL FLOTANTE -- BLACK-SCHOLES (derecha)
    // =========================================================================
    dsv::PanelFlotante PanelBS(
        window, "Black-Scholes",
        dsv::HandleOrientacion::Vertical,
        dsv::DespliegueDir::Izq,
        dsv::Color::verde
    );
    PanelBS.setDegradado(sf::Color(10, 35, 20, 240), sf::Color(8, 18, 10, 240));
    PanelBS.positionAbsoluta(dsv::Ubicacion::CentroDer);

    dsv::MenuFlotante* menuBS = PanelBS.crearContenido<dsv::MenuFlotante>(6.f, 14.f);

    dsv::CampoControl& bs_t = menuBS->agregarFila();
    bs_t.agregar<dsv::CampoTexto>("--- Black-Scholes GBM ---");

    // Nota: la media de log(S) crece a (mu - sigma^2/2)*t
    // Si sigma es grande, la media geometrica puede decrecer aunque mu > 0
    dsv::CampoControl& bs_mu = menuBS->agregarFila();
    bs_mu.agregar<dsv::CampoDeslizador>("mu  Retorno anual",   &modeloBS.mu,    -0.3f, 0.5f);

    dsv::CampoControl& bs_sig = menuBS->agregarFila();
    bs_sig.agregar<dsv::CampoDeslizador>("sigma  Volat. anual", &modeloBS.sigma,  0.01f, 0.8f);

    // Correccion de Ito en vivo: tasa efectiva = mu - sigma^2/2
    float itoDisplay = 0.0f;
    dsv::CampoControl& bs_ito = menuBS->agregarFila();
    bs_ito.agregar<dsv::CampoVariable>("Ito drift = mu-s2/2", &itoDisplay);

    // Precio referencia
    dsv::CampoControl& bs_var = menuBS->agregarFila();
    bs_var.agregar<dsv::CampoVariable>("S[0] =", &simsBS[0].state[0]);

    dsv::CampoControl& bs_res = menuBS->agregarFila();
    bs_res.agregar<dsv::CampoBoton>("Reset BS", [&]{
        std::uniform_real_distribution<float> p2(90.0f, 110.0f);
        for (auto& s : simsBS) { s.state[0] = p2(rng); s.t = 0.0f; }
    }, dsv::Color::verde);


    // =========================================================================
    //  PANEL FLOTANTE -- CIR (abajo centro)
    // =========================================================================
    dsv::PanelFlotante PanelCIR(
        window, "CIR",
        dsv::HandleOrientacion::Horizontal,
        dsv::DespliegueDir::Arriba,
        dsv::Color::naranja
    );
    PanelCIR.setDegradado(dsv::Color::naranja_dd % 220, dsv::Color::naranja_dd % 220);
    PanelCIR.positionAbsoluta(dsv::Ubicacion::AbajoCentro);

    dsv::MenuFlotante* menuCIR = PanelCIR.crearContenido<dsv::MenuFlotante>(6.f, 12.f);

    dsv::CampoControl& cir_t = menuCIR->agregarFila();
    cir_t.agregar<dsv::CampoTexto>("--- CIR Interest Rate ---");

    // Velocidad de reversion: cuanto mayor, mas fuerte el retorno a b
    dsv::CampoControl& cir_a = menuCIR->agregarFila();
    cir_a.agregar<dsv::CampoDeslizador>("a  Rev. speed",   &modeloCIR.a,     0.01f, 5.0f);

    // Media de largo plazo
    dsv::CampoControl& cir_b = menuCIR->agregarFila();
    cir_b.agregar<dsv::CampoDeslizador>("b  Long-run mean",&modeloCIR.b,     0.001f, 0.20f);

    // Volatilidad -- cuidado: si sigma^2 > 2*a*b se viola Feller
    dsv::CampoControl& cir_sig = menuCIR->agregarFila();
    cir_sig.agregar<dsv::CampoDeslizador>("sigma  Volat.",  &modeloCIR.sigma, 0.001f, 0.3f);

    // Condicion de Feller calculada en vivo
    float fellerDisplay = 0.0f;  // 2*a*b - sigma^2; debe ser >= 0
    dsv::CampoControl& cir_fel = menuCIR->agregarFila();
    cir_fel.agregar<dsv::CampoVariable>("Feller = 2ab-s2", &fellerDisplay);

    // Tasa referencia
    dsv::CampoControl& cir_var = menuCIR->agregarFila();
    cir_var.agregar<dsv::CampoVariable>("r[0] =", &simsCIR[0].state[0]);

    dsv::CampoControl& cir_res = menuCIR->agregarFila();
    cir_res.agregar<dsv::CampoBoton>("Reset CIR", [&]{
        std::uniform_real_distribution<float> p2(0.03f, 0.08f);
        for (auto& s : simsCIR) { s.state[0] = p2(rng); s.t = 0.0f; }
    }, dsv::Color::naranja);


    // =========================================================================
    //  PANEL FLOTANTE -- MENU TIEMPO
    // =========================================================================
    bool  pausa     = true;
    float timeScale = 1.0f;

    sf::Clock fpsClock;
    long long frameCount = 0;
    float fps    = 0.0f;
    float tiempo = 0.0f;

    const sf::Color colorMENU = sf::Color(200, 200, 200);

    dsv::PanelFlotante PanelTiempo(
        window, "Menu Tiempo",
        dsv::HandleOrientacion::Horizontal,
        dsv::DespliegueDir::Abajo,
        colorMENU
    );
    PanelTiempo.setDegradado(sf::Color(40, 42, 50, 235), sf::Color(25, 26, 32, 235));
    PanelTiempo.positionAbsoluta(dsv::Ubicacion::ArribaCentro);

    dsv::MenuFlotante* menuTiempo = PanelTiempo.crearContenido<dsv::MenuFlotante>(6.f, 12.f);

    dsv::CampoControl& mt_f1 = menuTiempo->agregarFila();
    mt_f1.agregar<dsv::CampoTexto>("- - SDE Financial Lab - -");

    dsv::CampoControl& mt_f2 = menuTiempo->agregarFila();
    mt_f2.agregar<dsv::CampoVariable>("fps",    &fps);
    mt_f2.agregar<dsv::CampoVariable>("t =",    &tiempo);

    dsv::CampoControl& mt_f3 = menuTiempo->agregarFila();
    mt_f3.agregar<dsv::CampoBoton>("<<", [&]{ timeScale = std::max(0.1f, timeScale - 0.1f); }, colorMENU);
    mt_f3.agregar<dsv::CampoToggleTexto>("", &pausa, "Pausa", "Play", colorMENU);
    mt_f3.agregar<dsv::CampoBoton>(">>", [&]{ timeScale += 0.1f; }, colorMENU);

    dsv::CampoControl& mt_f4 = menuTiempo->agregarFila();
    mt_f4.agregar<dsv::CampoVariable>("Speed x", &timeScale);

    dsv::CampoControl& mt_f5 = menuTiempo->agregarFila();
    mt_f5.agregar<dsv::CampoBoton>("Reset Vel", [&]{ timeScale = 1.0f; });
    mt_f5.agregar<dsv::CampoToggle>("Pausa", &pausa, colorMENU);

    // Reset global: reinicia las 3 familias de trayectorias
    dsv::CampoControl& mt_f6 = menuTiempo->agregarFila();
    mt_f6.agregar<dsv::CampoBoton>("Reset ALL [R]", [&]{
        {
            std::uniform_real_distribution<float> p(-0.5f, 0.5f);
            for (auto& s : simsDD)  { s.state[0] = p(rng);  s.t = 0.0f; }
        }
        {
            std::uniform_real_distribution<float> p(90.0f, 110.0f);
            for (auto& s : simsBS)  { s.state[0] = p(rng);  s.t = 0.0f; }
        }
        {
            std::uniform_real_distribution<float> p(0.03f, 0.08f);
            for (auto& s : simsCIR) { s.state[0] = p(rng);  s.t = 0.0f; }
        }
        tiempo = 0.0f;
    }, colorMENU);


    // =========================================================================
    //  BUCLE PRINCIPAL
    // =========================================================================
    sf::Clock clock;
    sf::Time  accumulator = sf::Time::Zero;
    // ups pequeno para que las trayectorias SDE sean suaves
    sf::Time  ups = sf::seconds(0.005f);

    while (window.isOpen()) {

        // --- Eventos ----------------------------------------------------------
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window.close();

            PanelTiempo.gestionarEvento(event);
            PanelDD.gestionarEvento(event);
            PanelBS.gestionarEvento(event);
            PanelCIR.gestionarEvento(event);

            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Space)
                    pausa = !pausa;
                if (event.key.code == sf::Keyboard::Up)
                    timeScale += 0.2f;
                if (event.key.code == sf::Keyboard::Down)
                    timeScale = std::max(0.1f, timeScale - 0.2f);
                // Tecla R: reiniciar todas las trayectorias
                if (event.key.code == sf::Keyboard::R) {
                    std::uniform_real_distribution<float> pDD(-0.5f,  0.5f);
                    std::uniform_real_distribution<float> pBS(90.0f, 110.0f);
                    std::uniform_real_distribution<float> pCIR(0.03f, 0.08f);
                    for (auto& s : simsDD)  { s.state[0] = pDD(rng);  s.t = 0.0f; }
                    for (auto& s : simsBS)  { s.state[0] = pBS(rng);  s.t = 0.0f; }
                    for (auto& s : simsCIR) { s.state[0] = pCIR(rng); s.t = 0.0f; }
                    tiempo = 0.0f;
                }
            }
        }

        // --- Simulacion -- acumulador de tiempo -------------------------------
        sf::Time elapsed = clock.restart();
        if (!pausa)
            accumulator += elapsed * timeScale;

        while (accumulator >= ups) {
            float dt = ups.asSeconds();

            // Integracion estocastica: sim::step() detecta noise_dim=1
            // y llama EM_step_simple (Euler-Maruyama) automaticamente.
            // Cada llamada genera un dW independiente por instancia.
            for (auto& s : simsDD)  dsv::sim::step(s, dt);
            for (auto& s : simsBS)  dsv::sim::step(s, dt);
            for (auto& s : simsCIR) dsv::sim::step(s, dt);

            // Metricas en vivo
            tiempo     = simsDD[0].t;
            // Correccion de Ito: tasa de crecimiento efectiva del log-precio
            itoDisplay = modeloBS.mu - 0.5f * modeloBS.sigma * modeloBS.sigma;
            // Condicion de Feller: debe ser >= 0 para garantizar x > 0 en CIR
            fellerDisplay = 2.0f * modeloCIR.a * modeloCIR.b
                          - modeloCIR.sigma * modeloCIR.sigma;

            accumulator -= ups;
        }

        // --- Alimentar graficas -----------------------------------------------
        if (!pausa) {
            for (int i = 0; i < NUM_SIMS_DD; ++i) {
                grafDD->push_back(simsDD[i].state[0], simsDD[i].t,
                                  "T" + std::to_string(i));
            }
            for (int i = 0; i < NUM_SIMS_BS; ++i) {
                grafBS->push_back(simsBS[i].state[0], simsBS[i].t,
                                  "T" + std::to_string(i));
            }
            for (int i = 0; i < NUM_SIMS_CIR; ++i) {
                grafCIR->push_back(simsCIR[i].state[0], simsCIR[i].t,
                                   "T" + std::to_string(i));
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
        PanelDD.draw();
        PanelBS.draw();
        PanelCIR.draw();
        window.display();
    }

    return 0;
}