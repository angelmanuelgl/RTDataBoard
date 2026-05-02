#include <SFML/Graphics.hpp>
#include <cmath>
#include <algorithm>
#include <string>

#include "DynSysVis.hpp"


// =========================================================================
// MAIN
// =========================================================================

int main() {
    // 1. Inicializacion rigurosa
    dsv::Color::cargar("assets/config/colores.txt");
    sf::RenderWindow window;
    dsv::Sistema::inicializarVentana(window, "Laboratorio Comparativo: Malthus vs Logistico");

    // 2. Instanciacion de Modelos Dinamicos
    dsv::mod::Instance<dsv::mod::Malthus_Model> malthus;
    dsv::mod::Instance<dsv::mod::Logistic_Model> logistico;

    // Condicion inicial compartida
    float x0_global = 10.0f;
    malthus.state[0]   = x0_global;
    logistico.state[0] = x0_global;

    auto& m_malthus = malthus.getModel();
    auto& m_logistico = logistico.getModel();

    // Colores base para consistencia visual
    const sf::Color colMalthus = dsv::Color::cian;
    const sf::Color colLogistic = dsv::Color::magenta;

    // 3. Arquitectura del Layout
    dsv::Layout layout = {
        "tM tM tM tL tL tL",
        "tM tM tM tL tL tL",
        "tc tc tc tc tc tc",
        "tc tc tc tc tc tc",
        "fM fM fM fL fL fL",
        "fM fM fM fL fL fL"
    };
    dsv::Tablero tablero(window, layout, dsv::Color::panelUp, dsv::Color::panelDown);

    // =========================================================================
    // VISUALIZACIONES 
    // =========================================================================

    // Graficas de Tiempo Individuales
    dsv::Vista<dsv::GraficaTiempo> grafMalthus = 
        tablero.add<dsv::GraficaTiempo>("Malthus: x(t)", colMalthus, "tM", colMalthus);
    dsv::Vista<dsv::GraficaTiempo> grafLogistico = 
        tablero.add<dsv::GraficaTiempo>("Logistico: x(t)", colLogistic, "tL", colLogistic);

    grafMalthus.objeto.configurarVentanaTiempo(50.0f);
    grafLogistico.objeto.configurarVentanaTiempo(50.0f);
    grafLogistico.objeto.configurarLimitesY(0.0f, 200.0f); // Para visualizar el tope en K

    // Grafica Comparativa
    dsv::Vista<dsv::GraficaTiempo> grafComp = 
        tablero.add<dsv::GraficaTiempo>("Comparativa Superpuesta", dsv::Color::blanco, "tc");
    grafComp->agregarSerie("Malthus", colMalthus);
    grafComp->agregarSerie("Logistico", colLogistic);
    grafComp.objeto.configurarVentanaTiempo(50.0f);
    grafComp.objeto.ponerSombreado(false);

    // Retratos de Fase (x vs dx/dt)
    dsv::Vista<dsv::EspacioFase2D> faseM = 
        tablero.add<dsv::EspacioFase2D>("Fase Malthus (x vs dx/dt)", colMalthus, "fM", colMalthus);
    dsv::Vista<dsv::EspacioFase2D> faseL = 
        tablero.add<dsv::EspacioFase2D>("Fase Logistico (x vs dx/dt)", colLogistic, "fL", colLogistic);

    faseM->activarAutoescalado(true);
    faseM->activarSeguimiento(true);
    faseL->activarAutoescalado(true);
    faseL->activarSeguimiento(false); // Para apreciar la parabola completa

    // =========================================================================
    // INTERFAZ DE USUARIO (Multiples Paneles)
    // =========================================================================

    // PANEL 1: Malthus
    dsv::PanelFlotante pMalthus(window, "Malthus", dsv::HandleOrientacion::Vertical, dsv::DespliegueDir::Der, colMalthus);
    pMalthus.positionAbsoluta(dsv::Ubicacion::ArribaIzq);
    dsv::MenuFlotante* menuM = pMalthus.crearContenido<dsv::MenuFlotante>(5.f, 10.f);
    menuM->agregarFila().agregar<dsv::CampoDeslizador>("Tasa (r) ", &m_malthus.r, 0.01f, 1.0f);
    menuM->agregarFila().agregar<dsv::CampoVariable>("Poblacion: ", &malthus.state[0]);

    // PANEL 2: Logistico
    dsv::PanelFlotante pLogistico(window, "Logistico", dsv::HandleOrientacion::Vertical, dsv::DespliegueDir::Izq, colLogistic);
    pLogistico.positionAbsoluta(dsv::Ubicacion::ArribaDer);
    dsv::MenuFlotante* menuL = pLogistico.crearContenido<dsv::MenuFlotante>(5.f, 10.f);
    menuL->agregarFila().agregar<dsv::CampoDeslizador>("Tasa (r) ", &m_logistico.r, 0.01f, 1.0f);
    menuL->agregarFila().agregar<dsv::CampoDeslizador>("Capacidad (K) ", &m_logistico.K, 10.0f, 500.0f);
    menuL->agregarFila().agregar<dsv::CampoVariable>("Poblacion: ", &logistico.state[0]);

    // PANEL 3: Condiciones Iniciales Globales
    dsv::PanelFlotante pCond(window, "Cond. Iniciales", dsv::HandleOrientacion::Horizontal, dsv::DespliegueDir::Arriba, dsv::Color::blanco);
    pCond.positionAbsoluta(dsv::Ubicacion::AbajoCentro);
    dsv::MenuFlotante* menuC = pCond.crearContenido<dsv::MenuFlotante>(5.f, 10.f);
    menuC->agregarFila().agregar<dsv::CampoDeslizador>("x0 Global ", &x0_global, 1.0f, 200.0f);
    menuC->agregarFila().agregar<dsv::CampoBoton>("Aplicar Reset", [&]{
        malthus.state[0] = x0_global;
        logistico.state[0] = x0_global;
        malthus.t = 0.0f;
        logistico.t = 0.0f;
    }, dsv::Color::rojo);

    // PANEL 4: Control de Tiempo
    bool pausa = true;
    float timeScale = 1.0f;
    dsv::PanelFlotante pTiempo(window, "Tiempo", dsv::HandleOrientacion::Horizontal, dsv::DespliegueDir::Abajo, dsv::Color::naranja);
    pTiempo.positionAbsoluta(dsv::Ubicacion::ArribaCentro);
    dsv::MenuFlotante* menuT = pTiempo.crearContenido<dsv::MenuFlotante>(5.f, 10.f);
    menuT->agregarFila().agregar<dsv::CampoToggleTexto>("", &pausa, "Play", "Pausa", dsv::Color::naranja);
    menuT->agregarFila().agregar<dsv::CampoVariable>("Escala x", &timeScale);

    // =========================================================================
    // BUCLE PRINCIPAL
    // =========================================================================

    sf::Clock clock;
    sf::Time accumulator = sf::Time::Zero;
    sf::Time ups = sf::seconds(0.005f); // 200 Hz para evitar divergencia temprana

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window.close();

            pMalthus.gestionarEvento(event);
            pLogistico.gestionarEvento(event);
            pCond.gestionarEvento(event);
            pTiempo.gestionarEvento(event);
        }

        sf::Time elapsed = clock.restart();
        if (!pausa) {
            accumulator += elapsed * timeScale;
        }

        while (accumulator >= ups) {
            float dt = ups.asSeconds();
            
            // Usamos RK4 para mitigar errores de truncamiento en exponenciales
            dsv::sim::RK4_step(malthus, dt);
            dsv::sim::RK4_step(logistico, dt);
            
            accumulator -= ups;
        }

        if (!pausa) {
            float xM = malthus.state[0];
            float xL = logistico.state[0];
            float t = malthus.t;

            // Inyeccion de datos a graficas temporales
            grafMalthus->push_back(xM, t);
            grafLogistico->push_back(xL, t);
            
            grafComp->push_back(xM, t, "Malthus");
            grafComp->push_back(xL, t, "Logistico");

            // Calculo analitico de la derivada para el retrato de fase
            float dxM = m_malthus.r * xM;
            float dxL = m_logistico.r * xL * (1.0f - xL / m_logistico.K);

            // Mapeo Topologico: Eje X = poblacion, Eje Y = tasa de cambio neta
            faseM->push_back(xM, dxM);
            faseL->push_back(xL, dxL);
        }

        // Render pipeline
        window.clear();
        tablero.draw();
        pMalthus.draw();
        pLogistico.draw();
        pCond.draw();
        pTiempo.draw();
        window.display();
    }

    return 0;
}