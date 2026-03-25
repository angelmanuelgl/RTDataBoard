/*  * autor: Angel Manuel Gonzalez Lopez 
    * github: https://github.com/angelmanuelgl
    * web: https://angelmanuelgl.github.io/
    * 
    * - - - - - - -  -  DynSysVis  - - - - - - - - - - 
    * Dynamical System Visualizer Real-Time
    * libreria de herramientas graficas para monitoreo de datos 
    * y comportamiento de sistemas complejos en tiempo Real.
*/
/*  PANEL.hpp
    gestion de paneles dinamicos
    posicionamiento y obtienen mas


    el gestor del contenido, esto unifica
    -> objeto generico
    -> titulo
    -> posciion
*/

#include "dsv/core/Panel.hpp"

namespace dsv{
/*
    STRUCT PANEL
    generar paneles dinamicos
*/
Panel::Panel(sf::RenderWindow& window_,  const std::string& tituloPanel, sf::Color extColor):
    window(window_), extColor(extColor) {   
    
    // asignar color en automatcio
    bgColor=sf::Color(30,30,30);
    
    // poner fuente por defecto 
    yafuenteCargada = false;
    if( tituloPanel != "" ){
        cargarFuenteSiFalta();
        ponerTitulo(tituloPanel, fuentePredeterminada);
    }
    
    
    //  se espera que el panel se use llamando el tablero asi que esto es de emergencia
    // el tableor lo pociciona y le dice cuanto medir
    setSize(2,2);
    setPosition(0,0);

    elMarco.generar(size, radio, bgColor, extColor);

    // tecnicamente con llamar al consturctor no basta hay que llamar a 
    // sizeEnRejilla  y positionEnRejilla para que funcione
    // esto ya lo hace el layout asi que no deberia haber problema
    
}
void Panel::setDegradado(sf::Color colorTop, sf::Color colorBot) {
    this->bgColor = colorTop;     // principal
    this->bgBotColor = colorBot;  // degrado hacia abajo
    
    // El color de borde (extColor) se mantiene igual
    elMarco.generar(size, radio, bgColor, bgBotColor, extColor, borde);
}
void Panel::setColorFondo(sf::Color color ){
    setDegradado(color,color);
}
void Panel::setBorderWidth(float grosor) {
    this->borde = grosor;
    elMarco.generar(size, radio, bgColor, bgBotColor, extColor, borde);
}
void Panel::setRadio(float r) {
    this->radio = r;
    elMarco.generar(size, radio, bgColor, bgBotColor, extColor, borde);
}

// ---- ---- --- --- --- --- --- --- --- --- 
// ---- ---- --- posicionamiento --- --- --- 
// ---- ---- --- --- --- --- --- --- --- --- 

void Panel::setSize(double nx, double ny){
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
    elMarco.generar(size, radio, bgColor, extColor);
}

void Panel::setSizeAbsoluto(sf::Vector2f tamano) {
    size = tamano;
    elMarco.generar(size, radio, bgColor, extColor);
}

void Panel::setPosition(float x, float y) {
    pos_actual = {x, y};
    mytransform = sf::Transform::Identity;
    mytransform.translate(x, y);
}

void Panel::configurarMedidas( float r, float esp, float margen){ 
    this->radio = r; 
    this->espaciado = esp; 
    this->margenVentana = margen;  
}

void Panel::positionAbsoluta(Ubicacion ubi){
    DSV_LOG_WARN("recomednado usar funcion positionEnRejilla con objeto Tablero");
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
    DSV_LOG_WARN("recomednado usar funcion positionEnRejilla con objeto Tablero");
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

void Panel::sizeEnRejilla(int spanF, int spanC, int totalFilas, int totalCols){
    setSize( (double)totalCols / spanC, (double)totalFilas / spanF );
}

void Panel::positionEnRejilla(int fila, int col, int totalFilas, int totalCols ){
    // cuanto mide cada celda
    sf::Vector2u winSize = window.getSize();
    float dispX = (float)winSize.x - (2.0f * margenVentana);
    float dispY = (float)winSize.y - (2.0f * margenVentana);
    float celdaX = (dispX - (totalCols - 1) * espaciado) / totalCols;
    float celdaY = (dispY - (totalFilas - 1) * espaciado) / totalFilas;

    // coordenas
    float x = margenVentana + col * (celdaX + espaciado);
    float y = margenVentana + fila * (celdaY + espaciado);

    
    setPosition(x, y);
}


sf::RenderStates Panel::getInternalState() const {
    sf::RenderStates states;
    states.transform = mytransform;
    return states;
}

/*
     --- --- --- D I B U J A R --- ---
*/
void Panel::aplicarRecorte(const sf::RenderWindow& window, sf::Vector2f pos, sf::Vector2f tam) {
    // 1. Obtener la posición en la pantalla (considerando el View actual de SFML)
    sf::Vector2i posPantalla = window.mapCoordsToPixel(pos);
    sf::Vector2i tamPantalla = window.mapCoordsToPixel(pos + tam) - posPantalla;

    // 2. Voltear el eje Y para OpenGL (SFML 0 arriba -> OpenGL 0 abajo)
    int yReverso = window.getSize().y - (posPantalla.y + tamPantalla.y);

    // 3. Activar el recorte
    glEnable(GL_SCISSOR_TEST);
    glScissor(posPantalla.x, yReverso, tamPantalla.x, tamPantalla.y);
}


void Panel::draw(void) {
  
    // dibujar el marco al final
    elMarco.draw(window, mytransform); 
    
    // getInternalState() devuelve el sf::RenderStates con mytransform
    sf::RenderStates states = getInternalState();
    sf::Vector2f sizeContenido = size;
    sf::Vector2f posContenido = pos_actual;


    // recorte
    aplicarRecorte(window, posContenido, sizeContenido);

    // si tiene titulo, reservar espacio
    if( titulo ){   
        titulo->draw(window, states);
        float h = titulo->getAltura();
        
        // desplazar hacia abajo el contenido
        states.transform.translate(0, h);
        posContenido.y += h; // 
        sizeContenido.y -= h;
    }

    // si tiene contenido se dibujar
    if( contenido ){
        contenido->draw(window, states, sizeContenido); 
    } 

    // temrianr recorde
    glDisable(GL_SCISSOR_TEST);
    elMarco.drawCont(window, mytransform); 
}

/*
   titulo 
*/
void Panel::cargarFuenteSiFalta(){
    if( !yafuenteCargada ){
        Objeto::cargarFuenteGlobal(); // Nos aseguramos que esté cargada
        fuentePredeterminada = Objeto::fuenteCompartida; 
        yafuenteCargada = Objeto::fuenteCargada;
    }
}
void Panel::ponerTitulo(const std::string& texto, const sf::Font& fuente){
    titulo = std::make_unique<Titulo>(texto, fuente);
}

/*
    como intertactua el panel con sus elementoss
*/

void Panel::setContenido(std::unique_ptr<Objeto> nuevoContenido ){
    contenido = std::move(nuevoContenido);
}


// end dsv
}