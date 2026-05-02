/*
 * DynSysVis RT - Lorenz Professional Lab
 * Enfoque: Estetica de Particulas y Control de Tiempo Avanzado
 * Nota: Uso estricto de ASCII y dsv::mod::Lorenz_Model directo.
 */

#include <SFML/Graphics.hpp>
#include <cmath>
#include <vector>
#include <string>
#include <iostream>

#include "DynSysVis.hpp"

int main() {
    // 1. Configuracion de Entorno
    dsv::Color::cargar("assets/config/colores.txt");
    sf::RenderWindow window;
    dsv::Sistema::inicializarVentana(window, "DynSysVis: Lorenz Butterfly Effect");

    // 2. Definicion de Instancias (8 trayectorias con perturbacion infinitesimal)
    using Lorenz = dsv::mod::Lorenz_Model;
    const int numSims = 8;
    std::vector<dsv::mod::Instance<Lorenz>> sims(numSims);

    auto resetSims = [&]() {
        for(int i = 0; i < numSims; ++i) {
            // Perturbacion en el eje X: 0.001 * i
            sims[i].state = { 10.0f + (i * 0.001f), 10.0f, 10.0f };
            sims[i].t = 0.0f;
        }
    };
    resetSims();

    // 3. Layout y Componentes de Visualizacion
    dsv::Layout miLayout = {
        "G3D G3D ST",
        "G3D G3D ST",
        "CT  CT  CT"
    };
    dsv::Tablero tablero(window, miLayout, dsv::Color::panelUp, dsv::Color::panelDown);

    // Configuracion de Grafica 3D (El Atractor)
    dsv::Vista<dsv::Grafica3D> vista3D = tablero.add<dsv::Grafica3D>("Atractor de Lorenz 3D", dsv::Color::blanco, "G3D");
    vista3D.objeto.getEjes().setLimites(-30.0f, 30.0f);
    
    // Configuracion de Estelas (Wake Effect)
    auto& gestor3D = vista3D.objeto.getGestor();
    gestor3D.setMaxPointsSeries(100);    // Cola corta para efecto de particula
    gestor3D.setDifuminadoSeries(true);  // Desvanecimiento alfa
    gestor3D.setAdelgazadoSeries(true);  // Reduccion de grosor al final
    gestor3D.setGrosorSeries(2.5f);

    // Preparar series con paleta de colores
    for(int i = 0; i < numSims; ++i) {
        std::string id = "P" + std::to_string(i);
        gestor3D.agregarSerie(id, dsv::Color::Magma(i, numSims)); // Colores variados
    }

    // Grafica de Tiempo para velocidad X
    dsv::Vista<dsv::GraficaTiempo> grafT = tablero.add<dsv::GraficaTiempo>("Divergencia en X", dsv::Color::blanco, "ST");
    for(int i = 0; i < numSims; ++i) {
        grafT->agregarSerie("X" + std::to_string(i), dsv::Color::Magma(i, numSims));
    }

    // 4. Paneles de Control y UI
    bool pausa = false;
    float timeScale = 1.0f;
    float fps = 0.0f;

    dsv::PanelFlotante panelT(window, "Control de Tiempo", dsv::HandleOrientacion::Horizontal, dsv::DespliegueDir::Abajo, dsv::Color::naranja);
    panelT.positionAbsoluta(dsv::Ubicacion::ArribaCentro);
    dsv::MenuFlotante* menu = panelT.crearContenido<dsv::MenuFlotante>(5.0f, 12.0f);

    menu->agregarFila().agregar<dsv::CampoVariable>("FPS: ", &fps);
    
    auto& filaCtrl = menu->agregarFila();
    filaCtrl.agregar<dsv::CampoBoton>("<<", [&]{ timeScale = std::max(0.1f, timeScale - 0.2f); });
    filaCtrl.agregar<dsv::CampoVariable>("Vel: ", &timeScale);
    filaCtrl.agregar<dsv::CampoBoton>(">>", [&]{ timeScale += 0.2f; });
    
    menu->agregarFila().agregar<dsv::CampoToggleTexto>("Estado: ", &pausa, "PLAY", "PAUSA", dsv::Color::verde);
    menu->agregarFila().agregar<dsv::CampoBoton>("Reiniciar", resetSims, dsv::Color::rojo);

    // 5. Bucle Principal
    sf::Clock clock;
    sf::Clock fpsClock;
    int frameCount = 0;
    sf::Time ups = sf::seconds(0.005f); 
    sf::Time accumulator = sf::Time::Zero;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window.close();

            // Gestion de Eventos DSV
            panelT.gestionarEvento(event);
            vista3D->gestionarEvento(event, window); // Rotacion/Zoom

            // Atajos de Teclado
            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Space) pausa = !pausa;
                if (event.key.code == sf::Keyboard::Up)    timeScale += 0.1f;
                if (event.key.code == sf::Keyboard::Down)  timeScale = std::max(0.1f, timeScale - 0.1f);
            }
        }

        // Calculo de FPS
        frameCount++;
        if (fpsClock.getElapsedTime().asSeconds() >= 1.0f) {
            fps = (float)frameCount;
            frameCount = 0;
            fpsClock.restart();
        }

        // Integracion Fisica
        sf::Time dt = clock.restart();
        if (!pausa) {
            accumulator += dt * timeScale;
            while (accumulator >= ups) {
                for (auto& s : sims) {
                    // Usamos RK4 para precision en trayectorias caoticas
                    dsv::sim::RK4_step(s, ups.asSeconds());
                }
                accumulator -= ups;
            }

            // Inyeccion de datos a las vistas
            for (int i = 0; i < numSims; ++i) {
                float x = sims[i].state[0];
                float y = sims[i].state[1];
                float z = sims[i].state[2];
                std::string id = "P" + std::to_string(i);

                gestor3D.push_back({x, y, z}, id);
                grafT->push_back(x, sims[i].t, "X" + std::to_string(i));
            }
        }

        // Renderizado
        window.clear();
        tablero.draw();
        panelT.draw();
        window.display();
    }

    return 0;
}