/*  * autor: Angel Manuel Gonzalez Lopez 
    * github: https://github.com/angelmanuelgl
    * web: https://angelmanuelgl.github.io/
    * 
    * - - - - - - -  -  DynSysVis  - - - - - - - - - - 
    * Dynamical System Visualizer Real-Time
    * libreria de herramientas graficas para monitoreo de datos 
    * y comportamiento de sistemas complejos en tiempo Real.
*/
/*  Tablero.hpp
    tiene 
*/

#ifndef TABLERO_HPP
#define TABLERO_HPP

#include "Vista.hpp"
#include "Logger.hpp"
#include <map>
#include <vector>
#include <string>

#include <SFML/Graphics.hpp>

namespace dsv {
struct Region { int fil, col, cnt_f, cnt_c, colsEnSuFila; };

struct Layout {
    int totalFilas = 0, totalCols = 0;
    std::map<std::string, Region> regiones;

    Layout(std::initializer_list<std::string> lineas);

private:
    // texto -> matriz
    std::vector<std::vector<std::string>> tokenizar(std::initializer_list<std::string> lineas);

    // limites y llena regiones
    void calcularRegiones(const std::vector<std::vector<std::string>>& m);

    // se usa cuando ya se calculo la matriz y regiones
    void validar(const std::vector<std::vector<std::string>>& matriz);
    
};

class Tablero {
     private:
        sf::RenderWindow& window;
        std::vector<std::pair<std::string, std::unique_ptr<Panel>>> panelesConId;
        std::unique_ptr<Layout> layoutActual;
        
        sf::Color colorFondo1;
        sf::Color colorFondo2;
        sf::VertexArray fondo;

        void dibujarFondoEspecial(); // Función auxiliar
    
    public:
        
        Tablero( sf::RenderWindow& ventana,  
                 sf::Color color1 = sf::Color(40, 40, 40),
                 sf::Color color2 = sf::Color(30, 30, 30)  );
        
        // recibir layour
        Tablero( sf::RenderWindow& ventana, const Layout& layoutInicial, 
                 sf::Color color1 = sf::Color(40, 40, 40),
                 sf::Color color2 = sf::Color(30, 30, 30));

        // gestioanr el layout
        void setLayout(const Layout& l);
        void aplicarLayoutaEste( Panel& panel,  const std::string& id);
        void aplicarLayout();

        // dibujar
        void setFondoDegradado( sf::Color color1, sf::Color color2);   
        void draw();

        // TODO, mejorar esta cosa
        template<typename T, typename... Args>
        Vista<T> add( const std::string& titulo, sf::Color color, const std::string& idLayout, Args&&... args){
            
            auto p = std::make_unique<Panel>(window, titulo, color);
            T* graf = p->crearContenido<T>(std::forward<Args>(args)...);
            
            Panel& refPanel = *p;
            // paneles.push_back(std::move(p));
            panelesConId.push_back({idLayout, std::move(p)});

            // aplicarle a este panel
            aplicarLayoutaEste(refPanel, idLayout );


            Vista<T>  ans =   { *graf, refPanel };
            return ans;
        }



    };
}

#endif