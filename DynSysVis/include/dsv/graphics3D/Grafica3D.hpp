#ifndef GRAFICA3D_HPP
#define GRAFICA3D_HPP

// SFML y Standard
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <limits>

// DSV
#include "dsv/core/Objeto.hpp"
#include "dsv/dataStructures/GestorSeries.hpp"
#include "dsv/graphics3D/Auxiliares3D.hpp" // Para Limites3D, Ejes3D, etc.
#include "dsv/geodraw/draw.hpp"

namespace dsv {

class Grafica3D : public Objeto {
protected:
    GestorSeries gestor;
    Camara3D camara;
    Limites3D lim;
    Ejes3D ejes; // Se activara cuando definamos la clase de ejes
    
    


public:
    Grafica3D( ){ 
        // Inicializar limites para el auto-ajuste de escala
        lim.minX = lim.minY = lim.minZ = std::numeric_limits<float>::max();
        lim.maxX = lim.maxY = lim.maxZ = std::numeric_limits<float>::lowest();
        // lim.minX = lim.minY = lim.minZ = -1;
        // lim.maxX = lim.maxY = lim.maxZ = 1;

        camara.distanciaCamara = 100.0f;
    }

    // TODO ============================================================
    // TODO--- GESTION DE SERIES (API UNIFICADA) --- // TODO <----------
    // TODO ============================================================



    // ============================================================
    // --- CONTROL Y RENDER ---
    // ============================================================

    void gestionarEvento(const sf::Event& ev, const sf::RenderWindow& win ){ 
        camara.gestionarEvento(ev, win);
    }

    void dibujarEjes(sf::RenderWindow& window, sf::RenderStates states, sf::Vector2f pSize) {
        if (!ejes.visible) return;

        // float margen = 0.5f;

        // float anchoX = lim.maxX - lim.minX;
        // ejes.minX = lim.minX - (anchoX * margen);
        // ejes.maxX = lim.maxX + (anchoX * margen);

        // float anchoY = lim.maxY - lim.minY;
        // ejes.minY = lim.minY - (anchoY * margen);
        // ejes.maxY = lim.maxY + (anchoY * margen);

        // float anchoZ = lim.maxZ - lim.minZ;
        // // ejes.minZ = lim.minZ - (anchoZ * margen);
        // // ejes.maxZ = lim.maxZ + (anchoZ * margen);

        // Eje X: De ejes.minX a ejes.maxX pasando por el ORIGEN del mundo
        sf::Vector2f p1x = camara.proyectar({ejes.minX, 0.f, 0.f}, pSize, lim);
        sf::Vector2f p2x = camara.proyectar({ejes.maxX, 0.f, 0.f}, pSize, lim);
        dsv::draw::linea(window, states, {p1x, p2x}, ejes.colorX, ejes.grosor);

        // Eje Y: 
        sf::Vector2f p1y = camara.proyectar({0.f, ejes.minY, 0.f}, pSize, lim);
        sf::Vector2f p2y = camara.proyectar({0.f, ejes.maxY, 0.f}, pSize, lim);
        dsv::draw::linea(window, states, {p1y, p2y}, ejes.colorY, ejes.grosor);

        // Eje Z:
        sf::Vector2f p1z = camara.proyectar({0.f, 0.f, ejes.minZ}, pSize, lim);
        sf::Vector2f p2z = camara.proyectar({0.f, 0.f, ejes.maxZ}, pSize, lim);
        dsv::draw::linea(window, states, {p1z, p2z}, ejes.colorZ, ejes.grosor);
    }



    void draw(sf::RenderWindow& window, sf::RenderStates states, sf::Vector2f pSize) override {

        // guardar el size y poscion de la camara
        camara.actualizarBoundsGlobal(states, pSize);

        // dibujar ejes (si estan activos)
        dibujarEjes(window,states,pSize);


        // Dibujar Series
        for( const auto& serie : gestor.lista ){ 
            if (serie.count < 2) continue;

            // Proyeccion de puntos del buffer circular
            std::vector<sf::Vector2f> puntos2D;
            puntos2D.reserve(serie.count);

            for( size_t i = 0; i < serie.count; ++i ){ 
                sf::Vector3f p3 = serie.getPunto3D(i);
                puntos2D.push_back(camara.proyectar(p3, pSize, lim));
            }

            // Grosor reactivo al zoom
            float grosorFinal = serie.grosor * (camara.zoom * 0.5f);
            if (grosorFinal < 1.0f) grosorFinal = 1.0f;
            bool grosorDinamico = serie.adelgazarCola;

            if( serie.paleta.empty() ){
                DSV_LOG_ERROR("has olvidado asignar un color o paleta a una serie ");
            }

            // paleta base
            std::vector<sf::Color> paletaRender = serie.paleta;

            if( serie.difuminarCola ){
                // Si es un color unico, lo duplicamos para poder hacer el degradado
                if( paletaRender.size() == 1 ){
                    for( int i=0; i<10; i++)
                        paletaRender.push_back(paletaRender[0]);
                }

                // aplicamos el fade linealmente
                size_t n = paletaRender.size();
                for( size_t i = 0; i < n; ++i ){
                    float factor = static_cast<float>(i) / (n - 1);
                    paletaRender[i].a = static_cast<sf::Uint8>(paletaRender[i].a * factor);
                }
            }

            // Dibujamos (si no entro al if, dibuja la paleta original intacta)
            dsv::draw::linea(window, states, puntos2D, paletaRender, grosorFinal, grosorDinamico);

        }
    }

    // Accesos para configuracion fina
    Ejes3D& getEjes( ){  return ejes; }
    Camara3D& getCamara( ){  return camara; }
    GestorSeries& getGestor( ){  return gestor; }
};

} // namespace dsv

#endif