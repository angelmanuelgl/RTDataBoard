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


#include "Tablero.hpp"


namespace dsv {
    /*
       L A Y O U T
    */

    Layout::Layout(std::initializer_list<std::string> lineas ){
        auto matriz = tokenizar(lineas);
        if( matriz.empty() ) return;

        totalFilas = matriz.size();
        totalCols = matriz[0].size();
        calcularRegiones(matriz);
        validar(matriz);
    }

    // texto -> matriz
    std::vector<std::vector<std::string>> Layout::tokenizar(std::initializer_list<std::string> lineas){
        std::vector<std::vector<std::string>> matriz;
        for( const auto& linea : lineas ){
            std::vector<std::string> fila;
            std::stringstream ss(linea); // convertir a flujo apra extraer palabras
            std::string t;
            while( ss >> t) fila.push_back(t);
            if(!fila.empty()) matriz.push_back(fila);
        }
        return matriz;
    }

    // limites y llena regiones
    void Layout::calcularRegiones(const std::vector<std::vector<std::string>>& m ){
        struct Box { int minF = 9999, maxF = 0, minC = 9999, maxC = 0; };
        std::map<std::string, Box> b;

        
        for( int f = 0; f < totalFilas; f++ ){
            int tokensEnFila = (int)m[f].size();
            for( int c = 0; c < tokensEnFila; c++ ){
                std::string id = m[f][c];
                if( id == ".") continue;
              
                // Si el ID no existe en regiones, lo inicializamos
                if( regiones.find(id) == regiones.end() ){
                    regiones[id] = {f, c, 1, 1, tokensEnFila};
                } 
                // Si el ID ya existe (es un panel multilineal), actualizamos
                else {
                    auto& r = regiones[id];
                    r.cnt_f = std::max(r.cnt_f, f - r.fil + 1);
                    r.cnt_c = std::max(r.cnt_c, c - r.col + 1);
                    //! un panel multilinea siempre tiene el mismo ancho de tokens
                    //! hayq ue ver como gestionar eso
                }
            }
        }

        // llenar 
        for( auto const& [id, l] : b)  
            regiones[id] = {l.minF, l.minC, l.maxF - l.minF + 1, l.maxC - l.minC + 1};
    }

    // se usa cuando ya se calculo la matriz y regiones
    void Layout::validar(const std::vector<std::vector<std::string>>& matriz){
        // contar AREA ( cantidad de casillas )
        std::map<std::string, int> conteoReal;
        for (const auto& fila : matriz ){
            for (const auto& id : fila ){
                if( id != ".") conteoReal[id]++;
            }
        }
        // comparar cque se calculo en las regiones
        for( auto const& [id, reg] : regiones ){
            int areaEsperada = reg.cnt_f * reg.cnt_c;
            if( conteoReal[id] != areaEsperada ){
                DSV_LOG_ERROR(  "ERROR DSV: El ID " + id + " no forma un rectangulo valido o esta disperso" );
            }
        }
    }

    /*
        T A B L E R O
    */

    Tablero::Tablero( sf::RenderWindow& ventana,  sf::Color color1,  sf::Color color2)
        :window(ventana),colorFondo1(color1), colorFondo2(color2), fondo(sf::Quads, 4 ){
        
        setFondoDegradado( colorFondo1, colorFondo2);

        // layour por defecto
        layoutActual = std::make_unique<Layout>(std::initializer_list<std::string>{
            "A1 . B1",
            "A2 . B2",
            "A3 . B3"
        });
    }

    Tablero::Tablero(sf::RenderWindow& ventana, const Layout& layout, sf::Color color1, sf::Color color2)
        : window(ventana), colorFondo1(color1), colorFondo2(color2), fondo(sf::Quads, 4 ){
        
        layoutActual = std::make_unique<Layout>(layout);

        setFondoDegradado(colorFondo1, colorFondo2);
    }

    /*  ---  GESTIOANR EL LAYOUR --- */

    void Tablero::setLayout(const Layout& l ){
        layoutActual = std::make_unique<Layout>(l);
        aplicarLayout();
    }


    void Tablero::aplicarLayoutaEste( Panel& panel,  const std::string& id){
      

        if( !layoutActual ){
            DSV_LOG_ERROR("no hay un LAYOUT");
            return;
        }

        if( !layoutActual->regiones.count(id) ){
            DSV_LOG_WARN("El ID '" + id + "' no existe en el Layout. El panel no se posicionara.");
        }

        int totalFilas = (float)layoutActual->totalFilas;
        int totalCols = (float)layoutActual->totalCols;
     

        auto r = layoutActual->regiones.at(id);

        totalCols = (float)r.colsEnSuFila; // filas dinamicas

        panel.sizeEnRejilla(r.cnt_f, r.cnt_c, totalFilas, totalCols);
        panel.positionEnRejilla(r.fil, r.col, totalFilas, totalCols);
    }


    void  Tablero::aplicarLayout( ){
        if( !layoutActual) return;

        for( auto& pair : panelesConId ){
            const std::string& id = pair.first;
            Panel& panel = *pair.second;
            aplicarLayoutaEste( panel, id);
        }
    }




    /*  ---  DIBUAJR --- */
    void Tablero::setFondoDegradado( sf::Color color1, sf::Color color2){
        sf::Vector2u size = window.getSize();

        // superior izquierda
        fondo[0].position = sf::Vector2f(0, 0);
        fondo[0].color = color1;

        // superior derecha
        fondo[1].position = sf::Vector2f(size.x, 0);
        fondo[1].color = color1;

        // inferior derecha
        fondo[2].position = sf::Vector2f(size.x, size.y);
        fondo[2].color = color2; 

        // inferior izquierda
        fondo[3].position = sf::Vector2f(0, size.y);
        fondo[3].color = color2;
    }

    void Tablero::draw( ){
        // dibujar el fondo
        window.clear(colorFondo1);
        window.draw(fondo);

        // paneles
        for( auto& pair : panelesConId ){
            
            const std::string& id = pair.first;
            Panel& panel = *pair.second;

            if( !layoutActual->regiones.count(id) ) continue;
           panel.draw(); 
        }
    }



// termina dsv
}