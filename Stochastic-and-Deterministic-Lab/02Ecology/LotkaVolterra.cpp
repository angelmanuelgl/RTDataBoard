/*
 * proyecto: DynSysVis RT -- Ecologia Estocastica: Lotka-Volterra Det vs SDE
 *
 * Comparativa de alta fidelidad entre el modelo clasico y su version con
 * ruido ambiental multiplicativo en las tasas de natalidad y mortalidad.
 *
 * Por que el ruido multiplicativo es biologicamente distinto al aditivo:
 *   El modelo SDE usa  dx1 = x1*(alpha-beta*x2)*dt + sigma1*x1*dW1
 *                      dx2 = x2*(delta*x1-gamma)*dt + sigma2*x2*dW2
 *   El termino sigma_i * x_i hace que el ruido se anule cuando x_i = 0.
 *   Consecuencia: la extincion es un ESTADO ABSORBENTE. Si una especie
 *   cae a cero por fluctuacion, no puede recuperarse. Esto no ocurre en
 *   el modelo determinista, donde el equilibrio es un centro estable y
 *   las orbitas son cerradas periodicas para siempre.
 *
 * Punto de equilibrio del sistema deterministico:
 *   (x1*, x2*) = (gamma/delta, alpha/beta)
 *   Con defaults alpha=1.1, beta=0.4, delta=0.1, gamma=0.4:
 *   x1* = 0.4/0.1 = 4.0 presas
 *   x2* = 1.1/0.4 = 2.75 depredadores
 *   En este punto las poblaciones son constantes. Fuera de el: orbitas cerradas.
 *
 * Efecto del ruido sobre la conservacion de energia (cantidad de Lyapunov):
 *   El modelo deterministico conserva V = delta*x1 - gamma*ln(x1)
 *                                       + beta*x2  - alpha*ln(x2)
 *   El ruido SDE rompe esta conservacion: la orbita puede espiralar hacia
 *   el origen (extincion) o alejarse del equilibrio de forma irregular.
 *   Sigma alto -> mayor probabilidad de extincion en tiempo finito.
 *
 * Layout:
 *   +--------+--------+--------+--------+
 *   | det    | det    | sto    | sto    |   Espacio de fase
 *   | faseD  | faseD  | faseS  | faseS  |   Det: orbitas perfectas (elipses)
 *   +--------+--------+--------+--------+   Sto: espiral erratica hacia ext.
 *   | tDet   | tDet   | tSto   | tSto   |   Tiempo: presas y depredadores
 *   +--------+--------+--------+--------+
 *   | cmpP   | cmpP   | cmpD   | cmpD   |   Comparativa directa superpuesta
 *   +--------+--------+--------+--------+
 *
 *  -- faseD: orbitas periodicas cerradas. Si el integrador es Euler,
 *    la orbita espirala artificialmente. Por eso usamos RK4.
 *  -- faseS: 5 trayectorias Monte Carlo. Algunas pueden extinguirse.
 *  -- tDet/tSto: oscilaciones presas (cian) y depredadores (naranja).
 *  -- cmpP/cmpD: presas y depredadores de ambos modelos superpuestos.
 *    La divergencia entre la curva Det y la media SDE es el efecto del ruido.
 */

#include <SFML/Graphics.hpp>
#include <cmath>
#include <algorithm>
#include <string>
#include <vector>

#include "DynSysVis.hpp"
// dsv::mod::LotkaVolterra_Model y StochasticLotkaVolterra_Model ya en la lib


// =============================================================================
//  CONSTANTES
// =============================================================================
static constexpr int   NUM_STO  = 5;    // trayectorias Monte Carlo estocasticas

// Condiciones iniciales -- fuera del equilibrio para orbitas visibles
// x1* = gamma/delta = 4.0,  x2* = alpha/beta = 2.75
// Iniciamos en (10, 5): lejos del equilibrio, orbita amplia
static constexpr float X1_0 = 10.0f;   // presas iniciales
static constexpr float X2_0 =  5.0f;   // depredadores iniciales


// =============================================================================
//  MAIN
// =============================================================================
int main() {

    // --- Init -----------------------------------------------------------------
    dsv::Color::cargar("assets/config/colores.txt");
    sf::RenderWindow window;
    dsv::Sistema::inicializarVentana(window,
        "DynSysVis RT -- Lotka-Volterra: Deterministico vs Estocastico");


    // =========================================================================
    //  MODELOS BASE
    //  Los parametros ecologicos (alpha, beta, delta, gamma) son compartidos:
    //  los sliders del Panel Ecologico apuntan al modelo deterministico y el
    //  bucle copia los valores al estocastico. Asi la comparacion es justa.
    // =========================================================================

    // Modelo deterministico -- una sola instancia, orbitas perfectas
    dsv::mod::LotkaVolterra_Model modeloDet;
    modeloDet.alpha = 1.1f;
    modeloDet.beta  = 0.4f;
    modeloDet.delta = 0.1f;
    modeloDet.gamma = 0.4f;

    // Modelo estocastico -- parametros ecologicos identicos + ruido propio
    dsv::mod::StochasticLotkaVolterra_Model modeloSto;
    modeloSto.alpha  = modeloDet.alpha;
    modeloSto.beta   = modeloDet.beta;
    modeloSto.delta  = modeloDet.delta;
    modeloSto.gamma  = modeloDet.gamma;
    modeloSto.sigma1 = 0.10f;   // ruido ambiental en presas
    modeloSto.sigma2 = 0.10f;   // ruido ambiental en depredadores


    // =========================================================================
    //  INSTANCIAS
    // =========================================================================

    // Una instancia determinista
    dsv::mod::Instance<dsv::mod::LotkaVolterra_Model> instDet;
    instDet.state[dsv::mod::LotkaVolterra_Model::PRESA]      = X1_0;
    instDet.state[dsv::mod::LotkaVolterra_Model::DEPREDADOR] = X2_0;
    instDet.model_ref.vincular(modeloDet);

    // NUM_STO instancias estocasticas (mismo estado inicial, ruido distinto)
    using StoInst = dsv::mod::Instance<dsv::mod::StochasticLotkaVolterra_Model>;
    std::vector<StoInst> simsSto(NUM_STO);
    for (auto& s : simsSto) {
        s.state[dsv::mod::StochasticLotkaVolterra_Model::PRESA]      = X1_0;
        s.state[dsv::mod::StochasticLotkaVolterra_Model::DEPREDADOR] = X2_0;
        s.model_ref.vincular(modeloSto);
    }

    // Lambda de reset
    auto resetTodo = [&]() {
        instDet.state[dsv::mod::LotkaVolterra_Model::PRESA]      = X1_0;
        instDet.state[dsv::mod::LotkaVolterra_Model::DEPREDADOR] = X2_0;
        instDet.t = 0.0f;
        for (auto& s : simsSto) {
            s.state[0] = X1_0;
            s.state[1] = X2_0;
            s.t = 0.0f;
        }
    };


    // =========================================================================
    //  PALETA
    //  Presas      --> cian   (agua, abundancia)
    //  Depredadores--> naranja (alerta, escasez)
    //  Det         --> colores solidos (certeza)
    //  Sto MC      --> degradado Oceano (incertidumbre)
    // =========================================================================
    const sf::Color colPresa  = dsv::Color::cian;
    const sf::Color colDepred = dsv::Color::naranja;
    const sf::Color colDetP   = dsv::Color::cian;
    const sf::Color colDetD   = dsv::Color::naranja;

    std::vector<sf::Color> paletaSto(NUM_STO);
    for (int i = 0; i < NUM_STO; ++i)
        paletaSto[i] = dsv::Color::Oceano(i, NUM_STO);


    // =========================================================================
    //  LAYOUT Y TABLERO
    //
    //  6 filas x 8 columnas (layout denso para comparacion directa):
    //
    //  Filas 0-2:  fase Det (izq) | fase Sto 5-MC (der)
    //  Filas 3-4:  tiempo Det     | tiempo Sto
    //  Fila  5:    comparativa presas superpuesta | comparativa depredadores
    // =========================================================================
    dsv::Layout miLayout = {
        "fD fD fD fD fS fS fS fS",
        "fD fD fD fD fS fS fS fS",
        "fD fD fD fD fS fS fS fS",
        "tD tD tD tD tS tS tS tS",
        "tD tD tD tD tS tS tS tS",
        "cP cP cP cP cD cD cD cD",
    };
    dsv::Tablero tablero(window, miLayout,
                         sf::Color(10, 14, 10), sf::Color(5, 7, 5));


    // =========================================================================
    //  ESPACIO DE FASE -- DETERMINISTICO
    //  Con RK4 las orbitas son elipses cerradas que se repiten indefinidamente.
    //  Con Euler, la orbita espirala hacia afuera (ganancia de energia falsa).
    //  La cantidad de Lyapunov V se conserva con RK4: las orbitas son isoV.
    // =========================================================================
    dsv::Vista<dsv::EspacioFase2D> faseD =
        tablero.add<dsv::EspacioFase2D>(
            "Fase DETERMINISTICO (Presas, Depredadores) -- orbitas cerradas",
            colPresa, "fD", colPresa);

    faseD->activarSeguimiento(false);
    faseD->activarAutoescalado(true);
    faseD->ponerDesvanecido(false);   // cola completa: ver la orbita entera
    faseD->ponerCabeza(true);
    faseD->configurarMaxPoints(6000);


    // =========================================================================
    //  ESPACIO DE FASE -- ESTOCASTICO (5 trayectorias Monte Carlo)
    //  Las orbitas son perturbadas por el ruido. A sigma alto, algunas
    //  trayectorias pueden colapsar al origen (extincion irreversible).
    // =========================================================================
    dsv::Vista<dsv::EspacioFase2D> faseS =
        tablero.add<dsv::EspacioFase2D>(
            "Fase ESTOCASTICO (5 MC) -- extincion posible con sigma alto",
            colPresa, "fS");

    faseS->activarSeguimiento(false);
    faseS->activarAutoescalado(true);
    faseS->ponerDesvanecido(true);    // cola desvanecida: ver la incertidumbre
    faseS->ponerCabeza(true);
    faseS->configurarMaxPoints(2000);

    for (int i = 0; i < NUM_STO; ++i)
        faseS->agregarSerie("S" + std::to_string(i), paletaSto[i]);


    // =========================================================================
    //  TIEMPO -- DETERMINISTICO (presas + depredadores)
    //  Oscilaciones periodicas perfectas. Periodo T ~= 2*pi/sqrt(alpha*gamma).
    //  El pico de depredadores siempre sigue al pico de presas con retraso T/4.
    // =========================================================================
    dsv::Vista<dsv::GraficaTiempo> tDet =
        tablero.add<dsv::GraficaTiempo>(
            "Tiempo DETERMINISTICO -- Presas (cian) y Depredadores (naranja)",
            colPresa, "tD");

    tDet->agregarSerie("Presas Det",      colDetP);
    tDet->agregarSerie("Depredadores Det",colDetD);
    tDet.objeto.configurarVentanaTiempo(80.0f);
    tDet.objeto.ponerSombreado(false);


    // =========================================================================
    //  TIEMPO -- ESTOCASTICO (todas las instancias MC)
    //  La "banda" de colores es la distribucion de probabilidad en tiempo real.
    //  Una trayectoria que cae a cero y no sube: extincion.
    // =========================================================================
    dsv::Vista<dsv::GraficaTiempo> tSto =
        tablero.add<dsv::GraficaTiempo>(
            "Tiempo ESTOCASTICO -- banda MC: presas (solido) dep (suave)",
            colPresa, "tS");

    tSto.objeto.configurarVentanaTiempo(80.0f);
    tSto.objeto.ponerSombreado(false);

    for (int i = 0; i < NUM_STO; ++i) {
        sf::Color cP = paletaSto[i]; cP.a = (i == 0) ? 255 : 100;
        sf::Color cD = colDepred;    cD.a = (i == 0) ? 200 :  60;
        tSto->agregarSerie("P_" + std::to_string(i), cP);
        tSto->agregarSerie("D_" + std::to_string(i), cD);
    }


    // =========================================================================
    //  COMPARATIVA DIRECTA -- Presas Det (solido) vs Presas Sto MC (suave)
    //  Ver si el ruido desplaza la media o solo agrega varianza.
    //  Teoricamente, la media del proceso SDE LV puede derivar hacia la extincion.
    // =========================================================================
    dsv::Vista<dsv::GraficaTiempo> cmpP =
        tablero.add<dsv::GraficaTiempo>(
            "Comparativa PRESAS: Det (cian solido) vs Sto MC (difuminado)",
            colPresa, "cP");

    cmpP.objeto.configurarVentanaTiempo(120.0f);
    cmpP.objeto.ponerSombreado(false);

    // Det: unica curva, solida y gruesa
    cmpP->agregarSerie("Presas_Det", colDetP);
    // Sto MC: transparentes
    for (int i = 0; i < NUM_STO; ++i) {
        sf::Color c = paletaSto[i]; c.a = 80;
        cmpP->agregarSerie("Presas_S" + std::to_string(i), c);
    }


    dsv::Vista<dsv::GraficaTiempo> cmpD =
        tablero.add<dsv::GraficaTiempo>(
            "Comparativa DEPREDADORES: Det (naranja solido) vs Sto MC",
            colDepred, "cD");

    cmpD.objeto.configurarVentanaTiempo(120.0f);
    cmpD.objeto.ponerSombreado(false);

    cmpD->agregarSerie("Dep_Det", colDetD);
    for (int i = 0; i < NUM_STO; ++i) {
        sf::Color c = colDepred; c.a = 70;
        cmpD->agregarSerie("Dep_S" + std::to_string(i), c);
    }

    // Degradado naturaleza oscura
    tablero.setPanelDegradado(sf::Color(10, 14, 10), sf::Color(5, 7, 5));


    // =========================================================================
    //  PANEL ECOLOGICO (izquierda) -- parametros compartidos por ambos modelos
    //  Los sliders apuntan al modeloDet; el bucle copia al modeloSto.
    // =========================================================================
    dsv::PanelFlotante PanelEco(
        window, "Eco",
        dsv::HandleOrientacion::Vertical,
        dsv::DespliegueDir::Der,
        dsv::Color::verde
    );
    PanelEco.setDegradado(sf::Color(8, 28, 12, 245), sf::Color(4, 14, 6, 245));
    PanelEco.positionAbsoluta(dsv::Ubicacion::ArribaIzq);

    dsv::MenuFlotante* menuEco = PanelEco.crearContenido<dsv::MenuFlotante>(6.f, 14.f);

    dsv::CampoControl& pe_t = menuEco->agregarFila();
    pe_t.agregar<dsv::CampoTexto>("-- Parametros Ecologicos --");

    dsv::CampoControl& pe_sep = menuEco->agregarFila();
    pe_sep.agregar<dsv::CampoTexto>("(aplican a Det y Sto)");

    // Barras de estado del deterministico
    dsv::CampoControl& pe_bP = menuEco->agregarFila();
    pe_bP.agregar<dsv::CampoBarra>("Presas  Det",
        &instDet.state[dsv::mod::LotkaVolterra_Model::PRESA],
        0.f, 40.f, colPresa);

    dsv::CampoControl& pe_bD = menuEco->agregarFila();
    pe_bD.agregar<dsv::CampoBarra>("Depr.   Det",
        &instDet.state[dsv::mod::LotkaVolterra_Model::DEPREDADOR],
        0.f, 20.f, colDepred);

    // Barras de estado de la primera instancia estocastica
    dsv::CampoControl& pe_bPS = menuEco->agregarFila();
    pe_bPS.agregar<dsv::CampoBarra>("Presas  Sto[0]",
        &simsSto[0].state[0], 0.f, 40.f, paletaSto[0]);

    dsv::CampoControl& pe_bDS = menuEco->agregarFila();
    pe_bDS.agregar<dsv::CampoBarra>("Depr.   Sto[0]",
        &simsSto[0].state[1], 0.f, 20.f, paletaSto[NUM_STO-1]);

    dsv::CampoControl& pe_sep2 = menuEco->agregarFila();
    pe_sep2.agregar<dsv::CampoTexto>("-- Interacciones --");

    // Los sliders apuntan al modeloDet; se sincronizan al modeloSto en el bucle
    dsv::CampoControl& pe_alpha = menuEco->agregarFila();
    pe_alpha.agregar<dsv::CampoDeslizador>("alpha Natalidad Presas",
        &modeloDet.alpha, 0.1f, 3.0f);

    dsv::CampoControl& pe_beta = menuEco->agregarFila();
    pe_beta.agregar<dsv::CampoDeslizador>("beta  Depredacion",
        &modeloDet.beta, 0.01f, 1.5f);

    dsv::CampoControl& pe_delta = menuEco->agregarFila();
    pe_delta.agregar<dsv::CampoDeslizador>("delta Ef. Alimenticia",
        &modeloDet.delta, 0.01f, 0.8f);

    dsv::CampoControl& pe_gamma = menuEco->agregarFila();
    pe_gamma.agregar<dsv::CampoDeslizador>("gamma Mort. Depredador",
        &modeloDet.gamma, 0.01f, 2.0f);

    // Punto de equilibrio calculado en vivo
    float eq_x1 = 0.0f, eq_x2 = 0.0f;
    dsv::CampoControl& pe_eq1 = menuEco->agregarFila();
    pe_eq1.agregar<dsv::CampoVariable>("Eq. Presas   = g/d", &eq_x1);

    dsv::CampoControl& pe_eq2 = menuEco->agregarFila();
    pe_eq2.agregar<dsv::CampoVariable>("Eq. Depr.    = a/b", &eq_x2);

    dsv::CampoControl& pe_res = menuEco->agregarFila();
    pe_res.agregar<dsv::CampoBoton>("Reset [R]",
        [&]{ resetTodo(); }, dsv::Color::verde);


    // =========================================================================
    //  PANEL RUIDO (derecha) -- solo para el modelo estocastico
    // =========================================================================
    dsv::PanelFlotante PanelRuido(
        window, "Ruido",
        dsv::HandleOrientacion::Vertical,
        dsv::DespliegueDir::Izq,
        dsv::Color::naranja
    );
    PanelRuido.setDegradado(sf::Color(35, 18, 5, 245), sf::Color(18, 9, 3, 245));
    PanelRuido.positionAbsoluta(dsv::Ubicacion::CentroDer);

    dsv::MenuFlotante* menuRuido = PanelRuido.crearContenido<dsv::MenuFlotante>(6.f, 14.f);

    dsv::CampoControl& pr_t = menuRuido->agregarFila();
    pr_t.agregar<dsv::CampoTexto>("-- Ruido Ambiental SDE --");

    dsv::CampoControl& pr_sep = menuRuido->agregarFila();
    pr_sep.agregar<dsv::CampoTexto>("(solo modelo estocastico)");

    dsv::CampoControl& pr_s1 = menuRuido->agregarFila();
    pr_s1.agregar<dsv::CampoDeslizador>("sigma1  Ruido Presas",
        &modeloSto.sigma1, 0.0f, 0.8f);

    dsv::CampoControl& pr_s2 = menuRuido->agregarFila();
    pr_s2.agregar<dsv::CampoDeslizador>("sigma2  Ruido Depr.",
        &modeloSto.sigma2, 0.0f, 0.8f);

    dsv::CampoControl& pr_sep2 = menuRuido->agregarFila();
    pr_sep2.agregar<dsv::CampoTexto>("-- Escenarios --");

    // Escenario: sin ruido (verificar equivalencia con deterministico)
    dsv::CampoControl& pr_calm = menuRuido->agregarFila();
    pr_calm.agregar<dsv::CampoBoton>("Sin Ruido (verificar)", [&]{
        modeloSto.sigma1 = 0.0f;
        modeloSto.sigma2 = 0.0f;
    }, dsv::Color::verde, dsv::Color::blanco, 150.f);

    // Escenario: ruido moderado (fluctuaciones visibles, no extincion rapida)
    dsv::CampoControl& pr_mod = menuRuido->agregarFila();
    pr_mod.agregar<dsv::CampoBoton>("Ruido Moderado", [&]{
        modeloSto.sigma1 = 0.10f;
        modeloSto.sigma2 = 0.10f;
    }, dsv::Color::naranja, dsv::Color::blanco, 150.f);


    // Escenario: ruido severo (alta probabilidad de extincion)
    dsv::CampoControl& pr_sev = menuRuido->agregarFila();
    pr_sev.agregar<dsv::CampoBoton>("Ruido Severo", [&]{
        modeloSto.sigma1 = 0.50f;
        modeloSto.sigma2 = 0.40f;
    }, dsv::Color::rojo, dsv::Color::blanco, 150.f);


    // Escenario: crisis de depredadores (solo ruido en presas)
    dsv::CampoControl& pr_crP = menuRuido->agregarFila();
    pr_crP.agregar<dsv::CampoBoton>("Crisis Presas", [&]{
        modeloSto.sigma1 = 0.60f;
        modeloSto.sigma2 = 0.05f;
    }, colPresa, dsv::Color::blanco, 150.f);


    // Escenario: crisis de depredadores
    dsv::CampoControl& pr_crD = menuRuido->agregarFila();
    pr_crD.agregar<dsv::CampoBoton>("Crisis Depredadores", [&]{
        modeloSto.sigma1 = 0.05f;
        modeloSto.sigma2 = 0.60f;
    }, colDepred, dsv::Color::blanco, 150.f);


    // Valores actuales de sigma en vivo
    dsv::CampoControl& pr_vs1 = menuRuido->agregarFila();
    pr_vs1.agregar<dsv::CampoVariable>("sigma1 actual =", &modeloSto.sigma1);

    dsv::CampoControl& pr_vs2 = menuRuido->agregarFila();
    pr_vs2.agregar<dsv::CampoVariable>("sigma2 actual =", &modeloSto.sigma2);


    // =========================================================================
    //  PANEL TIEMPO (arriba centro)
    // =========================================================================
    bool  pausa     = true;
    float timeScale = 1.0f;
    sf::Clock fpsClock;
    long long frameCount = 0;
    float fps    = 0.0f;
    float tiempo = 0.0f;

    const sf::Color colorMENU = dsv::Color::celeste;

    dsv::PanelFlotante PanelTiempo(
        window, "Menu Tiempo",
        dsv::HandleOrientacion::Horizontal,
        dsv::DespliegueDir::Abajo,
        colorMENU
    );
    PanelTiempo.setDegradado(sf::Color(8, 20, 30, 230), sf::Color(4, 10, 16, 230));
    PanelTiempo.positionAbsoluta(dsv::Ubicacion::ArribaCentro);

    dsv::MenuFlotante* menuTiempo = PanelTiempo.crearContenido<dsv::MenuFlotante>(6.f, 12.f);

    dsv::CampoControl& mt_f1 = menuTiempo->agregarFila();
    mt_f1.agregar<dsv::CampoTexto>("-- Lotka-Volterra Det vs SDE --");

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


    // =========================================================================
    //  BUCLE PRINCIPAL
    // =========================================================================
    sf::Clock clock;
    sf::Time  accumulator = sf::Time::Zero;
    // ups = 0.005f: 200 updates/s
    // Para LV deterministico: RK4 conserva la orbita cerrada (V = cte).
    // Para LV estocastico: EM_step (noise_dim=2) es correcto por definicion.
    sf::Time  ups = sf::seconds(0.005f);

    while (window.isOpen()) {

        // --- Eventos ----------------------------------------------------------
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window.close();
            PanelTiempo.gestionarEvento(event);
            PanelEco.gestionarEvento(event);
            PanelRuido.gestionarEvento(event);

            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Space)
                    pausa = !pausa;
                if (event.key.code == sf::Keyboard::Up)
                    timeScale += 0.2f;
                if (event.key.code == sf::Keyboard::Down)
                    timeScale = std::max(0.1f, timeScale - 0.2f);
                if (event.key.code == sf::Keyboard::R) {
                    resetTodo(); tiempo = 0.0f;
                }
            }
        }

        // --- Sincronizar parametros ecologicos al modelo estocastico ----------
        // Los sliders apuntan a modeloDet; copiamos aqui para comparacion justa.
        // sigma1 y sigma2 los gestiona el Panel de Ruido directamente.
        modeloSto.alpha = modeloDet.alpha;
        modeloSto.beta  = modeloDet.beta;
        modeloSto.delta = modeloDet.delta;
        modeloSto.gamma = modeloDet.gamma;

        // Punto de equilibrio calculado en vivo
        // x1* = gamma/delta,  x2* = alpha/beta
        eq_x1 = (modeloDet.delta > 0.0f) ? modeloDet.gamma / modeloDet.delta : 0.0f;
        eq_x2 = (modeloDet.beta  > 0.0f) ? modeloDet.alpha / modeloDet.beta  : 0.0f;

        // --- Simulacion -------------------------------------------------------
        sf::Time elapsed = clock.restart();
        if (!pausa)
            accumulator += elapsed * timeScale;

        while (accumulator >= ups) {
            float dt = ups.asSeconds();

            // DETERMINISTICO: RK4 obligatorio.
            // LV conserva la cantidad de Lyapunov V(x1,x2).
            // Euler introduce drift de energia: la orbita espirala falsamente.
            // Con RK4 y dt=0.005, la conservacion de V es ~1e-6 por periodo.
            dsv::sim::RK4_step(instDet, dt);

            // ESTOCASTICO: sim::step detecta noise_dim=2 y llama EM_step
            // (Euler-Maruyama con matriz de difusion 2x2 diagonal).
            // Cada instancia genera 2 dW independientes por paso.
            for (auto& s : simsSto)
                dsv::sim::step(s, dt);

            tiempo = instDet.t;
            accumulator -= ups;
        }

        // --- Alimentar graficas -----------------------------------------------
        if (!pausa) {
            using LV = dsv::mod::LotkaVolterra_Model;
            using SLV = dsv::mod::StochasticLotkaVolterra_Model;

            const float p_det = instDet.state[LV::PRESA];
            const float d_det = instDet.state[LV::DEPREDADOR];
            const float t_det = instDet.t;

            // Espacio de fase deterministico (monoserie)
            faseD->push_back(p_det, d_det);

            // Tiempo deterministico
            tDet->push_back(p_det, t_det, "Presas Det");
            tDet->push_back(d_det, t_det, "Depredadores Det");

            // Comparativa: curva Det solida
            cmpP->push_back(p_det, t_det, "Presas_Det");
            cmpD->push_back(d_det, t_det, "Dep_Det");

            // Estocastico: todas las instancias MC
            for (int i = 0; i < NUM_STO; ++i) {
                const std::string si = std::to_string(i);
                float p_sto = simsSto[i].state[SLV::PRESA];
                float d_sto = simsSto[i].state[SLV::DEPREDADOR];
                float t_sto = simsSto[i].t;

                faseS->push_back(p_sto, d_sto, "S" + si);

                tSto->push_back(p_sto, t_sto, "P_" + si);
                tSto->push_back(d_sto, t_sto, "D_" + si);

                cmpP->push_back(p_sto, t_sto, "Presas_S" + si);
                cmpD->push_back(d_sto, t_sto, "Dep_S"    + si);
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
        PanelEco.draw();
        PanelRuido.draw();
        window.display();
    }

    return 0;
}