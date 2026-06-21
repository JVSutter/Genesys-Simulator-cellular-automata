/*
 * File:   Smart_CellularAutomataGameOfLife.h
 *
 * Terminal example for manually debugging Game of Life through CellularAutomataComp.
 */

#ifndef SMART_CELLULARAUTOMATAGAMEOFLIFE_H
#define SMART_CELLULARAUTOMATAGAMEOFLIFE_H

#include "../../../BaseGenesysTerminalApplication.h"

class Smart_CellularAutomataGameOfLife : public BaseGenesysTerminalApplication {
public:
	Smart_CellularAutomataGameOfLife();
public:
	virtual int main(int argc, char** argv) override;
};

#endif /* SMART_CELLULARAUTOMATAGAMEOFLIFE_H */
