/*  * autor: Angel Manuel Gonzalez Lopez 
    * github: https://github.com/angelmanuelgl
    * web: https://angelmanuelgl.github.io/
    * 
    * proyecto: InsightRT - - - - - - - - - - - - - - - - - - - 
    * libreria de herramientas graficas para monitoreo de datos 
    * en tiempo real y comportamiento de sistemas complejos.
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
    STRUCT 
    generar la parte dibujable de un panel
    el borde y fondo

*/

void RectanguloRedondeado::generar(sf::Vector2f size, float radio, sf::Color bgColor, sf::Color extColor){
    background = generarRectanguloRelleno(size, radio, 40, bgColor);
    generarBorde(contorno, {0.f, 0.f}, size, radio, 2.0f, 40, extColor);
}

void RectanguloRedondeado::draw( sf::RenderWindow& window, const sf::Transform& transform) {
    window.draw(background, transform);
    window.draw(contorno, transform);
}

