/*
 * File:   Smart_CellularAutomataSemanticChecks.cpp
 *
 * Minimal terminal example that exercises CellularAutomataComp semantic checks.
 */

#include "Smart_CellularAutomataSemanticChecks.h"

#include "kernel/simulator/Simulator.h"
#include "plugins/components/ModalModel/CellularAutomataComp.h"

#include <functional>
#include <iostream>
#include <string>

namespace {

using ConfigureCase = std::function<void(CellularAutomataComp*)>;

void ConfigureRule90(CellularAutomataComp* cellularAutomata) {
	cellularAutomata->setCellularAutomataType(CellularAutomataComp::CellularAutomataType::CLASSIC);
	cellularAutomata->setLatticeType(CellularAutomataComp::LatticeType::RETICULAR);
	cellularAutomata->getlattice()->setDimensions({7});
	cellularAutomata->setNeighboorhoodType(CellularAutomataComp::NeighboorhoodType::CENTERED);
	cellularAutomata->getNeighboorhood()->setRadius(1);
	cellularAutomata->setBoundaryType(CellularAutomataComp::BoundaryType::FIXED);
	cellularAutomata->setStateSetType(CellularAutomataComp::StateSetType::ENUMERATED);
	cellularAutomata->setElementaryRuleNumber(90);
	cellularAutomata->setLocalRuleType(CellularAutomataComp::LocalRuleType::ELEMENTAR_CA);
}

void ConfigureGameOfLife(CellularAutomataComp* cellularAutomata) {
	cellularAutomata->setCellularAutomataType(CellularAutomataComp::CellularAutomataType::CLASSIC);
	cellularAutomata->setLatticeType(CellularAutomataComp::LatticeType::RETICULAR);
	cellularAutomata->getlattice()->setDimensions({5, 5});
	cellularAutomata->setNeighboorhoodType(CellularAutomataComp::NeighboorhoodType::MOORE);
	cellularAutomata->getNeighboorhood()->setRadius(1);
	cellularAutomata->setBoundaryType(CellularAutomataComp::BoundaryType::FIXED);
	cellularAutomata->setStateSetType(CellularAutomataComp::StateSetType::ENUMERATED);
	cellularAutomata->setLocalRuleType(CellularAutomataComp::LocalRuleType::GAME_OF_LIFE);
}

bool RunCase(Model* model, const std::string& name, bool expectedResult, ConfigureCase configureCase) {
	CellularAutomataComp* cellularAutomata = new CellularAutomataComp(model);
	configureCase(cellularAutomata);

	std::string errorMessage;
	const bool result = cellularAutomata->initializeCellularAutomata(&errorMessage);
	const bool passed = result == expectedResult;

	std::cout << (passed ? "PASS" : "FAIL") << " - " << name;
	if (!errorMessage.empty())
		std::cout << " - " << errorMessage;
	std::cout << std::endl;
	return passed;
}

}

Smart_CellularAutomataSemanticChecks::Smart_CellularAutomataSemanticChecks() {
}

int Smart_CellularAutomataSemanticChecks::main(int argc, char** argv) {
	Simulator* genesys = new Simulator();
	genesys->getTraceManager()->setTraceLevel(TraceManager::Level::L0_noTraces);
	setDefaultTraceHandlers(genesys->getTraceManager());

	Model* model = genesys->getModelManager()->newModel();

	std::cout << "CellularAutomataComp semantic checks" << std::endl;
	std::cout << std::endl;

	bool allPassed = true;
	allPassed &= RunCase(model, "Game of Life with Von Neumann neighborhood is rejected", false, [](CellularAutomataComp* cellularAutomata) {
		ConfigureGameOfLife(cellularAutomata);
		cellularAutomata->setNeighboorhoodType(CellularAutomataComp::NeighboorhoodType::VONNEUMANN);
		cellularAutomata->getNeighboorhood()->setRadius(1);
	});
	allPassed &= RunCase(model, "Rule 90 in 2D lattice is rejected", false, [](CellularAutomataComp* cellularAutomata) {
		ConfigureRule90(cellularAutomata);
		cellularAutomata->getlattice()->setDimensions({3, 3});
	});
	allPassed &= RunCase(model, "Lattice without dimensions is rejected", false, [](CellularAutomataComp* cellularAutomata) {
		ConfigureRule90(cellularAutomata);
		cellularAutomata->getlattice()->setDimensions({});
	});
	allPassed &= RunCase(model, "Hexagonal lattice is rejected as not implemented", false, [](CellularAutomataComp* cellularAutomata) {
		ConfigureRule90(cellularAutomata);
		cellularAutomata->setLatticeType(CellularAutomataComp::LatticeType::HEXAGONAL);
	});
	allPassed &= RunCase(model, "Valid Game of Life configuration is accepted", true, [](CellularAutomataComp* cellularAutomata) {
		ConfigureGameOfLife(cellularAutomata);
	});

	delete genesys;
	return allPassed ? 0 : 1;
}
