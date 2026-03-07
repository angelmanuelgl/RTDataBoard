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
#include "dsv/graphics2D/Graficas.hpp"


namespace dsv{
/*  
    --- --- --- --- --- --- --- --- ---  --- --- ---     
    --- --- --- --- --- --- --- --- ---  --- --- ---     
    --- --- --- -- AXULIARES DE GRAFICA -- --- --- --- 
    --- --- --- --- --- --- --- --- ---  --- --- ---     
    --- --- --- --- --- --- --- --- ---  --- --- ---
*/
void Serie2D::draw(sf::RenderWindow& window, sf::RenderStates states, 
                std::function<sf::Vector2f(sf::Vector2f)> mapearPunto,
                bool sombreado, bool desvanece, bool cabeza, float valorReferenciaY ){
    
    size_t n = puntos.size();
    if (n == 0) return;
    
   

    // dibujar Sombreado (TriangleStrip)
    if( sombreado ){
        sf::VertexArray degradado(sf::TriangleStrip, n * 2);
        for (size_t i = 0; i < n; i++) {
            sf::Vector2f puntoActual = puntos[i]; // ya devuelve sf::vector2f

            sf::Vector2f pPos = mapearPunto(puntoActual);
            sf::Vector2f pBase = mapearPunto({puntoActual.x, valorReferenciaY});

            degradado[2 * i].position = pPos;
            degradado[2 * i].color = sf::Color(color.r, color.g, color.b, color.a*80/255);

            degradado[2 * i + 1].position = pBase;
            degradado[2 * i + 1].color = sf::Color(color.r, color.g, color.b, 0);
        }
        window.draw(degradado, states);
    }

    // dibujar la linea principal
    sf::VertexArray linea(sf::LineStrip, n);
    for (size_t i = 0; i < n; i++) {
        sf::Vector2f puntoActual = puntos[i]; // ya devuelve sf::vector2f
        linea[i].position = mapearPunto(puntoActual);
        
        sf::Color colFinal = color;
        if( desvanece ){
            float factor = static_cast<float>(i) / static_cast<float>(n - 1);
            colFinal.a = static_cast<sf::Uint8>(color.a * factor);
        }
        linea[i].color = colFinal;
    }
    window.draw(linea, states);

    // dibujar punto caveza y desvanezimiento
    if( cabeza ){
        float radioPunto = 3.5f;
        sf::CircleShape cabeza(radioPunto);

        // el ultimo guardado es el primeor de hasta delante
        cabeza.setOrigin(radioPunto, radioPunto);
        
        sf::Vector2f ultimoPunto = puntos.back();

        cabeza.setPosition(mapearPunto(ultimoPunto) ) ;
        cabeza.setFillColor(color);

        // borde blanco p
        cabeza.setOutlineThickness(1.0f);
        cabeza.setOutlineColor(sf::Color::White);
        window.draw(cabeza, states);
    }
}



void Serie2D::recalcularExtremos(void){
    if( puntos.empty() ) return;
    misLimites = puntos.getLimites();
}

void Serie2D::agregarPunto(sf::Vector2f p){
    puntos.push(p.x, p.y);

    if( puntos.size() > maxPoints ){
        puntos.pop();
    }

    // limites en O(1)
    misLimites = puntos.getLimites();
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
    if( series.find(clave) == series.end() ){
        series[clave] = Serie2D(clave, lineaResaltado, maxPoints);
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
        series[nombre] = Serie2D(nombre, color, maxPoints);
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
        serie.draw(window, states, mapearPunto, sombreado, desvanece,  cabeza, valorRef);
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
    ponerDesvanecido(false, false);
    maxLimX = 20;// 20 segundos de largo

    // la grafica tiempos e encarga de borrarlos si apsa de cierot tiempo
    maxPoints = 1000000;
}


void GraficaTiempo::push_back(float val , std::string clave){
    
    std::string id = (clave == "") ? "default" : clave;
    
    // obtenemos x / /  x es el tiempo (podira ser un contador o segundos)
    float x = 0;
    if (!series[id].vacia()) {
        x = series[id].back().x + 1;
    }
    
    push_back_Gen({x, val}, id);
}

void GraficaTiempo::push_back(float val , float t, std::string clave){
    push_back_Gen({t, val}, clave);
}

void GraficaTiempo::recalcularExtremos(void){
    if( series.empty() ) return;


    bool primerSerie = true;
    for( auto const& [id, serie] : series ){
        if( serie.vacia() ) continue;

        Limites limSerie = serie.getLimites(); // ahora O(1)

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
    
    // x siempre llega hasta almenos maxLimX
    if( lim.maxX < maxLimX ) lim.maxX = maxLimX; 
    if( lim.maxX > maxLimX ) lim.minX = lim.maxX - maxLimX; 

    // a todas las series le quitamos la cola
    long long cnt = 0;
    float limiteIzq = lim.minX;
    for( auto& [id, serie] : series ){
        while( !serie.vacia() && serie.front().x < limiteIzq  ){
            
            serie.pop();
            cnt++;
            if( cnt >= 100000 ){
                DSV_LOG_ERROR("while ciclado");
                break;
            }
        }
    }

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
EspacioFase2D::EspacioFase2D( sf::Color color)    
    :GraficaBase(5000, color){    
    nombreEjeX = "X";
    nombreEjeY = "Y";
    ponerDesvanecido(true, true);
}

void EspacioFase2D::push_back(float x, float y , std::string clave){ 
    push_back_Gen({x, y}, clave);
}
void EspacioFase2D::recalcularExtremos(void){
    if( series.empty() ) return;

    bool haySeriesConDatos = false;

    Limites limAllSeries;
    bool primerSerie = true;
    for( auto const& [id, serie] : series ){
        if( serie.vacia() ) continue;
        haySeriesConDatos = true;

        Limites limSerie = serie.getLimites(); // ahora O(1)
        // la primer serie
        if( primerSerie ){
            limAllSeries = limSerie;        
            primerSerie = false;
        } 

        // actuaizar
        if( limSerie.minX < limAllSeries.minX) limAllSeries.minX = limSerie.minX;
        if( limSerie.maxX > limAllSeries.maxX) limAllSeries.maxX = limSerie.maxX;
        if( limSerie.minY < limAllSeries.minY) limAllSeries.minY = limSerie.minY;
        if( limSerie.maxY > limAllSeries.maxY) limAllSeries.maxY = limSerie.maxY;
        
    }
    // no actualizar nada si no tenemos datos
    if( !haySeriesConDatos ) return;


    //  --- margen  los limites que dan las series ---
    // 5% de margen
    float my = 0.05 * (limAllSeries.maxY - limAllSeries.minY); // margen
    float mx = 0.05 * (limAllSeries.maxX - limAllSeries.minX); // margen
    
    // poco mas de margen
    if( seguimiento ){
        mx *= 2;
        my *= 2;
    }

    limAllSeries = {   limAllSeries.minX-mx, limAllSeries.maxX+mx, limAllSeries.minY-my, limAllSeries.maxY+my };


    // --- --- --- ---  actualziar limites  --- --- --- ---
    // lim es la variable global de la grafica que se usa para mapear los puntos a pixeles, asi que aqui es donde se actualiza el zoom y el centro de la grafica

    // la primera vez que se actualizan los limites es diferente porque antes no hay datos, asi que se ponen esos limites, si no se expanden
    // para evitar que se expandan desde el 0,0 
    // queremos que se expandan desde donde  esten lod datos
    if( !YaSeActualizaronLimites ){
        lim = limAllSeries;
        YaSeActualizaronLimites = true;
        return;
    }

    // si se ajusta siempre
    if( seguimiento ) lim = limAllSeries;
    // si solo crece
    else{
        if( limAllSeries.minX < lim.minX) lim.minX = limAllSeries.minX;
        if( limAllSeries.maxX > lim.maxX) lim.maxX = limAllSeries.maxX;
        if( limAllSeries.minY < lim.minY) lim.minY = limAllSeries.minY;
        if( limAllSeries.maxY > lim.maxY) lim.maxY = limAllSeries.maxY;
    }


    // --- PERSONALZIACION EXTRA --- 

    // siempre graficar hasta el 0 Y (opcional)
    // if( lim.minY > 0 ) lim.minY = 0; 
    // if( lim.maxY < 0 ) lim.maxY = 0;
    

    // siempre graficar hasta el 0 X (opcional)
    //if( lim.minX > 0 ) lim.minX = 0; 
    //if( lim.maxX < 0 ) lim.maxX = 0;



    // ---  OJO CON DIVIDIR ENTRE 0 --- 
    // esta funcion esta en estructuras
    ajustarLimites_EvitarDivisionCero(lim);
}



// end dsv
}