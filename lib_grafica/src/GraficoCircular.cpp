/*  * autor: Angel Manuel Gonzalez Lopez 
    * github: https://github.com/angelmanuelgl
    * web: https://angelmanuelgl.github.io/
    * 
    * proyecto: InsightRT - - - - - - - - - - - - - - - - - - - 
    * libreria de herramientas graficas para monitoreo de datos 
    * en tiempo real y comportamiento de sistemas complejos.
*/
/*  GRAFICOCIRCULAR.hpp
    frafica circular (tipo pastel o dona)
    ->>. pie charts
    
*/

#include "GraficoCircular.hpp"


/*  
    CLASE GraficoCircular
*/

GraficoCircular::GraficoCircular(sf::Color bgColor ): 
    colorBorde(bgColor){
    // poner colores por si no llama
    colores = { sf::Color::Red, sf::Color::Green, sf::Color::Blue, 
                sf::Color::Yellow, sf::Color::Magenta, sf::Color::Cyan  };
}
void GraficoCircular::personalizarColores(const std::vector<sf::Color>& cols){
    if( cols.size() == 0 ) return;
    colores = cols;
}


/* --- --- --- ---- --- --- --- */
/* --- FUNCIONES AUXILIARES --- */

// --- cosas geometricas --- 
float GraficoCircular::aRadianes(float grados ){
    return grados * 3.14159265f / 180.f;
}
sf::Vector2f GraficoCircular::puntoEnCirculo(sf::Vector2f centro, float radio, float grados ){
    float rad = aRadianes(grados);
    return centro + sf::Vector2f(std::cos(rad) * radio, std::sin(rad) * radio);
}

// --- resetear datoss ---
void GraficoCircular::limpiarGeometria() {
    secciones.clear();
    bordesExteriores.clear();
    bordesInteriores.clear();
    bordesRadiales.clear();
}
void GraficoCircular::reiniciar(){
    valores.clear();
    limpiarGeometria();
    hayAlgunCambio = true;
}

// --- agregar valores --- 
void GraficoCircular::addValue(float valor){
    // el problema es que hay que resetear manuealmente
    valores.push_back(valor);
    hayAlgunCambio = true;
}
void GraficoCircular::addValues(const std::vector<float>& vals){
    reiniciar();
    for( const auto& v : vals ) valores.push_back(v);
    hayAlgunCambio = true;
}


/* --- --- --- --- --- --- --- */
/* ---  GENERAR RELLENO CONTORNO --- */

// --- separadores ---
sf::VertexArray GraficoCircular::crearBordeRadial(sf::Vector2f centro, float rInt, float rExt, float angInt, float angExt, sf::Color color ){
    sf::VertexArray linea(sf::Lines, 2);
    linea[0] = sf::Vertex(puntoEnCirculo(centro, rInt, angInt), color);
    linea[1] = sf::Vertex(puntoEnCirculo(centro, rExt, angExt), color);
    return linea;
}

// --- arcos ---
sf::VertexArray GraficoCircular::crearBorde(sf::Vector2f centro, float radio, float inicio, float barrido, sf::Color color ){
    sf::VertexArray borde(sf::LineStrip);

    int puntos = std::max(10, static_cast<int>(barrido / 5)); 
    for( int j = 0; j <= puntos; j++ ){
        float angulo = inicio + (static_cast<float>(j) / puntos) * barrido;
        borde.append(sf::Vertex(puntoEnCirculo(centro, radio, angulo), color));
    }

    return borde;
}

// --- relleno ---
sf::VertexArray GraficoCircular::crearSector( sf::Vector2f centro, float radioExt, float inicioExt, float barridoExt, 
                                float inicioInt, float barridoInt,  sf::Color color){

    sf::VertexArray sector(sf::TriangleStrip);
    float radioInt = generarRellenoCirculo ? 0.0f : (radioExt / factorDona);

    sf::Color colExt = color; colExt.a = 20;
    sf::Color colInt = color; colInt.a = 120;

    // para la calidad nos basamos el barrido mas largo
    int puntos = std::max(10, static_cast<int>(std::max(barridoExt, barridoInt) / 5)); 

    for( int j = 0; j <= puntos; j++ ){
        float t = static_cast<float>(j) / puntos;

        // angulos actuales
        float angActualExt = inicioExt + t * barridoExt;
        float angActualInt = inicioInt + t * barridoInt;

        sector.append(sf::Vertex(puntoEnCirculo(centro, radioExt, angActualExt), colExt));
        sector.append(sf::Vertex(puntoEnCirculo(centro, radioInt, angActualInt), colInt));
    }
    return sector;
}

/* --- --- --- --- --- --- --- */
/* ---  GENERAR Y DIBUJAR --- */

// --- orquesta la generacion ---
void GraficoCircular::generar(sf::Vector2f size ){
    // --- calcular radio ---
    float disponible = std::min(size.x, size.y) - 2.0f * padding;
    radio = disponible / 2.0f;
    sf::Vector2f centro = { radio, radio };

    //  --- calcular total ---
    float total = 0;
    for (float v : valores) total += v;
    if (total == 0) return;

    limpiarGeometria();

    // --- estilo ---
    float gapPixeles = 3.0f; 
    float anguloActual = 0;
    float radioInt = radio / factorDona;

    // --- generar todo ---
    for( size_t i = 0; i < valores.size(); i++ ){
        float barridoOriginal = (valores[i] / total) * 360.f;
        sf::Color col = colores[i % colores.size()];

        // --- OFFSETS PARA SEPARACIÓN UNIFORME ---
        // Convertimos la distancia lineal (pixeles) a desplazamiento angular
        float offsetExt = (gapPixeles / radio) * (180.f / 3.14159f);
        float offsetInt = (gapPixeles / radioInt) * (180.f / 3.14159f);

        // limites angulares de la seccion con offset
        float inicioExt = anguloActual + offsetExt;
        float finExt    = anguloActual + barridoOriginal - offsetExt;
        float barridoExt = barridoOriginal - 2 * offsetExt;

        float inicioInt = anguloActual + offsetInt;
        float finInt    = anguloActual + barridoOriginal - offsetInt;
        float barridoInt = barridoOriginal - 2 * offsetInt;

        // --- GENERACION DE TODOS ---
        // relleno con degradado
        secciones.push_back(crearSector(centro, radio, inicioExt, barridoExt, 
                                        inicioInt, barridoInt, col));

        // arcos de borde ( exterior e interior)
        bordesExteriores.push_back(crearBorde(centro, radio, inicioExt, barridoExt, col));
        bordesInteriores.push_back(crearBorde(centro, radioInt, inicioInt, barridoInt, col));

        // paredes Radiales (Lineas que unen los arcos)
        bordesRadiales.push_back(crearBordeRadial(centro, radioInt, radio, inicioInt, inicioExt, col));
        bordesRadiales.push_back(crearBordeRadial(centro, radioInt, radio, finInt, finExt, col));

        anguloActual += barridoOriginal;
    }
}

// --- dibujar ---
void GraficoCircular::draw(sf::RenderWindow& window, sf::RenderStates states, sf::Vector2f pSize){
    // margen
    float offsetX = (pSize.x - (radio * 2)) / 2.f;
    float offsetY = (pSize.y - (radio * 2)) / 2.f;
    states.transform.translate(offsetX, offsetY);
    

    // --- actualizar contorno y secciones --- 
    if( hayAlgunCambio ){
        generar(pSize); //todo que se genere al momento de agregar datos y no en el draw
        hayAlgunCambio = false;
    } 

    // --- dibujar  ---
    for( const auto& sec : secciones ) window.draw(sec, states);
    for( const auto& borde : bordesExteriores) window.draw(borde, states);
    for( const auto& borde : bordesInteriores) window.draw(borde, states);
    for( const auto& borde : bordesRadiales) window.draw(borde, states);
    // window.draw(background, states);
    // window.draw(contorno, states);
}