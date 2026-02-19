/*  * autor: Angel Manuel Gonzalez Lopez 
    * github: https://github.com/angelmanuelgl
    * web: https://angelmanuelgl.github.io/
    * 
    * - - - - - - -  -  DynSysVis  - - - - - - - - - - 
    * Dynamical System Visualizer Real-Time
    * libreria de herramientas graficas para monitoreo de datos 
    * y comportamiento de sistemas complejos en tiempo Real.
*/
/*  TITULO.hpp
    gestion de titulo
    para no sobrecagar el panel esto se hara aparte
    el panel se encargara de llamar a este objeto
*/
#ifndef TITULO_HPP
#define TITULO_HPP

namespace dsv{

class Titulo{
private:
    sf::Text elTitulo;
    float alturaReservada;
    // formato


public:
    void actualizaAltura(){
        double baseHeight = elTitulo.getFont()->getLineSpacing(elTitulo.getCharacterSize());
        alturaReservada = baseHeight*1.8;
    }

    Titulo(const std::string& str, const sf::Font& font, unsigned int size= 12){
        elTitulo.setFont(font);
        elTitulo.setString(str);
        elTitulo.setCharacterSize(size);
        elTitulo.setFillColor(sf::Color::White);
        // margen interno para que no pegue al borde del panel
        actualizaAltura();
    }

    void draw(sf::RenderWindow& window, sf::RenderStates states) {
        const sf::Font* font = elTitulo.getFont();
        if (!font) return;

        unsigned int size = elTitulo.getCharacterSize();
        
        // 1. En lugar de LineSpacing, usamos la suma de la altura de ascenso y descenso
        // Esto nos da la altura real que ocupan las letras "físicamente"
        float ascent = font->getLineSpacing(size); 
        
        // SFML a veces mete un offset en el bounding box // jugar con este pinhce numero 0,.8 1.0 1.1
        float alturaReal = ascent * 1.0f; 

        // centrado
        float posY = std::round((alturaReservada - alturaReal) / 2.0f);
        

        elTitulo.setPosition(15.f, posY); 
        window.draw(elTitulo, states);
    }

    
    float getAltura() const { return alturaReservada; }

    // personalizar
    void setSize( unsigned int s ){
        elTitulo.setCharacterSize( s ); 
        actualizaAltura();
    }
    void setString(const std::string& s){ 
        elTitulo.setString(s);   
        actualizaAltura();
    }
    void setFont( const sf::Font& f ){ 
        elTitulo.setFont(f); 
        actualizaAltura();
    }
};


// end dvs
}
#endif