/*
    CONTIENE
        -> ejes 3d (gestiona grosor, color, min max, dibujado)

*/
#ifndef AUX3D_HPP
#define AUX3D_HPP

//  sfml y std
#include <SFML/Graphics.hpp>
#include <cmath>
#include <vector>
#include <deque>



namespace dsv{

/*
    --- --- --- --- --- --- --- --- --- --- --- 
    --- --- - ELEMENTOS DE LA GRAFICA - --- --- 
    --- --- --- --- --- --- --- --- --- --- --- 
*/  

struct Ejes3D { 
    // limites ejes
    float minX = -20.f, maxX = 20.f;
    float minY = -20.f, maxY = 20.f;
    float minZ = -20.f, maxZ = 20.f;

    sf::Color colorX = sf::Color::Red;
    sf::Color colorY = sf::Color::Green;
    sf::Color colorZ = sf::Color::Blue;

    bool visible = true;
    float grosor = 1.0f;

    // set Todos
    void setLimites(float mx, float Mx, float my, float My, float mz, float Mz ){ 
        minX = mx; maxX = Mx;
        minY = my; maxY = My;
        minZ = mz; maxZ = Mz;
    }

    // set Individual
    void setEjeXLimites(float min, float max ){  minX = min; maxX = max; }
    void setEjeYLimites(float min, float max ){  minY = min; maxY = max; }
    void setEjeZLimites(float min, float max ){  minZ = min; maxZ = max; }
};



/*
    --- --- --- --- --- --- --- --- --- --- --- 
    --- --- --- CAMARA Y PROYECCION --- --- --- 
    --- --- --- --- --- --- --- --- --- --- --- 
*/  

class Camara3D {
public:
    float rotacionX = 45.0f;
    float rotacionY = 45.0f;
    float zoom = 1.0f;
    float distanciaCamara = 50.0f;
    float sensibilidad = 0.4f;

    // mouse
    sf::Vector2i ultimaPosMouse;
    bool estaRotando = false;

    void rotar(float dx, float dy) {
        rotacionY += dx * sensibilidad; 
        rotacionX += dy * sensibilidad; 

        // El "clamp" para no marear al usuario
        if (rotacionX > 89.0f)  rotacionX = 89.0f;
        if (rotacionX < -89.0f) rotacionX = -89.0f;
    }

    // 3D -> 2d
    sf::Vector2f proyectar(sf::Vector3f p, sf::Vector2f pSize, const Limites3D& lim) const {
        // 1. Centrado respecto a los límites que le pasemos
        float cx = (lim.maxX + lim.minX) / 2.0f;
        float cy = (lim.maxY + lim.minY) / 2.0f;
        float cz = (lim.maxZ + lim.minZ) / 2.0f;

        float px = p.x - cx;
        float py = p.y - cy;
        float pz = p.z - cz;

        // 2. Rotación
        float radX = rotacionX * 3.14159f / 180.0f;
        float radY = rotacionY * 3.14159f / 180.0f;

        float x1 = px * cos(radY) + pz * sin(radY);
        float z1 = -px * sin(radY) + pz * cos(radY);
        
        float y2 = py * cos(radX) - z1 * sin(radX);
        float z2 = py * sin(radX) + z1 * cos(radX);

        // 3. Perspectiva
        float den = distanciaCamara - z2;
        if (den < 0.5f) return sf::Vector2f(-10000.f, -10000.f); 
        
        float factorPersp = 1.0f / den;
        float escalaBase = (pSize.x < pSize.y ? pSize.x : pSize.y) * 0.5f;
        
        float finalX = x1 * factorPersp * zoom * escalaBase;
        float finalY = -y2 * factorPersp * zoom * escalaBase;

        return sf::Vector2f(finalX + pSize.x / 2.0f, finalY + pSize.y / 2.0f);
    }

    //evento del mouse
    void gestionarEvento(const sf::Event& evento, const sf::RenderWindow& window){
        // --- ROTACION CON EL MOUSE ---
        if (evento.type == sf::Event::MouseButtonPressed) {
            if (evento.mouseButton.button == sf::Mouse::Left) {
                estaRotando = true;
                ultimaPosMouse = sf::Mouse::getPosition(window);
            }
        }

        if (evento.type == sf::Event::MouseButtonReleased) {
            if (evento.mouseButton.button == sf::Mouse::Left) {
                estaRotando = false;
            }
        }

        if (evento.type == sf::Event::MouseMoved && estaRotando) {
            sf::Vector2i posActual = sf::Mouse::getPosition(window);
            
            float dx = static_cast<float>(posActual.x - ultimaPosMouse.x);
            float dy = static_cast<float>(posActual.y - ultimaPosMouse.y);
            
            rotar(dx, dy);
            
            ultimaPosMouse = posActual;
        }

        // --- ZOOM CON EL SCROLL ---
        if (evento.type == sf::Event::MouseWheelScrolled) {
            if (evento.mouseWheelScroll.delta > 0) zoom *= 1.1f;
            else zoom *= 0.9f;
        }
    }
};

}
#endif