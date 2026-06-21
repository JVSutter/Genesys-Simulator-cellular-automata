/*
 * File:   Smart_CellularAutomataCompRule90.cpp
 *
 * Minimal terminal example that runs elementary cellular automaton Rule 90
 * through the GenESyS CellularAutomataComp component.
 */

#include "Smart_CellularAutomataCompRule90.h"

#include "kernel/simulator/Simulator.h"
#include "plugins/components/ModalModel/CellularAutomataComp.h"

#include <iostream>
#include <string>

namespace {

void SetInitialPattern(CellularAutomataComp* cellularAutomata, const std::string& pattern) {
	for (unsigned long cellNumber = 0; cellNumber < pattern.size(); ++cellNumber) {
		const long value = pattern.at(cellNumber) == '1' ? 1 : 0;
		cellularAutomata->setCellState(static_cast<long>(cellNumber), value);
	}
}

}

Smart_CellularAutomataCompRule90::Smart_CellularAutomataCompRule90() {
}

int Smart_CellularAutomataCompRule90::main(int argc, char** argv) {
	Simulator* genesys = new Simulator();
	genesys->getTraceManager()->setTraceLevel(TraceManager::Level::L0_noTraces);
	setDefaultTraceHandlers(genesys->getTraceManager());

	Model* model = genesys->getModelManager()->newModel();

	CellularAutomataComp* cellularAutomata = new CellularAutomataComp(model);
	cellularAutomata->setCellularAutomataType(CellularAutomataComp::CellularAutomataType::CLASSIC);
	cellularAutomata->setLatticeType(CellularAutomataComp::LatticeType::RETICULAR);
	cellularAutomata->getlattice()->setDimensions({7});
	cellularAutomata->setNeighboorhoodType(CellularAutomataComp::NeighboorhoodType::CENTERED);
	cellularAutomata->getNeighboorhood()->setRadius(1);
	cellularAutomata->setBoundaryType(CellularAutomataComp::BoundaryType::CLOSED);
	cellularAutomata->setStateSetType(CellularAutomataComp::StateSetType::ENUMERATED);
	cellularAutomata->setElementaryRuleNumber(90);
	cellularAutomata->setLocalRuleType(CellularAutomataComp::LocalRuleType::ELEMENTAR_CA);

	std::string errorMessage;
	if (!cellularAutomata->initializeCellularAutomata(&errorMessage)) {
		std::cout << "Could not initialize CellularAutomataComp: " << errorMessage << std::endl;
		delete genesys;
		return 1;
	}

	SetInitialPattern(cellularAutomata, "0001000");

	std::cout << "Elementary cellular automaton through CellularAutomataComp - Rule 90" << std::endl;
	std::cout << "1D lattice, 7 cells, centered radius-1 neighborhood, closed boundary" << std::endl;
	std::cout << std::endl;

	const unsigned int steps = 6;
	std::cout << "t0: " << cellularAutomata->showCellularAutomata() << std::endl;
	for (unsigned int step = 1; step <= steps; ++step) {
		cellularAutomata->stepCellularAutomata();
		std::cout << "t" << step << ": " << cellularAutomata->showCellularAutomata() << std::endl;
	}

	delete genesys;
	return 0;
}
