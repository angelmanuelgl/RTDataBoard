/*  * autor: Angel Manuel Gonzalez Lopez 
    * github: https://github.com/angelmanuelgl
    * web: https://angelmanuelgl.github.io/
    * 
    * proyecto: InsightRT - - - - - - - - - - - - - - - - - - - 
    * libreria de herramientas graficas para monitoreo de datos 
    * en tiempo real y comportamiento de sistemas complejos.
*/
/*  GEOMETRIA.hpp
    generacion de formas procedimentales 
    rectangulos redondeados
*/
#ifndef GEOMETRIA_HPP
#define GEOMETRIA_HPP

#include <SFML/Graphics.hpp>
#include <cmath>
#include <algorithm>

// --- estructuras euxiliares ---
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

// funciones globales
sf::VertexArray generarRectanguloRelleno(sf::Vector2f size, float radio, uint32_t calidad, sf::Color color);

void generarBorde(sf::VertexArray& vertex_array, sf::Vector2f position, sf::Vector2f size, 
                  float radio, float grosor, uint32_t calidad, sf::Color color);

// el dibujo de lo que sera el poanel un rectangulo con esquinaws redondeadas

class  RectanguloRedondeado{
private:
    sf::VertexArray background;  
    sf::VertexArray contorno;
public:
    void generar(sf::Vector2f size, float radio, sf::Color bgColor, sf::Color extColor);
    

    void draw( sf::RenderWindow& window, const sf::Transform& transform);
};



#endif