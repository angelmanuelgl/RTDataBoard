/*  * autor: Angel Manuel Gonzalez Lopez 
    * github: https://github.com/angelmanuelgl
    * web: https://angelmanuelgl.github.io/
    * 
    * proyecto: InsightRT - - - - - - - - - - - - - - - - - - - 
    * libreria de herramientas graficas para monitoreo de datos 
    * en vivo y comportamiento de sistemas complejos.
*/
/*  GEOMETRIA.hpp
    generacion de formas procedimentales y 
    gestion de paneles dinamicos con bordes redondeados
*/
#ifndef GEOMETRIA_HPP
#define GEOMETRIA_HPP

#include <SFML/Graphics.hpp>
#include <cmath>
#include <algorithm>

// Estructura de utilidad
struct Vector2f { 
    float x, y;
    Vector2f operator+(const Vector2f& other) const { return {x + other.x, y + other.y}; }
    operator sf::Vector2f() const { return sf::Vector2f(x, y); }
};

struct generadorCirculo {
    float radio;
    generadorCirculo(float r) : radio(r) {}
    sf::Vector2f getPuntoByAngulo(float angulo) const;
};

struct generadorRectangulo {
    sf::Vector2f size;
    float radio;
    uint32_t calidad_arco;
    generadorCirculo circleGen;
    sf::Vector2f centers[4];

    generadorRectangulo(sf::Vector2f sz, float r, uint32_t q);
    sf::Vector2f getPunto(uint32_t i) const;
};

// Firmas de funciones globales
sf::VertexArray generarRectanguloRelleno(sf::Vector2f size, float radio, uint32_t calidad, sf::Color color);

void generarBorde(sf::VertexArray& vertex_array, sf::Vector2f position, sf::Vector2f size, 
                  float radio, float grosor, uint32_t calidad, sf::Color color);

// --- Elementos de Interfaz ---

enum class Ubicacion {
    ArribaIzq, ArribaCentro, ArribaDer,
    AbajoIzq, AbajoCentro, AbajoDer,
    CentroIzq, CentroDer, Centro
};

enum class RelativoA {
    Arriba, Abajo, Izq, Der
};

class Panel {
private:
    sf::Vector2f size;
    float radio;
    sf::VertexArray background;
    sf::VertexArray contorno;
    sf::Transform mytransform; 
    sf::Vector2f pos_actual;

public:
    Panel(sf::Vector2f sz, float r, sf::Color bgColor, sf::Color extColor);

    sf::Vector2f getPosition() const { return pos_actual; }
    sf::Vector2f getSize() const { return size; }

    void positionAbsoluta(Ubicacion ubi, const sf::RenderWindow& window, float margin = 10.f);
    void positionRelativa(RelativoA rel, const Panel& other, float spacing = 15.f);
    void setPosition(float x, float y);
    
    sf::RenderStates getInternalState() const;
    void draw(sf::RenderWindow& window);
};

#endif