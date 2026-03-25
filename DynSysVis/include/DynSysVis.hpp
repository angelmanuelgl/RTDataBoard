/*  * autor: Angel Manuel Gonzalez Lopez 
    * github: https://github.com/angelmanuelgl
    * web: https://angelmanuelgl.github.io/
    * 
    NOMBRE:
    Dynamical System Visualizer Real-Time
    Dynamical System Visualizer RT
    Dynamical System Visualizer
    DynSysVis
    dsv
    
*/
/*  
    para incluir todo mas facilmente
*/

#ifndef DYNSYSVIS_HPP
#define DYNSYSVIS_HPP


// --- estructuras de Datos Base ---
#include "dsv/dataStructures/Estructuras.hpp"

// --- el nucleo de paneles ---
#include "dsv/core/Objeto.hpp"
#include "dsv/core/Logger.hpp"
#include "dsv/core/Panel.hpp"
#include "dsv/core/Titulo.hpp"

// --- interfaz y layour ---
#include "dsv/interface/Tablero.hpp"
#include "dsv/interface/Vista.hpp"
#include "dsv/interface/Temas.hpp"

// --- graficos en 2D ---
#include "dsv/graphics2D/Geometria.hpp"
#include "dsv/graphics2D/Graficas.hpp"
#include "dsv/graphics2D/GraficoCircular.hpp"

// --- graficos en 3D  ---
#include "dsv/graphics3D/Grafica3D.hpp"
// #include "dsv/graphics3D/GraficaND.hpp"

// --- menu ---
#include "dsv/menu/PanelFlotante.hpp"


// --- Estilo ---


// --- Simulacion Matematicas ---
#include "dsv/simulation/Integradores.hpp"

#include "dsv/simulation/Lorenz.hpp"
#include "dsv/simulation/ORG.hpp"
#include "dsv/simulation/SIR.hpp"

#endif