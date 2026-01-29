/*  * autor: Angel Manuel Gonzalez Lopez 
    * github: https://github.com/angelmanuelgl
    * web: https://angelmanuelgl.github.io/
    * 
    * proyecto: InsightRT - - - - - - - - - - - - - - - - - - - 
    * libreria de herramientas graficas para monitoreo de datos 
    * en vivo y comportamiento de sistemas complejos.
 */
/*  GEOMETRIA.cpp
    generacion de formas procedimentales y 
    gestion de paneles dinamicos con bordes redondeados
*/

#include "geometria.hpp"


/*
    --- generar cordenadas ---
    la idea es no estar almacenando cordenadas si non poder calcualrlas en tiempo real
*/
sf::Vector2f generadorCirculo::getPuntoByAngulo(float angulo) const {
    return { radio * std::cos(angulo), radio * std::sin(angulo) };
}


/// struct generadorRectangulo
generadorRectangulo::generadorRectangulo(sf::Vector2f sz, float r, uint32_t q)
    : size(sz), radio(r), calidad_arco(q), circleGen(r) {
    centers[0] = { size.x - radio, radio };
    centers[1] = { size.x - radio, size.y - radio };
    centers[2] = { radio, size.y - radio };
    centers[3] = { radio, radio };
}
sf::Vector2f generadorRectangulo::getPunto(uint32_t i) const {
    uint32_t idx_esquina = i / calidad_arco;
    float iniAngulos[] = { 4.7123f, 0.0f, 1.5707f, 3.1415f };
    float step = (1.5707f / static_cast<float>(calidad_arco - 1)) * (i % calidad_arco);
    return centers[idx_esquina % 4] + circleGen.getPuntoByAngulo(iniAngulos[idx_esquina % 4] + step);
}

// --- --- ---- ---
// --- Rectangulos ---
sf::VertexArray generarRectanguloRelleno(sf::Vector2f size, float radio, uint32_t calidad, sf::Color color) {
    uint32_t puntosPorEsquina = calidad / 4;
    generadorRectangulo gen(size, radio, puntosPorEsquina);
    sf::VertexArray fan(sf::TriangleFan);
    fan.append(sf::Vertex({size.x / 2.0f, size.y / 2.0f}, color));
    for (uint32_t i = 0; i < calidad; i++) {
        fan.append(sf::Vertex(gen.getPunto(i), color));
    }
    fan.append(fan[1]);
    return fan;
}

void generarBorde(sf::VertexArray& vertex_array, sf::Vector2f position, sf::Vector2f size, 
                  float radio, float grosor, uint32_t calidad, sf::Color color) {
    vertex_array.setPrimitiveType(sf::TriangleStrip);
    uint32_t puntosPorEsquina = calidad / 4;
    generadorRectangulo gen_ex(size, radio, puntosPorEsquina);
    
    sf::Vector2f in_borde{grosor, grosor};
    sf::Vector2f in_size = {size.x - 2.0f * grosor, size.y - 2.0f * grosor};
    float in_radio = std::max(0.0f, radio - grosor);
    generadorRectangulo gen_in(in_size, in_radio, puntosPorEsquina);

    vertex_array.resize((calidad + 1) * 2);
    for(uint32_t i = 0; i < calidad; i++) {
        vertex_array[2 * i + 0].position = position + in_borde + gen_in.getPunto(i);
        vertex_array[2 * i + 1].position = position + gen_ex.getPunto(i);
        vertex_array[2 * i + 0].color = color;
        vertex_array[2 * i + 1].color = color;
    }
    vertex_array[2 * calidad + 0].position = position + in_borde + gen_in.getPunto(0);
    vertex_array[2 * calidad + 1].position = position + gen_ex.getPunto(0);
    vertex_array[2 * calidad + 0].color = color;
    vertex_array[2 * calidad + 1].color = color;
}

/*
    STRUCT PANEL
    generar paneles dinamicos
*/
Panel::Panel(sf::Vector2f sz, float r, sf::Color bgColor, sf::Color extColor) 
    : size(sz), radio(r) {
    background = generarRectanguloRelleno(size, radio, 40, bgColor);
    generarBorde(contorno, {0,0}, size, radio, 2.0f, 40, extColor);
}

void Panel::setPosition(float x, float y) {
    pos_actual = {x, y};
    mytransform = sf::Transform::Identity;
    mytransform.translate(x, y);
}

void Panel::positionAbsoluta(Ubicacion ubi, const sf::RenderWindow& window, float margin) {
    sf::Vector2u winSize = window.getSize();
    float x = 0, y = 0;
    switch(ubi) {
        case Ubicacion::ArribaIzq:    x = margin; y = margin; break;
        case Ubicacion::ArribaCentro: x = (winSize.x - size.x) / 2.f; y = margin; break;
        case Ubicacion::ArribaDer:    x = winSize.x - size.x - margin; y = margin; break;
        case Ubicacion::AbajoIzq:     x = margin; y = winSize.y - size.y - margin; break;
        case Ubicacion::AbajoCentro:  x = (winSize.x - size.x) / 2.f; y = winSize.y - size.y - margin; break;
        case Ubicacion::AbajoDer:     x = winSize.x - size.x - margin; y = winSize.y - size.y - margin; break;
        case Ubicacion::Centro:       x = (winSize.x - size.x) / 2.f; y = (winSize.y - size.y) / 2.f; break;
        case Ubicacion::CentroIzq:    x = margin;  y = (winSize.y - size.y) / 2.f; break;
        case Ubicacion::CentroDer:    x = winSize.x - size.x - margin;  y = (winSize.y - size.y) / 2.f; break;
    }
    setPosition(x, y);
}

void Panel::positionRelativa(RelativoA rel, const Panel& other, float spacing) {
    sf::Vector2f oPos = other.getPosition();
    sf::Vector2f oSize = other.getSize();
    float x = oPos.x;
    float y = oPos.y;
    switch (rel) {
        case RelativoA::Arriba: y = oPos.y - size.y - spacing; break;
        case RelativoA::Abajo:  y = oPos.y + oSize.y + spacing; break;
        case RelativoA::Izq:    x = oPos.x - size.x - spacing; break;
        case RelativoA::Der:    x = oPos.x + oSize.x + spacing; break;
    }
    setPosition(x, y);
}

sf::RenderStates Panel::getInternalState() const {
    sf::RenderStates states;
    states.transform = mytransform;
    return states;
}

void Panel::draw(sf::RenderWindow& window) {
    window.draw(background, mytransform);
    window.draw(contorno, mytransform);
}