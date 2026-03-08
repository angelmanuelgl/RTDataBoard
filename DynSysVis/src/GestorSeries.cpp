/*
    CONTIENE
        -> serie
        -> gestor de series

*/
#include "dsv/dataStructures/GestorSeries.hpp"


namespace dsv{



GestorSeries::GestorSeries( ){ 
    // La serie 0 siempre es "default" con color naranjita
    agregarSerie("default", sf::Color(200,150,20));
}

void GestorSeries::reajustarCapacidad(size_t idNecesario ){ 
    if( lista.size() <= idNecesario ){ 
        // Log de advertencia si el salto de ID es muy grande
        if( idNecesario > lista.size()  ){ 
            std::string x = std::to_string(lista.size());
            std::string y = std::to_string(idNecesario);
    
            DSV_LOG_WARN("Tienes " + x  + " indices y agregaste el indice" + y  );

            std::string reco = "Considera usar strings y los indices se manejaran automaticamente";
            DSV_LOG_WARN("Salto de indice detectado. Se recomienda usar indices consecutivos. " + reco);
        
        }

        // crecimiento amortizado para evitar realojos constantes
        while( lista.size() <= idNecesario ){ 
            lista.emplace_back();
            // configuramos la serie recien creada con los valores globales
            aplicarEstiloBase(lista.back());
            lista.back().configurar(dimensionGlobal, maxPointsGlobal);
        }
    }
}


// --- --- --- --- --- --- --- ---
// --- --- - Agregar Serie --- ---
// --- --- --- --- --- --- --- ---

// por paleta
size_t GestorSeries::agregarSerie(const std::string& nombre, const std::vector<sf::Color>& paleta ){ 
    DSV_LOG_INFO("agregando serie con nombre " + nombre );
    
    auto it = mapaNombres.find(nombre);
    
    // si ya existe actualizar color y paleta
    if( it != mapaNombres.end() ){ 
        DSV_LOG_INFO("la serie " + nombre + "ya existia" );
        size_t id = it->second;
        lista[id].paleta = paleta;
        return id;
    }
    
    // si no existe crear
    size_t nuevoId = lista.size();
    mapaNombres[nombre] = nuevoId;

    // Creamos la serie y asignamos la paleta
    DSV_LOG_INFO("se le aplicaran los estilos a la serie " + nombre );
    lista.emplace_back(paleta); 
    
    lista.back().configurar(dimensionGlobal, maxPointsGlobal);
    aplicarEstiloBase( lista.back() );
    lista[nuevoId].paleta = paleta;

    DSV_LOG_SUCCESS("serie agregada con nombre " + nombre );
    return nuevoId;
}

size_t GestorSeries::agregarSerie(size_t id, const std::vector<sf::Color>& paleta ){ 
    reajustarCapacidad(id);
    aplicarEstiloBase(lista[id]);
    lista[id].paleta = paleta;
    DSV_LOG_INFO("seriie con id " +  std::to_string(id)  + "agregado exitosamente");
    return id;
}

size_t GestorSeries::agregarSerie(const std::string& nombre, sf::Color col ){ 
    return agregarSerie(nombre, std::vector<sf::Color>{col});
}

size_t GestorSeries::agregarSerie(size_t id, sf::Color col ){ 
    return agregarSerie(id, std::vector<sf::Color>{col});
}

size_t GestorSeries::agregarSerie(const std::string& nombre ){ 
    return agregarSerie(nombre, paleta_global);
}

size_t GestorSeries::agregarSerie(size_t id){ 
    DSV_LOG_INFO("la serie con id " + std::to_string(id) + " se agregara con paleta global");
    return agregarSerie(id, paleta_global);
}



void GestorSeries::setMaxPointsSeries(size_t n) {
    maxPointsGlobal = n;
    for(auto& s : lista) s.setMaxPoints(n); // Cuidado: esto limpia la serie
}

// --- --- --- --- --- --- --- ---
// --- --- -- Push Back -- --- ---
// --- --- --- --- --- --- --- ---

void GestorSeries::push_back(const std::vector<float>& punto, size_t id ){ 
    if( id >= lista.size()) agregarSerie(id);
    lista[id].push_back(punto);
}

void GestorSeries::push_back(const std::vector<float>& punto, const std::string& nombre ){ 
    auto it = mapaNombres.find(nombre);
    if( it != mapaNombres.end() ){ 
        lista[it->second].push_back(punto);
    } else {
        size_t nuevoId = agregarSerie(nombre);
        lista[nuevoId].push_back(punto);
    }
}

// --- --- --- --- --- --- --- ---
// --- --- --- Colores --- --- ---
// --- --- --- --- --- --- --- ---

// Por ID
void GestorSeries::setColor(sf::Color col, size_t id ){ 
    if( id < lista.size() ){ 
        lista[id].setColor(col);
    }else {
        DSV_LOG_WARN("No se puede establecer color: el ID " + std::to_string(id) + " no existe.");
    }
}

// Por Nombre
void GestorSeries::setColor(sf::Color col, const std::string& nombre ){ 
    auto it = mapaNombres.find(nombre);
    if( it != mapaNombres.end() ){ 
       lista[it->second].setColor(col);
    }else {
        DSV_LOG_WARN("No se puede establecer color: la serie '" + nombre + "' no existe.");
    }
}

//Para todas las series (Global)
void GestorSeries::setColorSeries(sf::Color col ){
    paleta_global = std::vector<sf::Color>{col};
    for (auto& s : lista) s.setColor(col);
}

// --- --- --- --- --- --- --- ---
// ---  Colores por Paleta  --- ---
// --- --- --- --- --- --- --- ---

// Por ID
void GestorSeries::setColor(const std::vector<sf::Color>& paleta, size_t id ){ 
    if( id < lista.size()){
        lista[id].paleta = paleta;
    }else {
        DSV_LOG_WARN("No se puede establecer paleta: el ID " + std::to_string(id) + " no existe.");
    }
}

// Por Nombre
void GestorSeries::setColor(const std::vector<sf::Color>& paleta, const std::string& nombre ){ 
    auto it = mapaNombres.find(nombre);
    if( it != mapaNombres.end() ){ 
        lista[it->second].paleta = paleta;
    } else {
        DSV_LOG_WARN("No se puede establecer paleta: la serie '" + nombre + "' no existe.");
    }
}

//Para todas las series (Global)
void GestorSeries::setColorSeries(const std::vector<sf::Color>& paleta ){ 
    paleta_global = paleta;
    for (auto& s : lista ){ 
        s.paleta = paleta;
    }
}


// --- --- --- --- --- --- --- ---
// --- --- --- Grosores --- --- ---
// --- --- --- --- --- --- --- ---
// Por ID
void GestorSeries::setGrosor( float g, size_t id ){ 
    if( id < lista.size()){
       lista[id].grosor = g;
    }else {
        DSV_LOG_WARN("No se puede establecer  el grosor: el ID " + std::to_string(id) + " no existe.");
    }
}

// Por Nombre
void GestorSeries::setGrosor(float g, const std::string& nombre ){ 
    auto it = mapaNombres.find(nombre);
    if( it != mapaNombres.end() ){ 
       lista[it->second].grosor = g; 
    } else {
        DSV_LOG_WARN("No se puede establecer el grosor: la serie '" + nombre + "' no existe.");
    }
}

//Para todas las series (Global)
void GestorSeries::setGrosorSeries(float g ){ 
    grosor_global = g;
    for( auto& s : lista ){ 
        s.grosor = g;
    }
}



// --- --- --- --- --- --- --- ---
// --- --- --- Difuminado --- --- ---
// --- --- --- --- --- --- --- ---
// Por ID
void GestorSeries::setDifuminado( bool b, size_t id ){ 
    if( id < lista.size()){
       lista[id].difuminarCola = b;
    }else {
        DSV_LOG_WARN("No se puede establecer el difuminado: el ID " + std::to_string(id) + " no existe.");
    }
}

// Por Nombre
void GestorSeries::setDifuminado(bool b, const std::string& nombre ){ 
    auto it = mapaNombres.find(nombre);
    if( it != mapaNombres.end() ){ 
       lista[it->second].difuminarCola = b;
    } else {
        DSV_LOG_WARN("No se puede establecer el difuminado: la serie '" + nombre + "' no existe.");
    }
}

//Para todas las series (Global)
void GestorSeries::setDifuminadoSeries(bool b ){ 
    difuminar_global = b;
    for( auto& s : lista ){ 
        s.difuminarCola = b;
    }
}


// --- --- --- --- --- --- --- ---
// --- --- --- Delgazar Cola --- --- ---
// --- --- --- --- --- --- --- ---
// Por ID
void GestorSeries::setAdelgazado( bool b, size_t id ){ 
    if( id < lista.size()){
       lista[id].adelgazarCola = b;
    }else {
        DSV_LOG_WARN("No se puede establecer el grosor variable: el ID " + std::to_string(id) + " no existe.");
    }
}

// Por Nombre
void GestorSeries::setAdelgazado(bool b, const std::string& nombre ){ 
    auto it = mapaNombres.find(nombre);
    if( it != mapaNombres.end() ){ 
       lista[it->second].adelgazarCola = b;
    } else {
        DSV_LOG_WARN("No se puede establecer  el grosor variable: la serie '" + nombre + "' no existe.");
    }
}

//Para todas las series (Global)
void GestorSeries::setAdelgazadoSeries(bool b ){ 
    adelgazar_global = b;
    for( auto& s : lista ){ 
        s.adelgazarCola = b;
    }
}


// --- --- --- --- --- --- --- ---
// ---  Acceso rapido a contenido de la serie  --- ---
// --- --- --- --- --- --- --- ---


Serie& GestorSeries::operator[](const std::string& nombre ){ 
    auto it = mapaNombres.find(nombre);
    if( it == mapaNombres.end()) agregarSerie(nombre);
    return lista[mapaNombres[nombre]];
}

Serie& GestorSeries::operator[](size_t id ){ 
    if( id >= lista.size()) reajustarCapacidad(id);
    return lista[id];
}

} // dsv