/*
 * File:   Smart_CellularAutomataNDLattice.h
 *
 * Terminal example for manually debugging N-dimensional reticular lattices.
 */

#ifndef SMART_CELLULARAUTOMATANDLATTICE_H
#define SMART_CELLULARAUTOMATANDLATTICE_H

#include "../../../BaseGenesysTerminalApplication.h"

class Smart_CellularAutomataNDLattice : public BaseGenesysTerminalApplication {
public:
	Smart_CellularAutomataNDLattice();
public:
	virtual int main(int argc, char** argv) override;
};

#endif /* SMART_CELLULARAUTOMATANDLATTICE_H */
