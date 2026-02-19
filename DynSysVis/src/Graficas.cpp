/*  * autor: Angel Manuel Gonzalez Lopez 
    * github: https://github.com/angelmanuelgl
    * web: https://angelmanuelgl.github.io/
    * 
    * - - - - - - -  -  DynSysVis  - - - - - - - - - - 
    * Dynamical System Visualizer Real-Time
    * libreria de herramientas graficas para monitoreo de datos 
    * y comportamiento de sistemas complejos en tiempo Real.
*/
/*  Graficas.cpp
    sistema de clases base para graficar en el plano x y
    clases derivadas: 
            -> funciones que dependen del tiempo
            -> retratos de fase 2D
*/
#include "Graficas.hpp"


namespace dsv{
/*  
    --- --- --- --- --- --- --- --- ---  --- --- ---     
    --- --- --- --- --- --- --- --- ---  --- --- ---     
    --- --- --- -- AXULIARES DE GRAFICA -- --- --- --- 
    --- --- --- --- --- --- --- --- ---  --- --- ---     
    --- --- --- --- --- --- --- --- ---  --- --- ---
*/
// En Graficas.hpp o .cpp
void Serie::draw(sf::RenderWindow& window, sf::RenderStates states, 
                std::function<sf::Vector2f(sf::Vector2f)> mapearPunto,
                bool sombreado, bool desvanece, float valorReferenciaY ){
    
    if (puntos.empty()) return;

    // dibujar Sombreado (TriangleStrip)
    if(sombreado ){
        sf::VertexArray degradado(sf::TriangleStrip, puntos.size() * 2);
        for (size_t i = 0; i < puntos.size(); i++) {
            sf::Vector2f pPos = mapearPunto(puntos[i]);
            sf::Vector2f pBase = mapearPunto({puntos[i].x, valorReferenciaY});

            degradado[2 * i].position = pPos;
            degradado[2 * i].color = sf::Color(color.r, color.g, color.b, color.a*80/255);

            degradado[2 * i + 1].position = pBase;
            degradado[2 * i + 1].color = sf::Color(color.r, color.g, color.b, 0);
        }
        window.draw(degradado, states);
    }

    // dibujar la linea principal
    sf::VertexArray linea(sf::LineStrip, puntos.size());
    for (size_t i = 0; i < puntos.size(); i++) {
        linea[i].position = mapearPunto(puntos[i]);
        
        sf::Color colFinal = color;
        if( desvanece ){
            float factor = static_cast<float>(i) / static_cast<float>(puntos.size() - 1);
            colFinal.a = static_cast<sf::Uint8>(color.a * factor);
        }
        linea[i].color = colFinal;
    }
    window.draw(linea, states);

    // dibujar punto caveza y desvanezimiento
    if(desvanece ){
        float radioPunto = 3.5f;
        sf::CircleShape cabeza(radioPunto);

        // el ultimo guardado es el primeor de hasta delante
        cabeza.setOrigin(radioPunto, radioPunto);
        cabeza.setPosition(mapearPunto(puntos.back()));
        cabeza.setFillColor(color);

        // borde blanco p
        cabeza.setOutlineThickness(1.0f);
        cabeza.setOutlineColor(sf::Color::White);
        window.draw(cabeza, states);
    }
}

void Serie::recalcularExtremos(void){
    bool primero = true;
    for( const auto& p : puntos ){
        if( primero ){
            misLimites = {p.x, p.x, p.y, p.y};
            primero = false;
        } else {
            if( p.x < misLimites.minX ) misLimites.minX = p.x;
            if( p.x > misLimites.maxX ) misLimites.maxX = p.x;
            if( p.y < misLimites.minY ) misLimites.minY = p.y;
            if( p.y > misLimites.maxY ) misLimites.maxY = p.y;
        }
    }
}

void Serie::agregarPunto(sf::Vector2f p) {
    
    // ver si tendremosq ue recalcular
    bool recalcularNecesario = false;
    if( puntos.size() >= maxPoints ){

        sf::Vector2f elQueSeVaBorrar = puntos.front();
        //! considerar usar arelglo circular O(n) -> O(1)
        puntos.erase(puntos.begin());  

        // si el punto era un extremos
        if (elQueSeVaBorrar.x == misLimites.minX || elQueSeVaBorrar.x == misLimites.maxX ||
            elQueSeVaBorrar.y == misLimites.minY || elQueSeVaBorrar.y == misLimites.maxY) {
            recalcularNecesario = true;
        }
    }
    
    // agregar
    puntos.push_back(p);

    // recalclar si fuese encesario 
    if (recalcularNecesario || !primerPuntoAgregado) {
            // ! considerar usar heap O(n) -> O(log n)
        recalcularExtremos();
        primerPuntoAgregado = true;
    } else {
        // Solo expandir límites si el nuevo punto es mayor/menor
        if (p.x < misLimites.minX) misLimites.minX = p.x;
        if (p.x > misLimites.maxX) misLimites.maxX = p.x;
        if (p.y < misLimites.minY) misLimites.minY = p.y;
        if (p.y > misLimites.maxY) misLimites.maxY = p.y;
    }

}


/*  
    --- --- --- --- --- --- --- --- ---  --- --- ---     
    --- --- --- --- --- --- --- --- ---  --- --- ---     
    --- --- --- -- GRAFICA GENERICA -- --- --- --- 
    --- --- --- --- --- --- --- --- ---  --- --- ---     
    --- --- --- --- --- --- --- --- ---  --- --- ---
*/

GraficaBase::GraficaBase(unsigned int maxPts, sf::Color color) 
    : Objeto(), maxPoints(maxPts), lineaResaltado(color) { 

    
    // ejes
    nombreEjeX = "Eje X"; nombreEjeY = "Eje Y";
    unidadEjeX = "u"; unidadEjeY = "u";
    numMarcasX = 5; numMarcasY = 4;
    
    // detalles
    mostrarEtiquetasEjes = false;
    sombreado = false;
    desvanece = false;

    lim = {0,0,0,0};

    agregarSerie( seriePrincipal ,color);
}

// --- --- --- --- --- --- --- --- ---
// --- INTERACTUAR CON LAS SERIES ---
// --- --- --- --- --- --- --- --- ---
void GraficaBase::push_back_Gen(sf::Vector2f p, std::string clave) {
    // usar la clave por defeto 
    if( clave == "" ){
        clave = "default";
    }

    // creae la serie si no existe
    if (series.find(clave) == series.end()) {
        series[clave] = Serie(clave, lineaResaltado, maxPoints);
    }

    // agregar
    series[clave].agregarPunto(p);
}
void GraficaBase::agregarSerie(std::string nombre, sf::Color color) {
    // Buscamos si ya existe para no borrar los puntos existentes
    auto it = series.find(nombre);
    
    if( it != series.end() ){
        // Si ya existe, solo cambiamos el color (por si el usuario quiere cambiarlo en tiempo real)
        it->second.setColor(color); 
    } else {
        // Si es nueva, la creamos con los puntos maximos actuales de la gráfica
        series[nombre] = Serie(nombre, color, maxPoints);
    }
}

void GraficaBase::configurarMaxPoints(int mp) { 
    maxPoints = mp; 
    // Actualizar las series que ya existen
    for(auto& [nombre, serie] : series) {
        serie.setMaxPoints(mp); 
    }
}

// --- --- --- --- --- --- --- --- ---
// --- --- --- PREPROCESO --- --- --- ---
// --- --- --- --- --- --- --- --- ---
std::string GraficaBase::getEtiquetaY(int i ){
    if( series.empty()) return "0";

    // i va de 0 a (numMarcasY - 1)
    // fraccion va de 0.0 a 1.0
    float fraccion = (float)i / (numMarcasY - 1);
    
    // Interpolación lineal: valor = min + fraccion * rango
    float valorReal = lim.minY + (fraccion * (lim.maxY - lim.minY));

    // 2. Formateo inteligente mejorado
    if (std::abs(valorReal) < 1.0f && std::abs(valorReal) > 0.0001f) {
        char buffer[16];
        snprintf(buffer, sizeof(buffer), "%.3f", valorReal); // 3 decimales para valores pequeños
        return std::string(buffer);
    } 
    else if (std::abs(valorReal) >= 1000.0f) {
        // Para 1200 -> 1.2k
        char buffer[16];
        snprintf(buffer, sizeof(buffer), "%.1fk", valorReal / 1000.f);
        return std::string(buffer);
    }
    
    // para entreros
    if (valorReal == (int)valorReal) {
        return std::to_string((int)valorReal);
    } else {
        char buffer[16];
        snprintf(buffer, sizeof(buffer), "%.1f", valorReal);
        return std::string(buffer);
    }
}

std::string GraficaBase::getEtiquetaX(int i) {
    if (series.empty()) return "0";

    float fraccion = (float)i / (numMarcasX - 1);
    float valorReal = lim.minX + (fraccion * (lim.maxX - lim.minX));

    // En el eje X (tiempo), generalmente preferimos ver el número entero
    // Pero en Espacio de Fase, X puede ser un valor decimal pequeño
    if (std::abs(valorReal) < 10.0f && std::abs(valorReal) > 0.0f && valorReal != (int)valorReal) {
        char buffer[16];
        snprintf(buffer, sizeof(buffer), "%.1f", valorReal);
        return std::string(buffer);
    }

    return std::to_string((int)valorReal);
}

// --- --- --- --- --- --- --- --- ---
// --- --- --- DIBUJAR --- --- --- ---
// --- --- --- --- --- --- --- --- ---
void GraficaBase::dibujarContenido(sf::RenderWindow& window, sf::RenderStates states, 
                                  float paddingL, float offsetTop, float graphWidth, float graphHeight) {
    if (series.empty()) return;

    if( autoEscalado )  recalcularExtremos();

    auto mapearPunto = [&](sf::Vector2f p) {
        float xNorm = (p.x - lim.minX) / (lim.maxX - lim.minX);
        float yNorm = (p.y - lim.minY) / (lim.maxY - lim.minY);
        return sf::Vector2f(
            paddingL + (xNorm * graphWidth),
            offsetTop + graphHeight - (yNorm * graphHeight)
        );
    };

    float valorRef = sombreadoAlEje ? 0.0f : lim.minY;

    // Dibujamos cada serie
    for (auto& [id, serie] : series) {
        serie.draw(window, states, mapearPunto, sombreado, desvanece, valorRef);
    }
}

void GraficaBase::draw(sf::RenderWindow& window, sf::RenderStates states, sf::Vector2f pSize ){ 

    float paddingL = mostrarEtiquetasEjes ? 45.f : 30.f;
    float paddingB = mostrarEtiquetasEjes ? 35.f : 25.f;


    float offsetTop = 10.f;

    float graphWidth = pSize.x - (paddingL + 20.f);
    float graphHeight = pSize.y - (paddingB + offsetTop * 1.0);

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

        // poner etiquetas
        sf::Text label(getEtiquetaY(i), fuenteCompartida, 10);
        float textWidth = label.getGlobalBounds().width;
        // Alinear a la derecha del margen izquierdo (con 5px de separación del eje)
        label.setPosition(paddingL - textWidth - 5.f, yPos - 7.f);
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
        sf::Text label(getEtiquetaX(i), fuenteCompartida, 10);
        label.setPosition(xPos - 5.f, offsetTop + graphHeight + 5.f);
        label.setFillColor(axisColor);
        window.draw(label, states);
    }

    // etiquetas de los ejes
    if( mostrarEtiquetasEjes ){ // por lo general no uso esta parte
        sf::Text txtEjeY(nombreEjeY + " (" + unidadEjeY + ")", fuenteCompartida, 11);
        txtEjeY.setPosition(5.f, offsetTop - 20.f);
        txtEjeY.setFillColor(sf::Color::Cyan);
        window.draw(txtEjeY, states);

        sf::Text txtEjeX(nombreEjeX + " (" + unidadEjeX + ")", fuenteCompartida, 11);
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
    --- --- --- --- --- --- --- --- ---  --- --- ---     
    --- --- --- --- --- --- --- --- ---  --- --- ---     
    --- --- --- -- ESPECIALZIACIONES -- --- --- --- 
    --- --- --- --- --- --- --- --- ---  --- --- ---     
    --- --- --- --- --- --- --- --- ---  --- --- ---
*/


/*
    GRAFICAS TIEMPO
   el eje x es el tiempo y vamos agtregando datos
   se supone que los datos se van agregando en tiempo real
*/
GraficaTiempo::GraficaTiempo( sf::Color color)
    :GraficaBase(500, color) {    
    ponerSombreado(true, true);
}


void GraficaTiempo::push_back(float val , std::string clave){
    
    std::string id = (clave == "") ? "default" : clave;
    
    // obtenemos x / /  x es el tiempo (podira ser un contador o segundos)
    float x = 0;
    if (!series[id].vacia()) {
        x = series[id].getPuntos().back().x + 1;
    }
    
    push_back_Gen({x, val}, id);
}

void GraficaTiempo::recalcularExtremos(void){
    if(series.empty() ) return;


    bool primerSerie = true;
    for( auto const& [id, serie] : series ){
        if( serie.vacia() ) continue;

        Limites limSerie = serie.getLimites();

        // la primer serie
        if( primerSerie ){
            // x // se ajusta a la series
            lim.minX = limSerie.minX; 
            lim.maxX = limSerie.maxX; 
            // y // matiene el hitorico
            
            primerSerie = false;
        } 

        // las demas series
        if( limSerie.minX < lim.minX) lim.minX = limSerie.minX;
        if( limSerie.maxX > lim.maxX) lim.maxX = limSerie.maxX;
        if( limSerie.minY < lim.minY) lim.minY = limSerie.minY;
        if( limSerie.maxY > lim.maxY) lim.maxY = limSerie.maxY;
        
    }
    

    // --- PERSONALZIACION EXTRA --- 
    
    // x siempre llega hasta almenos maxPoints
    if( lim.maxX < maxPoints ) lim.maxX = maxPoints; 

    // siempre graficar hasta el 0 Y (opcional)
    if( lim.minY > 0 ) lim.minY = 0; 
    if( lim.maxY < 0 ) lim.maxY = 0;
    

    // siempre graficar hasta el 0 X (opcional)
    //if( lim.minX > 0 ) lim.minX = 0; 
    //if( lim.maxX < 0 ) lim.maxX = 0;



    // ---  OJO CON DIVIDIR ENTRE 0 --- 
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
GraficaEspacioFase::GraficaEspacioFase( sf::Color color)    
    :GraficaBase(5000, color)
{    
    nombreEjeX = "X";
    nombreEjeY = "Y";
    ponerDesvanecido(true);
}

void GraficaEspacioFase::push_back(float x, float y , std::string clave){ 
    push_back_Gen({x, y}, clave);
}
void GraficaEspacioFase::recalcularExtremos(void){
     if(series.empty() ) return;


    bool primerSerie = true;
    for( auto const& [id, serie] : series ){
        if( serie.vacia() ) continue;

        Limites limSerie = serie.getLimites();

        // la primer serie
        if( primerSerie ){
            // x // se ajusta a la series
            // lim.minX = limSerie.minX; 
            // lim.maxX = limSerie.maxX; 
            // // y // matiene el hitorico
            // lim.maxY = limSerie.maxY; 
            // lim.minY = limSerie.minY;

            primerSerie = false;
        } 

        // las demas series
        float m = 10; // margen
        if( limSerie.minX +m < lim.minX) lim.minX = limSerie.minX +m;
        if( limSerie.maxX +m > lim.maxX) lim.maxX = limSerie.maxX +m;
        if( limSerie.minY +m < lim.minY) lim.minY = limSerie.minY +m;
        if( limSerie.maxY +m > lim.maxY) lim.maxY = limSerie.maxY +m;
        
    }
    

    // --- PERSONALZIACION EXTRA --- 

    // siempre graficar hasta el 0 Y (opcional)
    if( lim.minY > 0 ) lim.minY = 0; 
    if( lim.maxY < 0 ) lim.maxY = 0;
    

    // siempre graficar hasta el 0 X (opcional)
    //if( lim.minX > 0 ) lim.minX = 0; 
    //if( lim.maxX < 0 ) lim.maxX = 0;



    // ---  OJO CON DIVIDIR ENTRE 0 --- 
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



// end dsv
}