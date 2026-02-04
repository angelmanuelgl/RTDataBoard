/*  * autor: Angel Manuel Gonzalez Lopez 
    * github: https://github.com/angelmanuelgl
    * web: https://angelmanuelgl.github.io/
    * 
    * proyecto: InsightRT - - - - - - - - - - - - - - - - - - - 
    * libreria de herramientas graficas para monitoreo de datos 
    * en tiempo real y comportamiento de sistemas complejos.
*/
/*  PANEL.hpp
    gestion de paneles dinamicos
    posicionamiento y obtienen mas
*/


#include "panel.hpp"


/*
    STRUCT PANEL
    generar paneles dinamicos
*/
Panel::Panel(sf::RenderWindow& window_,  sf::Color extColor,  int nx, int ny , sf::Color bgColor):
    window(window_) {
    // --- calcular el tamano ---
    sf::Vector2u windowSize = window.getSize();
    
    // --- espacio TOTAL disponible ---
    // restando los margenes de la ventana
    float disponibleX = static_cast<float>(windowSize.x) - (2.0f * margenVentana);
    float disponibleY = static_cast<float>(windowSize.y) - (2.0f * margenVentana);

    // -- restar espaciado entre paneles ---
    // si nx es 1, no hay huecos. Si nx es 2, hay 1 hueco .... 
    float huecosX = (nx > 1) ? (nx - 1) * espaciado : 0.f;
    float huecosY = (ny > 1) ? (ny - 1) * espaciado : 0.f;

    // dividir el espacio neto restante entre el número de paneles
    float x = (disponibleX - huecosX) / nx;
    float y = (disponibleY - huecosY) / ny;
    
    size = {x, y};

    // generar
    elMarco.generar(size, radio, bgColor, extColor);
    // background = generarRectanguloRelleno(size, radio, 40, bgColor);
    // generarBorde(contorno, {0,0}, size, radio, 2.0f, 40, extColor);
}

void Panel::setPosition(float x, float y) {
    pos_actual = {x, y};
    mytransform = sf::Transform::Identity;
    mytransform.translate(x, y);
}

void Panel::configurarMedidas( float r, float esp, float margen){ 
    radio = r; 
    espaciado = esp; 
    margen = margenVentana;  
}

void Panel::positionAbsoluta(Ubicacion ubi){
    sf::Vector2u winSize = window.getSize();
    float x = 0, y = 0;
    switch(ubi) {
        case Ubicacion::ArribaIzq:    x = margenVentana; y = margenVentana; break;
        case Ubicacion::ArribaCentro: x = (winSize.x - size.x) / 2.f; y = margenVentana; break;
        case Ubicacion::ArribaDer:    x = winSize.x - size.x - margenVentana; y = margenVentana; break;
        case Ubicacion::AbajoIzq:     x = margenVentana; y = winSize.y - size.y - margenVentana; break;
        case Ubicacion::AbajoCentro:  x = (winSize.x - size.x) / 2.f; y = winSize.y - size.y - margenVentana; break;
        case Ubicacion::AbajoDer:     x = winSize.x - size.x - margenVentana; y = winSize.y - size.y - margenVentana; break;
        case Ubicacion::Centro:       x = (winSize.x - size.x) / 2.f; y = (winSize.y - size.y) / 2.f; break;
        case Ubicacion::CentroIzq:    x = margenVentana;  y = (winSize.y - size.y) / 2.f; break;
        case Ubicacion::CentroDer:    x = winSize.x - size.x - margenVentana;  y = (winSize.y - size.y) / 2.f; break;
    }
    setPosition(x, y);
}

void Panel::positionRelativa(RelativoA rel, const Panel& other){
    sf::Vector2f oPos = other.getPosition();
    sf::Vector2f oSize = other.getSize();
    float x = oPos.x;
    float y = oPos.y;
    switch (rel) {
        case RelativoA::Arriba: y = oPos.y - size.y - espaciado; break;
        case RelativoA::Abajo:  y = oPos.y + oSize.y + espaciado; break;
        case RelativoA::Izq:    x = oPos.x - size.x - espaciado; break;
        case RelativoA::Der:    x = oPos.x + oSize.x + espaciado; break;
    }
    setPosition(x, y);
}
 
sf::RenderStates Panel::getInternalState() const {
    sf::RenderStates states;
    states.transform = mytransform;
    return states;
}

void Panel::draw(void) {
    elMarco.draw(window, mytransform); 

    if( contenido ){
        // getInternalState() devuelve el sf::RenderStates con mytransform
        contenido->draw(window, getInternalState(), size);
    } 
}

/*
    como intertactua el panel con sus elementosd 
*/
void Panel::setContenido(std::unique_ptr<Objeto> nuevoContenido) {
    contenido = std::move(nuevoContenido);
}

