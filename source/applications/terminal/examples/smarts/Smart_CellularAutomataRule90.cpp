/*
 * File:   Smart_CellularAutomataRule90.cpp
 *
 * Minimal terminal example for manually debugging a one-dimensional elementary
 * cellular automaton. Rule 90 is useful as a first case because the next state
 * is the XOR of the left and right neighbors.
 */

#include "Smart_CellularAutomataRule90.h"

#include "plugins/components/ModalModel/CellularAutomata/Boundary_Closed.h"
#include "plugins/components/ModalModel/CellularAutomata/CellularAutomata_Classic.h"
#include "plugins/components/ModalModel/CellularAutomata/Lattice.h"
#include "plugins/components/ModalModel/CellularAutomata/LocalRule_Elementary.h"
#include "plugins/components/ModalModel/CellularAutomata/Neighborhood_Center.h"
#include "plugins/components/ModalModel/CellularAutomata/State.h"
#include "plugins/components/ModalModel/CellularAutomata/StateSet_Enumerable.h"

#include <iostream>
#include <string>
#include <vector>

namespace {

std::string ShowLattice(Lattice& lattice) {
	std::string line;
	for (unsigned long cellNumber = 0; cellNumber < lattice.getCellsSize(); ++cellNumber) {
		line += std::to_string(lattice.getCell(static_cast<long>(cellNumber))->getCurrentState().getValue());
	}
	return line;
}

void SetInitialPattern(Lattice& lattice, const std::string& pattern) {
	for (unsigned long cellNumber = 0; cellNumber < pattern.size(); ++cellNumber) {
		const long value = pattern.at(cellNumber) == '1' ? 1 : 0;
		State state(value);
		lattice.setCellState(static_cast<long>(cellNumber), &state);
	}
}

}

Smart_CellularAutomataRule90::Smart_CellularAutomataRule90() {
}

int Smart_CellularAutomataRule90::main(int argc, char** argv) {
	CellularAutomata_Classic cellularAutomata;
	Lattice lattice(&cellularAutomata, nullptr, {7}, LatticeType::RETICULAR);
	State zero(0);
	State one(1);
	StateSet_Enumerable stateSet(&cellularAutomata, {&zero, &one});
	Boundary_Closed boundary(&lattice);
	Neighborhood_Center neighborhood(&cellularAutomata, 1, &boundary);
	LocalRule_Elementary rule90(&cellularAutomata, 90, &stateSet);

	cellularAutomata.setLattice(&lattice);
	cellularAutomata.setStateSet(&stateSet);
	cellularAutomata.setNeighborhood(&neighborhood);
	cellularAutomata.setLocalRule(&rule90);

	lattice.init();
	SetInitialPattern(lattice, "0001000");

	std::cout << "Elementary cellular automaton - Rule 90" << std::endl;
	std::cout << "1D lattice, 7 cells, centered radius-1 neighborhood, closed boundary" << std::endl;
	std::cout << std::endl;

	const unsigned int steps = 6;
	std::cout << "t0: " << ShowLattice(lattice) << std::endl;
	for (unsigned int step = 1; step <= steps; ++step) {
		cellularAutomata.step();
		std::cout << "t" << step << ": " << ShowLattice(lattice) << std::endl;
	}

	return 0;
}
