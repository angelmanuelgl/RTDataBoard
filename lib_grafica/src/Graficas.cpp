/*  * autor: Angel Manuel Gonzalez Lopez 
    * github: https://github.com/angelmanuelgl
    * web: https://angelmanuelgl.github.io/
    * 
    * proyecto: InsightRT - - - - - - - - - - - - - - - - - - - 
    * libreria de herramientas graficas para monitoreo de datos 
    * en vivo y comportamiento de sistemas complejos.
 */
/*  GEOMETRIA.cpp
    sistema de clases base para graficar
    clases derivadas: para la visualización de series temporales  
                      para retratos de fase.
*/
#include "Graficas.hpp"

/*
    GRAFICAS BASE
    aquiv an lso metodos que deberian tener todas las graficas en comun
*/
GraficaBase::GraficaBase(unsigned int maxPts, sf::Color color, std::string t ) 
    : maxPoints(maxPts), lineaResaltado(color), titulo(t) { 
    

    // --- la fuente sera la misma por ahora ---
    //TODO agregar mas fuentes y una funcion para cambiar fuentes
    const std::string& ruta_fuente = "assets/fonts/Roboto.ttf";
    if( !font.loadFromFile(ruta_fuente) ){  /* error */ }
    

    // medidas
    tamanoTitulo = 12.f;
    titulo_texto.setFont(font);
    titulo_texto.setString(titulo);
    titulo_texto.setCharacterSize(tamanoTitulo);
    titulo_texto.setFillColor(sf::Color::White);
    
    // ejes
    nombreEjeX = "Eje X"; nombreEjeY = "Eje Y";
    unidadEjeX = "u"; unidadEjeY = "u";
    numMarcasX = 5; numMarcasY = 4;
    
    // detalles
    mostrarEtiquetasEjes = false;
    sombreado = false;
    desvanece = false;

    lim = {0,0,0,0};
}

void GraficaBase::dibujarContenido(sf::RenderWindow& window, sf::RenderStates states, float paddingL, float offsetTop, float graphWidth, float graphHeight ){
    if( puntos.empty()) return;

    // --- auto escala -- 
    recalcularExtremos();
  

    // --- normalizacion ---
    // pasar a coordenadas de pantalla
    auto mapearPunto = [&](sf::Vector2f p ){
        float xNorm = (p.x - lim.minX) / (lim.maxX - lim.minX);
        float yNorm = (p.y - lim.minY) / (lim.maxY - lim.minY);
        return sf::Vector2f(
            paddingL + (xNorm * graphWidth),
            offsetTop + graphHeight - (yNorm * graphHeight)
        );
    };
    
    // --- efecto sombreado ---
    if( sombreado ){
        sf::VertexArray degradado(sf::TriangleStrip, puntos.size() * 2);
        for( size_t i = 0; i < puntos.size(); i++ ){
            sf::Vector2f pPos = mapearPunto(puntos[i]);

            // vertice arriba: posicion del dato, color semi-transparente
            degradado[2 * i].position = pPos;
            degradado[2 * i].color = sf::Color(lineaResaltado.r, lineaResaltado.g, lineaResaltado.b, 80);

            // vertice abajo: proyectado al fondo de la grafica, color invisible
            //  podemos usar lim.minY para que el degradado baje hasta el "suelo" de los datos
            float valorReferencia = 0.0; //lim.minY
            sf::Vector2f pBase = mapearPunto({puntos[i].x, lim.minY});
            degradado[2 * i + 1].position = pBase;
            degradado[2 * i + 1].color = sf::Color(lineaResaltado.r, lineaResaltado.g, lineaResaltado.b, 0);
        }
        window.draw(degradado, states);
    }

    // --- linea principal ---
    sf::VertexArray linea(sf::LineStrip, puntos.size());
    for( size_t i = 0; i < puntos.size(); i++ ){
        linea[i].position = mapearPunto(puntos[i]);
        
          linea[i].color = lineaResaltado;
        if( desvanece ){
            // el punto mas reciente (al final) es opaco, 
            // el mas viejo (al principio) es transparente.
            // i / puntos.size()  in [0.0 a 1.0]
            float factor = static_cast<float>(i) / static_cast<float>(puntos.size() - 1);
            sf::Uint8 alpha = static_cast<sf::Uint8>(255 * factor);
            
            linea[i].color = sf::Color(lineaResaltado.r, lineaResaltado.g, lineaResaltado.b, alpha);
        }
    }
    window.draw(linea, states);
    

    // --- ponerle puntito ---
    if( !puntos.empty() && desvanece ){
        float radioPunto = 3.5f; 
        sf::CircleShape puntoCabeza(radioPunto);
        
        // el ultimo guardado es el primero de hasta delante
        sf::Vector2f ultimaPos = mapearPunto(puntos.back());
        
        puntoCabeza.setOrigin(radioPunto, radioPunto); // Centrar el círculo
        puntoCabeza.setPosition(ultimaPos);
        puntoCabeza.setFillColor(lineaResaltado); 
        
        //  borde blanco p
        puntoCabeza.setOutlineThickness(1.0f);
        puntoCabeza.setOutlineColor(sf::Color::White);
        
        window.draw(puntoCabeza, states);
    }
}

std::string GraficaBase::getEtiquetaY(int i ){
    if( puntos.empty()) return "0";

    // i va de 0 a (numMarcasY - 1)
    // fraccion va de 0.0 a 1.0
    float fraccion = (float)i / (numMarcasY - 1);
    
    // Interpolación lineal: valor = min + fraccion * rango
    float valorReal = lim.minY + (fraccion * (lim.maxY - lim.minY));

    // Formateo inteligente
    if( std::abs(valorReal) < 0.01f && std::abs(valorReal) > 0.000001f ){
        // mas precision
        char buffer[16];
        snprintf(buffer, sizeof(buffer), "%.4f", valorReal);
        return std::string(buffer);
    } else if( std::abs(valorReal) >= 1000.0f ){
        // Para valores muy grandes (ej. 1.2k)
        return std::to_string((int)(valorReal / 1000)) + "k";
    }
    
    // usar (int) para limpiar // fixed para decimales
    return std::to_string((int)valorReal);
}

std::string GraficaBase::getEtiquetaX(int i ){
    if( puntos.empty()) return "0";

    float fraccion = (float)i / (numMarcasX - 1);
    float valorReal = lim.minX + (fraccion * (lim.maxX - lim.minX));

    //  GraficaTiempo,  ver enteros (pasos de tiempo)
    //  EspacioFase,  valores de la variable X
    return std::to_string((int)valorReal);
}

// --- dibujar ---
void GraficaBase::draw(sf::RenderWindow& window, Panel& parent ){ 
    float alturaTitulo = titulo_texto.getGlobalBounds().height;
    sf::Vector2f pSize = parent.getSize();
    sf::RenderStates states = parent.getInternalState();
    
    float paddingL = mostrarEtiquetasEjes ? 45.f : 30.f;
    float paddingB = mostrarEtiquetasEjes ? 35.f : 25.f;
    float graphWidth = pSize.x - (paddingL + 20.f);
    float offsetTop = alturaTitulo + 25.f;
    float graphHeight = pSize.y - (paddingB + offsetTop * 1.0);

    titulo_texto.setPosition(15.f, alturaTitulo - 3.0f);
    window.draw(titulo_texto, states);

    sf::Color colorGuia(60, 60, 60); 
    sf::Color axisColor(100, 100, 100);

    // --- marcas Y con etiquetas ---
    for( int i = 0; i < numMarcasY; i++ ){ 
        float fraccion = (float)i / (numMarcasY - 1);
        float yPos = offsetTop + graphHeight - (fraccion * graphHeight);

        sf::VertexArray lineaGuia(sf::Lines, 2);
        lineaGuia[0] = { {paddingL, yPos}, colorGuia };
        lineaGuia[1] = { {paddingL + graphWidth, yPos}, colorGuia };
        window.draw(lineaGuia, states);

        // AQUI LLAMAMOS A LOS METODOS DE LA GRAFICA HIJA
        sf::Text label(getEtiquetaY(i), font, 10);
        label.setPosition(paddingL - 25.f, yPos - 7.f);
        label.setFillColor(axisColor);
        window.draw(label, states);
    }

    // --- marcas X con etiquetas ---
    for( int i = 0; i < numMarcasX; i++ ){ 
        float fraccion = (float)i / (numMarcasX - 1);
        float xPos = paddingL + (fraccion * graphWidth);

        sf::VertexArray lineaGuia(sf::Lines, 2);
        lineaGuia[0] = { {xPos, offsetTop}, colorGuia };
        lineaGuia[1] = { {xPos, offsetTop + graphHeight}, colorGuia };
        window.draw(lineaGuia, states);

        // AQUI LLAMAMOS A LOS METODOS DE LA GRAFICA HIJA
        sf::Text label(getEtiquetaX(i), font, 10);
        label.setPosition(xPos - 5.f, offsetTop + graphHeight + 5.f);
        label.setFillColor(axisColor);
        window.draw(label, states);
    }

    // etiquetas de los ejes
    if( mostrarEtiquetasEjes ){ // por lo general no uso esta parte
        sf::Text txtEjeY(nombreEjeY + " (" + unidadEjeY + ")", font, 11);
        txtEjeY.setPosition(5.f, offsetTop - 20.f);
        txtEjeY.setFillColor(sf::Color::Cyan);
        window.draw(txtEjeY, states);

        sf::Text txtEjeX(nombreEjeX + " (" + unidadEjeX + ")", font, 11);
        txtEjeX.setPosition(paddingL + graphWidth - 80.f, offsetTop + graphHeight + 18.f);
        txtEjeX.setFillColor(sf::Color::Cyan);
        window.draw(txtEjeX, states);
    }

    // --- colorcar --- 
    sf::VertexArray ejes(sf::Lines, 4);
    ejes[0] = { {paddingL, offsetTop}, axisColor };
    ejes[1] = { {paddingL, offsetTop + graphHeight}, axisColor };
    ejes[2] = { {paddingL, offsetTop + graphHeight}, axisColor };
    ejes[3] = { {paddingL + graphWidth, offsetTop + graphHeight}, axisColor };
    window.draw(ejes, states);

    dibujarContenido(window, states, paddingL, offsetTop, graphWidth, graphHeight);
}


/*
    GRAFICAS TIEMPO
   el eje x es el tiempo y vamos agtregando datos
   se supone que los datos se van agregando en tiempo real
*/
GraficaTiempo::GraficaTiempo( sf::Color color, std::string t)
    :GraficaBase(500, color, t)
{    
    ponerSobreado(true);
}

void GraficaTiempo::addValue(float val ){
    // x es el tiempo (podira ser un contador o segundos)
    float x = puntos.empty() ? 0 : puntos.back().x + 1; 
    puntos.push_back({x, val});
    
    if( puntos.size() > maxPoints) puntos.erase(puntos.begin());
}

void GraficaTiempo::recalcularExtremos(void){
    // --- auto escala -- 
    // calcula los limites super inferior der e izq dinamicamente
    // lim = { puntos[0].x, puntos[0].x, puntos[0].y, puntos[0].y }; // lo coemntamos para que sea max total
    lim.minX = puntos[0].x;
    for( const auto& p : puntos ){
        if( p.x < lim.minX) lim.minX = p.x;
        if( p.x > lim.maxX) lim.maxX = p.x;
        if( p.y < lim.minY) lim.minY = p.y;
        if( p.y > lim.maxY) lim.maxY = p.y;
    }
    // if( puntos.size() < maxPoints ) lim.minX = 0;
    if( puntos.size() < maxPoints ) lim.maxX = maxPoints;

    // siempre graficar ahsta el 0 (opcional)
    if( lim.minY > 0 ) lim.minY = 0; 
    if( lim.maxY < 0 ) lim.maxY = 0;
    //if( lim.minX > 0 ) lim.minX = 0; 
    //if( lim.maxX < 0 ) lim.maxX = 0;

    // por si dividir entre 0
    float epsilon = 0.0001f;
    if( abs(lim.maxX - lim.minX) < epsilon ){
        lim.minX -= 1.0f;
        lim.maxX += 1.0f;
    }
    if( std::abs(lim.maxY - lim.minY) < epsilon ){
        lim.minY -= 1.0f;
        lim.maxY += 1.0f;
    }
}


/*
    GRAFICAS DE ESPACIO FASE
    agrega pares
*/
GraficaEspacioFase::GraficaEspacioFase( sf::Color color, std::string t)    
    :GraficaBase(500, color, t)
{    
    nombreEjeX = "X";
    nombreEjeY = "Y";
    ponerDesvanecido(true);
}

void GraficaEspacioFase::addValue(float x, float y ){ 
    puntos.push_back({x, y});
    if( puntos.size() > maxPoints ){ 
        puntos.erase(puntos.begin());
    }
}
void GraficaEspacioFase::recalcularExtremos(void){
    // --- auto escala -- 
    // calcula los limites super inferior der e izq dinamicamente
    // lim = { puntos[0].x, puntos[0].x, puntos[0].y, puntos[0].y }; // lo coemntamos para que sea max total
    for( const auto& p : puntos ){
        if( p.x < lim.minX) lim.minX = p.x;
        if( p.x > lim.maxX) lim.maxX = p.x;
        if( p.y < lim.minY) lim.minY = p.y;
        if( p.y > lim.maxY) lim.maxY = p.y;
    }

    // siempre graficar ahsta el 0 (opcional)
    if( lim.minY > 0 ) lim.minY = 0; 
    if( lim.maxY < 0 ) lim.maxY = 0;
    //if( lim.minX > 0 ) lim.minX = 0; 
    //if( lim.maxX < 0 ) lim.maxX = 0;

    // por si dividir entre 0
    float epsilon = 0.0001f;
    if( abs(lim.maxX - lim.minX) < epsilon ){
        lim.minX -= 1.0f;
        lim.maxX += 1.0f;
    }
    if( std::abs(lim.maxY - lim.minY) < epsilon ){
        lim.minY -= 1.0f;
        lim.maxY += 1.0f;
    }
}