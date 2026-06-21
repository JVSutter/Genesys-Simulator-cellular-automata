/*
 * File:   Smart_CellularAutomataRule90.h
 *
 * Small terminal example for manually debugging a one-dimensional elementary
 * cellular automaton with Rule 90.
 */

#ifndef SMART_CELLULARAUTOMATARULE90_H
#define SMART_CELLULARAUTOMATARULE90_H

#include "../../../BaseGenesysTerminalApplication.h"

class Smart_CellularAutomataRule90 : public BaseGenesysTerminalApplication {
public:
	Smart_CellularAutomataRule90();
public:
	virtual int main(int argc, char** argv) override;
};

#endif /* SMART_CELLULARAUTOMATARULE90_H */
