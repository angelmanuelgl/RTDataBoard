/*
 * proyecto: DynSysVis RT -- Poblacion de Tortugas Marinas (SDE)
 *
 * Modelo de tres etapas: Juveniles (J), Sub-adultos (S), Adultos (A)
 * Basado en Caretta caretta (tortuga boba), parametros tipicos de la literatura.
 *
 * Por que un SDE para poblaciones:
 *   El ruido ambiental (temperatura del oceano, disponibilidad de alimento,
 *   tormentas) afecta las tasas de supervivencia de forma estocastica.
 *   sigma[i] modela la volatilidad de la tasa de supervivencia de la etapa i.
 *   El ruido es multiplicativo (escala con la poblacion): sigma[i] * x[i] * dW.
 *   Esto garantiza que si x[i] = 0 (extincion), el ruido se anula y no
 *   "resucita" la poblacion artificialmente -- comportamiento biologicamente correcto.
 *
 * Estructura demografica (modelo de Leslie estocastico lineal):
 *   dJ/dt = f*A - (d1 + gamma1)*J           + sigma[0]*J*dW1
 *   dS/dt = gamma1*J - (d2 + gamma2)*S       + sigma[1]*S*dW2
 *   dA/dt = gamma2*S - d3*A                  + sigma[2]*A*dW3
 *
 * Nota sobre noise_dim=3 y el integrador:
 *   dsv::sim::step() detecta noise_dim > 1 y llama EM_step (Euler-Maruyama
 *   matricial). La matriz de difusion G es diagonal (3x3): cada etapa tiene
 *   su propio proceso de Wiener independiente.
 *   IMPORTANTE: la firma de diffusion() usa std::array<std::array<float,m>,d>
 *   (matriz anidada) para ser compatible con EM_step de la libreria -- distinta
 *   a la version plana del enunciado que se adapta aqui.
 *
 * Analisis de viabilidad (R0 demografico):
 *   R0 = f * gamma1/(d2+gamma2) * gamma2/d3 / (d1+gamma1)
 *   R0 > 1: la poblacion crece en ausencia de ruido.
 *   Con los defaults: R0 = 0.5 * (0.15/0.15) * (0.05/0.05) / 0.35 ~= 1.43
 *
 * Layout:
 *
 *   +----+----+----+----+------+------+
 *   |                  |             |
 *   |  Espacio 3D      |  all: J,S,A |
 *   |  (J, S, A)       |  multiserie |
 *   |  5 trayectorias  |             |
 *   |  Monte Carlo     +------+------+
 *   |                  | tJ   | tS   |
 *   +------------------+------+------+
 *   |  tA (adultos)    |  pie chart  |
 *   +------------------+-------------+
 *
 *  -- Espacio 3D: las 5 trayectorias Monte Carlo divergen lentamente
 *    (el ruido en tortugas es bajo, sigma ~ 0.05-0.10).
 *    El atractor estocastico es una nube alrededor del equilibrio deterministico.
 *  -- all multiserie: la banda de incertidumbre crece con el tiempo.
 *  -- tJ, tS, tA: spread de las 5 instancias por etapa.
 *  -- pie: fraccion actual de cada etapa (estructura de edades en vivo).
 */

#include <SFML/Graphics.hpp>
#include <cmath>
#include <algorithm>
#include <string>
#include <vector>
#include <array>

#include "DynSysVis.hpp"


// =============================================================================
//  MODELO SDE -- Poblacion de Tortugas Marinas
//
//  NOTA: diffusion() usa la firma std::array<std::array<float,m>,d> (anidada)
//  requerida por EM_step de dsv. La version del enunciado (plana) se adapta aqui.
// =============================================================================
namespace dsv { namespace mod {

struct SeaTurtlePopulation_SDE {

    static constexpr size_t dim       = 3; // J, S, A
    static constexpr size_t noise_dim = 3; // un ruido independiente por etapa

    // Parametros biologicos (Caretta caretta)
    float f      = 0.50f;  // fecundidad: huevos viables por adulto por unidad de tiempo
    float gamma1 = 0.15f;  // tasa de transicion J -> S (madurez juvenil)
    float gamma2 = 0.05f;  // tasa de transicion S -> A (madurez sexual)
    float d1     = 0.20f;  // mortalidad juvenil
    float d2     = 0.10f;  // mortalidad sub-adulta
    float d3     = 0.05f;  // mortalidad adulta

    // Intensidades de ruido ambiental (una por etapa)
    // sigma[0]: volatilidad de supervivencia juvenil (mayor: mas exposicion)
    // sigma[1]: volatilidad de supervivencia sub-adulta
    // sigma[2]: volatilidad de supervivencia adulta (menor: mas robustos)
    std::array<float, noise_dim> sigma = { 0.10f, 0.08f, 0.05f };

    enum { J, S, A };

    // Parte determinista: modelo de Leslie lineal
    void drift(const std::array<float, dim>& x,
               float /*t*/,
               std::array<float, dim>& out) const
    {
        // dJ/dt = fecundidad de adultos - (mortalidad + transicion a S)
        out[J] = f * x[A] - (d1 + gamma1) * x[J];

        // dS/dt = llegada desde J - (mortalidad + transicion a A)
        out[S] = gamma1 * x[J] - (d2 + gamma2) * x[S];

        // dA/dt = llegada desde S - mortalidad adulta
        out[A] = gamma2 * x[S] - d3 * x[A];
    }

    // Matriz de difusion G(x) -- firma compatible con EM_step de dsv
    // G es diagonal: cada etapa tiene su propio canal de ruido
    // G[i][k] = sigma[i] * x[i]  si i == k, 0 si no
    // Ruido multiplicativo: la volatilidad escala con la poblacion.
    // Cuando x[i] -> 0, el ruido se anula: extincion es un estado absorbente.
    void diffusion(const std::array<float, dim>& x,
                   float /*t*/,
                   std::array<std::array<float, noise_dim>, dim>& out) const
    {
        // Inicializar toda la matriz a cero
        for (auto& fila : out) fila.fill(0.0f);

        // Solo la diagonal: canal k solo afecta a etapa k
        out[J][0] = sigma[0] * std::max(0.0f, x[J]); // ruido en juveniles
        out[S][1] = sigma[1] * std::max(0.0f, x[S]); // ruido en sub-adultos
        out[A][2] = sigma[2] * std::max(0.0f, x[A]); // ruido en adultos
    }
};

}} // namespace dsv::mod


// =============================================================================
//  CONSTANTES
// =============================================================================
static constexpr int   NUM_SIMS = 5;

// Condiciones iniciales (individuos)
// Tipicas para una poblacion amenazada de Caretta caretta en zona protegida
static constexpr float J0 = 800.0f;
static constexpr float S0 = 200.0f;
static constexpr float A0 = 60.0f;


// =============================================================================
//  MAIN
// =============================================================================
int main() {

    // --- Init -----------------------------------------------------------------
    dsv::Color::cargar("assets/config/colores.txt");
    sf::RenderWindow window;
    dsv::Sistema::inicializarVentana(window,
        "DynSysVis RT -- Tortugas Marinas: Dinamica Poblacional SDE");


    // =========================================================================
    //  MODELO BASE (compartido por todas las instancias)
    // =========================================================================
    dsv::mod::SeaTurtlePopulation_SDE modeloBase;


    // =========================================================================
    //  5 INSTANCIAS MONTE CARLO (mismo modelo, mismas CI exactas)
    //  La unica fuente de diferencia es el ruido: cada instancia genera
    //  sus propios incrementos brownianos independientes.
    // =========================================================================
    using TurtleInst = dsv::mod::Instance<dsv::mod::SeaTurtlePopulation_SDE>;
    std::vector<TurtleInst> sims(NUM_SIMS);

    for (auto& s : sims) {
        s.state[dsv::mod::SeaTurtlePopulation_SDE::J] = J0;
        s.state[dsv::mod::SeaTurtlePopulation_SDE::S] = S0;
        s.state[dsv::mod::SeaTurtlePopulation_SDE::A] = A0;
        s.model_ref.vincular(modeloBase);
    }

    auto& m = sims[0].getModel();

    // Lambda de reset
    auto resetTodo = [&]() {
        for (auto& s : sims) {
            s.state[dsv::mod::SeaTurtlePopulation_SDE::J] = J0;
            s.state[dsv::mod::SeaTurtlePopulation_SDE::S] = S0;
            s.state[dsv::mod::SeaTurtlePopulation_SDE::A] = A0;
            s.t = 0.0f;
        }
    };


    // =========================================================================
    //  PALETA MARINA
    //  Juveniles  --> cian  (jovenes, brillantes)
    //  Sub-adultos--> azul  (profundidad media)
    //  Adultos    --> verde_mar / esmeralda (maduros)
    // =========================================================================
    const sf::Color colJ = dsv::Color::cian;
    const sf::Color colS = dsv::Color::azul;
    const sf::Color colA = dsv::Color::verde;

    // Paleta por instancia: variaciones del Oceano para el spread Monte Carlo
    std::vector<sf::Color> paletaMC(NUM_SIMS);
    for (int i = 0; i < NUM_SIMS; ++i)
        paletaMC[i] = dsv::Color::Oceano(i, NUM_SIMS);


    // =========================================================================
    //  LAYOUT Y TABLERO
    //
    //  Fila 0-1 (2/3): 3D (grande) | all multiserie
    //  Fila 2   (1/6): 3D continua | tJ | tS
    //  Fila 3   (1/6): tA (ancha)  | pie
    // =========================================================================
    dsv::Layout miLayout = {
        "3d 3d 3d 3d all all all",
        "3d 3d 3d 3d all all all",
        "3d 3d 3d 3d tJ  tJ  tS ",
        "tA tA tA tA tA  pie pie",
    };
    dsv::Tablero tablero(window, miLayout,
                         sf::Color(8, 15, 25), sf::Color(4, 8, 14));


    // =========================================================================
    //  GRAFICA 3D -- atractor estocastico en (J, S, A)
    //  Las 5 trayectorias Monte Carlo forman una "nube" alrededor del
    //  equilibrio deterministico. A mayor sigma, mayor dispersion.
    // =========================================================================
    dsv::Vista<dsv::Grafica3D> fase3D =
        tablero.add<dsv::Grafica3D>(
            "Atractor Estocastico (J, S, A) -- 5 trayectorias MC",
            dsv::Color::cian, "3d");

    fase3D.objeto.getEjes().setLimites(0.0f, 1200.0f);
    fase3D.objeto.getEjes().colorX = colJ;
    fase3D.objeto.getEjes().colorY = colS;
    fase3D.objeto.getEjes().colorZ = colA;

    {
        auto& g = fase3D.objeto.getGestor();
        g.setMaxPointsSeries(3000);
        g.setGrosorSeries(1.5f);
        g.setDifuminadoSeries(true);
        g.setAdelgazadoSeries(false);
        for (int i = 0; i < NUM_SIMS; ++i)
            g.agregarSerie("MC" + std::to_string(i), paletaMC[i]);
    }


    // =========================================================================
    //  GRAFICATIEMPO -- todas las etapas, todas las instancias (spread MC)
    //  "all": 5 series de J (cian), 5 de S (azul), 5 de A (verde)
    //  La "banda" de cada color es la incertidumbre por el ruido ambiental.
    // =========================================================================
    dsv::Vista<dsv::GraficaTiempo> grafAll =
        tablero.add<dsv::GraficaTiempo>(
            "J(t), S(t), A(t) -- banda de incertidumbre Monte Carlo",
            dsv::Color::celeste, "all");

    grafAll.objeto.configurarVentanaTiempo(150.0f);
    grafAll.objeto.ponerSombreado(false);

    for (int i = 0; i < NUM_SIMS; ++i) {
        sf::Color cJ = colJ; cJ.a = (i == 0) ? 255 : 90;
        sf::Color cS = colS; cS.a = (i == 0) ? 255 : 90;
        sf::Color cA = colA; cA.a = (i == 0) ? 255 : 90;
        grafAll->agregarSerie("J_" + std::to_string(i), cJ);
        grafAll->agregarSerie("S_" + std::to_string(i), cS);
        grafAll->agregarSerie("A_" + std::to_string(i), cA);
    }


    // =========================================================================
    //  TIEMPO POR ETAPA -- spread de las 5 instancias
    //  Ver como el ruido separa trayectorias que empezaron identicas.
    // =========================================================================

    // Juveniles
    dsv::Vista<dsv::GraficaTiempo> grafJ =
        tablero.add<dsv::GraficaTiempo>(
            "Juveniles J(t) -- spread MC (sigma[0])",
            dsv::Color::cian, "tJ");
    grafJ.objeto.configurarVentanaTiempo(100.0f);
    grafJ.objeto.ponerSombreado(false);
    for (int i = 0; i < NUM_SIMS; ++i) {
        sf::Color c = colJ; c.a = (i == 0) ? 255 : 110;
        grafJ->agregarSerie("J_" + std::to_string(i), c);
    }

    // Sub-adultos
    dsv::Vista<dsv::GraficaTiempo> grafS =
        tablero.add<dsv::GraficaTiempo>(
            "Sub-adultos S(t) -- spread MC (sigma[1])",
            dsv::Color::azul, "tS");
    grafS.objeto.configurarVentanaTiempo(100.0f);
    grafS.objeto.ponerSombreado(false);
    for (int i = 0; i < NUM_SIMS; ++i) {
        sf::Color c = colS; c.a = (i == 0) ? 255 : 110;
        grafS->agregarSerie("S_" + std::to_string(i), c);
    }

    // Adultos (panel ancho en la fila inferior)
    dsv::Vista<dsv::GraficaTiempo> grafA =
        tablero.add<dsv::GraficaTiempo>(
            "Adultos A(t) -- spread MC (sigma[2]) -- componente mas lenta",
            dsv::Color::verde, "tA");
    grafA.objeto.configurarVentanaTiempo(100.0f);
    grafA.objeto.ponerSombreado(true);  // sombreado: adultos son el recurso critico
    for (int i = 0; i < NUM_SIMS; ++i) {
        sf::Color c = colA; c.a = (i == 0) ? 255 : 110;
        grafA->agregarSerie("A_" + std::to_string(i), c);
    }


    // =========================================================================
    //  PIE CHART -- estructura de edades en tiempo real
    //  Muestra la proporcion J:S:A de la instancia 0.
    //  En una poblacion sana: J > S > A (piramide demografica normal).
    //  Una inversion (A > J) indica colapso reproductivo inminente.
    // =========================================================================
    dsv::Vista<dsv::GraficoCircular> pie =
        tablero.add<dsv::GraficoCircular>("Estructura de Edades [J:S:A]", dsv::Color::aqua, "pie");
    pie->personalizarColores({colJ, colS, colA});

    // Degradado oceano profundo
    tablero.setPanelDegradado(sf::Color(8, 15, 25), sf::Color(4, 8, 14));


    // =========================================================================
    //  PANEL BIOLOGICO (izquierda) -- parametros de historia de vida
    // =========================================================================
    dsv::PanelFlotante PanelBio(
        window, "Bio",
        dsv::HandleOrientacion::Vertical,
        dsv::DespliegueDir::Der,
        dsv::Color::verde
    );
    PanelBio.setDegradado(sf::Color(5, 30, 20, 245), sf::Color(3, 15, 10, 245));
    PanelBio.positionAbsoluta(dsv::Ubicacion::CentroIzq);

    dsv::MenuFlotante* menuBio = PanelBio.crearContenido<dsv::MenuFlotante>(6.f, 14.f);

    dsv::CampoControl& pb_t = menuBio->agregarFila();
    pb_t.agregar<dsv::CampoTexto>("-- Historia de Vida --");

    // Barras de estado de la instancia 0
    dsv::CampoControl& pb_bJ = menuBio->agregarFila();
    pb_bJ.agregar<dsv::CampoBarra>("J  Juveniles",   &sims[0].state[0], 0.f, 2000.f, colJ);

    dsv::CampoControl& pb_bS = menuBio->agregarFila();
    pb_bS.agregar<dsv::CampoBarra>("S  Sub-adultos", &sims[0].state[1], 0.f, 1000.f, colS);

    dsv::CampoControl& pb_bA = menuBio->agregarFila();
    pb_bA.agregar<dsv::CampoBarra>("A  Adultos",     &sims[0].state[2], 0.f, 500.f,  colA);

    dsv::CampoControl& pb_sep1 = menuBio->agregarFila();
    pb_sep1.agregar<dsv::CampoTexto>("-- Tasas Biologicas --");

    // Fecundidad
    dsv::CampoControl& pb_f = menuBio->agregarFila();
    pb_f.agregar<dsv::CampoDeslizador>("f  Fecundidad",   &m.f,      0.0f, 2.0f);

    // Transiciones
    dsv::CampoControl& pb_g1 = menuBio->agregarFila();
    pb_g1.agregar<dsv::CampoDeslizador>("gamma1  J->S",   &m.gamma1, 0.001f, 0.5f);

    dsv::CampoControl& pb_g2 = menuBio->agregarFila();
    pb_g2.agregar<dsv::CampoDeslizador>("gamma2  S->A",   &m.gamma2, 0.001f, 0.3f);

    // Mortalidades
    dsv::CampoControl& pb_d1 = menuBio->agregarFila();
    pb_d1.agregar<dsv::CampoDeslizador>("d1  Mort. J",    &m.d1,     0.0f, 0.8f);

    dsv::CampoControl& pb_d2 = menuBio->agregarFila();
    pb_d2.agregar<dsv::CampoDeslizador>("d2  Mort. S",    &m.d2,     0.0f, 0.5f);

    dsv::CampoControl& pb_d3 = menuBio->agregarFila();
    pb_d3.agregar<dsv::CampoDeslizador>("d3  Mort. A",    &m.d3,     0.0f, 0.3f);

    // R0 demografico en vivo: viabilidad de la poblacion
    float R0_display = 0.0f;
    dsv::CampoControl& pb_R0 = menuBio->agregarFila();
    pb_R0.agregar<dsv::CampoVariable>("R0 =", &R0_display);

    dsv::CampoControl& pb_res = menuBio->agregarFila();
    pb_res.agregar<dsv::CampoBoton>("Reset C.I.", [&]{
        resetTodo();
    }, dsv::Color::verde);


    // =========================================================================
    //  PANEL AMBIENTAL (derecha) -- intensidades de ruido por etapa
    // =========================================================================
    dsv::PanelFlotante PanelAmb(
        window, "Ruido",
        dsv::HandleOrientacion::Vertical,
        dsv::DespliegueDir::Izq,
        dsv::Color::cian
    );
    PanelAmb.setDegradado(sf::Color(5, 20, 35, 245), sf::Color(3, 10, 18, 245));
    PanelAmb.positionAbsoluta(dsv::Ubicacion::CentroDer);

    dsv::MenuFlotante* menuAmb = PanelAmb.crearContenido<dsv::MenuFlotante>(6.f, 14.f);

    dsv::CampoControl& pa_t = menuAmb->agregarFila();
    pa_t.agregar<dsv::CampoTexto>("-- Ruido Ambiental --");

    dsv::CampoControl& pa_sep = menuAmb->agregarFila();
    pa_sep.agregar<dsv::CampoTexto>("(sigma: volatilidad por etapa)");

    // Ruido por etapa -- apunta a m.sigma[i] directamente
    dsv::CampoControl& pa_s0 = menuAmb->agregarFila();
    pa_s0.agregar<dsv::CampoDeslizador>("sigma[0]  Juveniles",   &m.sigma[0], 0.0f, 0.5f);

    dsv::CampoControl& pa_s1 = menuAmb->agregarFila();
    pa_s1.agregar<dsv::CampoDeslizador>("sigma[1]  Sub-adultos", &m.sigma[1], 0.0f, 0.4f);

    dsv::CampoControl& pa_s2 = menuAmb->agregarFila();
    pa_s2.agregar<dsv::CampoDeslizador>("sigma[2]  Adultos",     &m.sigma[2], 0.0f, 0.3f);

    dsv::CampoControl& pa_sep2 = menuAmb->agregarFila();
    pa_sep2.agregar<dsv::CampoTexto>("-- Escenarios Rapidos --");

    // Escenario: contaminacion (sube sigma[0] drásticamente)
    dsv::CampoControl& pa_cont = menuAmb->agregarFila();
    pa_cont.agregar<dsv::CampoBoton>("Contaminacion [J]", [&]{
        m.sigma[0] = 0.40f; // alta mortalidad juvenil por contaminacion
    }, dsv::Color::rojo_d, dsv::Color::blanco, 130.f);

    // Escenario: clima estable
    dsv::CampoControl& pa_calm = menuAmb->agregarFila();
    pa_calm.agregar<dsv::CampoBoton>("Calma Ambiental", [&]{
        m.sigma[0] = 0.02f;
        m.sigma[1] = 0.02f;
        m.sigma[2] = 0.01f;
    }, dsv::Color::verde_d, dsv::Color::blanco, 130.f);

    // Escenario: tormenta severa
    dsv::CampoControl& pa_storm = menuAmb->agregarFila();
    pa_storm.agregar<dsv::CampoBoton>("Tormenta Severa", [&]{
        m.sigma[0] = 0.45f;
        m.sigma[1] = 0.35f;
        m.sigma[2] = 0.20f;
    }, dsv::Color::naranja_d, dsv::Color::blanco, 130.f);

    // Escenario: conservacion activa (baja mortalidad, bajo ruido)
    dsv::CampoControl& pa_cons = menuAmb->agregarFila();
    pa_cons.agregar<dsv::CampoBoton>("Zona Protegida", [&]{
        m.d1 = 0.05f; m.d2 = 0.03f; m.d3 = 0.02f;
        m.sigma[0] = 0.05f; m.sigma[1] = 0.04f; m.sigma[2] = 0.02f;
    }, dsv::Color::aqua_d, dsv::Color::blanco, 130.f);


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
    PanelTiempo.setDegradado(sf::Color(8, 25, 45, 230), sf::Color(5, 15, 28, 230));
    PanelTiempo.positionAbsoluta(dsv::Ubicacion::ArribaCentro);

    dsv::MenuFlotante* menuTiempo = PanelTiempo.crearContenido<dsv::MenuFlotante>(6.f, 12.f);

    dsv::CampoControl& mt_f1 = menuTiempo->agregarFila();
    mt_f1.agregar<dsv::CampoTexto>("-- Tortugas Marinas SDE --");

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
    // ups = 0.05f: las tortugas tienen dinamica lenta (escala de anos).
    // 1 unidad de tiempo ~= 1 ano; ups=0.05 ~= 18 dias por paso.
    // El integrador EM matricial (EM_step) es llamado automaticamente
    // porque noise_dim=3 > 1 -- el dispatcher de sim::step lo detecta.
    sf::Time  ups = sf::seconds(0.01f);

    while (window.isOpen()) {

        // --- Eventos ----------------------------------------------------------
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window.close();
            PanelTiempo.gestionarEvento(event);
            PanelBio.gestionarEvento(event);
            PanelAmb.gestionarEvento(event);
            fase3D->gestionarEvento(event, window);

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

        // --- Metricas en vivo -------------------------------------------------
        // R0 demografico: producto de supervivencias y transiciones
        // R0 = f * [gamma1/(d1+gamma1)] * [gamma2/(d2+gamma2)] * [1/d3]
        // Simplificado como proxy: f*gamma1*gamma2 / [(d1+gamma1)*(d2+gamma2)*d3]
        float den_J = (m.d1 + m.gamma1);
        float den_S = (m.d2 + m.gamma2);
        R0_display = (den_J > 0.0f && den_S > 0.0f && m.d3 > 0.0f)
                   ? (m.f * m.gamma1 * m.gamma2) / (den_J * den_S * m.d3)
                   : 0.0f;

        // --- Simulacion -------------------------------------------------------
        sf::Time elapsed = clock.restart();
        if (!pausa)
            accumulator += elapsed * timeScale;

        while (accumulator >= ups) {
            float dt = ups.asSeconds();
            // sim::step detecta noise_dim=3 y llama EM_step (Euler-Maruyama
            // con matriz de difusion d x m). Cada instancia genera 3 dW
            // independientes -- el spread Monte Carlo emerge de aqui.
            for (auto& s : sims)
                dsv::sim::step(s, dt);

            tiempo = sims[0].t;
            accumulator -= ups;
        }

        // --- Alimentar graficas -----------------------------------------------
        if (!pausa) {
            using ST = dsv::mod::SeaTurtlePopulation_SDE;

            for (int i = 0; i < NUM_SIMS; ++i) {
                const std::string si = std::to_string(i);
                float J_val = sims[i].state[ST::J];
                float S_val = sims[i].state[ST::S];
                float A_val = sims[i].state[ST::A];
                float t_val = sims[i].t;

                // Graficas 3D
                fase3D.objeto.getGestor().push_back({J_val, S_val, A_val},
                                                    "MC" + si);

                // Multiserie: J, S, A de todas las instancias (banda MC)
                grafAll->push_back(J_val, t_val, "J_" + si);
                grafAll->push_back(S_val, t_val, "S_" + si);
                grafAll->push_back(A_val, t_val, "A_" + si);

                // Series individuales por etapa
                grafJ->push_back(J_val, t_val, "J_" + si);
                grafS->push_back(S_val, t_val, "S_" + si);
                grafA->push_back(A_val, t_val, "A_" + si);
            }

            // Pie chart: estructura de edades de la instancia 0 (referencia)
            pie->push_back({
                sims[0].state[ST::J],
                sims[0].state[ST::S],
                sims[0].state[ST::A]
            });
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
        PanelBio.draw();
        PanelAmb.draw();
        window.display();
    }

    return 0;
}