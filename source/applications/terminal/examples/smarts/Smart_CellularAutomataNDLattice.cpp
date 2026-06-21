/*
 * File:   Smart_CellularAutomataNDLattice.cpp
 *
 * Minimal terminal example that runs a 3D reticular cellular automaton through
 * the GenESyS CellularAutomataComp component.
 */

#include "Smart_CellularAutomataNDLattice.h"

#include "kernel/simulator/Simulator.h"
#include "plugins/components/ModalModel/CellularAutomataComp.h"

#include <iostream>
#include <string>
#include <vector>

namespace {

CellularAutomataComp* CreateAutomaton(Model* model, CellularAutomataComp::NeighboorhoodType neighborhoodType) {
	CellularAutomataComp* cellularAutomata = new CellularAutomataComp(model);
	cellularAutomata->setCellularAutomataType(CellularAutomataComp::CellularAutomataType::CLASSIC);
	cellularAutomata->setLatticeType(CellularAutomataComp::LatticeType::RETICULAR);
	cellularAutomata->getlattice()->setDimensions({3, 3, 3});
	cellularAutomata->setNeighboorhoodType(neighborhoodType);
	cellularAutomata->getNeighboorhood()->setRadius(1);
	cellularAutomata->setBoundaryType(CellularAutomataComp::BoundaryType::CLOSED);
	cellularAutomata->setStateSetType(CellularAutomataComp::StateSetType::ENUMERATED);
	cellularAutomata->setLocalRuleType(CellularAutomataComp::LocalRuleType::BIASED_COMPETITION);
	return cellularAutomata;
}

bool Initialize(CellularAutomataComp* cellularAutomata) {
	std::string errorMessage;
	if (!cellularAutomata->initializeCellularAutomata(&errorMessage)) {
		std::cout << "Could not initialize CellularAutomataComp: " << errorMessage << std::endl;
		return false;
	}
	return true;
}

void SetInitial3DPattern(CellularAutomataComp* cellularAutomata) {
	cellularAutomata->setCellState({1, 1, 1}, 1);
	cellularAutomata->setCellState({0, 1, 1}, 1);
	cellularAutomata->setCellState({2, 1, 1}, 1);
	cellularAutomata->setCellState({1, 0, 1}, 1);
	cellularAutomata->setCellState({1, 2, 1}, 1);
	cellularAutomata->setCellState({1, 1, 0}, 1);
	cellularAutomata->setCellState({1, 1, 2}, 1);
}

void PrintSlices(CellularAutomataComp* cellularAutomata, const std::string& title) {
	std::cout << title << std::endl;
	for (int z = 0; z < 3; ++z) {
		std::cout << "z=" << z << std::endl;
		for (int y = 0; y < 3; ++y) {
			for (int x = 0; x < 3; ++x) {
				Cell* cell = cellularAutomata->getlattice()->getCell({x, y, z});
				std::cout << cell->getCurrentState().getValue();
			}
			std::cout << std::endl;
		}
	}
}

unsigned long CountCenterNeighbors(Model* model, CellularAutomataComp::NeighboorhoodType neighborhoodType) {
	CellularAutomataComp* cellularAutomata = CreateAutomaton(model, neighborhoodType);
	if (!Initialize(cellularAutomata))
		return 0;
	Cell* center = cellularAutomata->getlattice()->getCell({1, 1, 1});
	return center->getNeighbors().size();
}

}

Smart_CellularAutomataNDLattice::Smart_CellularAutomataNDLattice() {
}

int Smart_CellularAutomataNDLattice::main(int argc, char** argv) {
	Simulator* genesys = new Simulator();
	genesys->getTraceManager()->setTraceLevel(TraceManager::Level::L0_noTraces);
	setDefaultTraceHandlers(genesys->getTraceManager());

	Model* model = genesys->getModelManager()->newModel();

	std::cout << "N-dimensional CellularAutomataComp example" << std::endl;
	std::cout << "3D reticular lattice, 3x3x3 cells, closed boundary" << std::endl;
	std::cout << std::endl;

	std::cout << "Moore 3D radius 1 center neighbors: "
			<< CountCenterNeighbors(model, CellularAutomataComp::NeighboorhoodType::MOORE) << std::endl;
	std::cout << "Von Neumann 3D radius 1 center neighbors: "
			<< CountCenterNeighbors(model, CellularAutomataComp::NeighboorhoodType::VONNEUMANN) << std::endl;

	CellularAutomataComp* cellularAutomata = CreateAutomaton(model, CellularAutomataComp::NeighboorhoodType::MOORE);
	if (!Initialize(cellularAutomata)) {
		delete genesys;
		return 1;
	}

	std::cout << "Total cells: " << cellularAutomata->getlattice()->getCellsSize() << std::endl;
	std::cout << "Invalid coordinate {-1, 0, 0} accepted: "
			<< (cellularAutomata->setCellState({-1, 0, 0}, 1) ? "yes" : "no") << std::endl;
	std::cout << std::endl;

	SetInitial3DPattern(cellularAutomata);
	PrintSlices(cellularAutomata, "t0:");
	for (unsigned int step = 1; step <= 2; ++step) {
		cellularAutomata->stepCellularAutomata();
		std::cout << std::endl;
		PrintSlices(cellularAutomata, "t" + std::to_string(step) + ":");
	}

	delete genesys;
	return 0;
}
