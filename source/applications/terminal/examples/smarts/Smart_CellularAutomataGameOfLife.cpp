/*
 * File:   Smart_CellularAutomataGameOfLife.cpp
 *
 * Minimal terminal example that runs Conway's Game of Life through the
 * GenESyS CellularAutomataComp component.
 */

#include "Smart_CellularAutomataGameOfLife.h"

#include "kernel/simulator/Simulator.h"
#include "plugins/components/ModalModel/CellularAutomataComp.h"

#include <iostream>
#include <vector>

namespace {

void SetBlinker(CellularAutomataComp* cellularAutomata) {
	cellularAutomata->setCellState({2, 1}, 1);
	cellularAutomata->setCellState({2, 2}, 1);
	cellularAutomata->setCellState({2, 3}, 1);
}

void PrintGrid(CellularAutomataComp* cellularAutomata, unsigned short width, unsigned short height) {
	for (unsigned short y = 0; y < height; ++y) {
		for (unsigned short x = 0; x < width; ++x) {
			std::cout << cellularAutomata->getlattice()->getCell({static_cast<int>(x), static_cast<int>(y)})->getCurrentState().getValue();
		}
		std::cout << std::endl;
	}
}

}

Smart_CellularAutomataGameOfLife::Smart_CellularAutomataGameOfLife() {
}

int Smart_CellularAutomataGameOfLife::main(int argc, char** argv) {
	Simulator* genesys = new Simulator();
	genesys->getTraceManager()->setTraceLevel(TraceManager::Level::L0_noTraces);
	setDefaultTraceHandlers(genesys->getTraceManager());

	Model* model = genesys->getModelManager()->newModel();

	CellularAutomataComp* cellularAutomata = new CellularAutomataComp(model);
	cellularAutomata->setCellularAutomataType(CellularAutomataComp::CellularAutomataType::CLASSIC);
	cellularAutomata->setLatticeType(CellularAutomataComp::LatticeType::RETICULAR);
	cellularAutomata->getlattice()->setDimensions({5, 5});
	cellularAutomata->setNeighboorhoodType(CellularAutomataComp::NeighboorhoodType::MOORE);
	cellularAutomata->getNeighboorhood()->setRadius(1);
	cellularAutomata->setBoundaryType(CellularAutomataComp::BoundaryType::FIXED);
	cellularAutomata->setStateSetType(CellularAutomataComp::StateSetType::ENUMERATED);
	cellularAutomata->setLocalRuleType(CellularAutomataComp::LocalRuleType::GAME_OF_LIFE);

	std::string errorMessage;
	if (!cellularAutomata->initializeCellularAutomata(&errorMessage)) {
		std::cout << "Could not initialize CellularAutomataComp: " << errorMessage << std::endl;
		delete genesys;
		return 1;
	}

	SetBlinker(cellularAutomata);

	std::cout << "Game of Life through CellularAutomataComp - blinker" << std::endl;
	std::cout << "2D lattice, 5x5 cells, Moore radius-1 neighborhood, fixed boundary" << std::endl;
	std::cout << std::endl;

	const unsigned short width = 5;
	const unsigned short height = 5;
	const unsigned int steps = 2;
	std::cout << "t0:" << std::endl;
	PrintGrid(cellularAutomata, width, height);
	for (unsigned int step = 1; step <= steps; ++step) {
		cellularAutomata->stepCellularAutomata();
		std::cout << std::endl;
		std::cout << "t" << step << ":" << std::endl;
		PrintGrid(cellularAutomata, width, height);
	}

	delete genesys;
	return 0;
}
