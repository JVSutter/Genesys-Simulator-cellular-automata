#pragma once

#include "plugins/components/ModalModel/CellularAutomata/StateSet.h"

#include <string>

class StateSet_Bit : public StateSet {
public:
	StateSet_Bit(CellularAutomataBase* parentCellularAutomata)
		: StateSet(parentCellularAutomata) {
	}

	virtual bool contains(const State& state) const override {
		return state.getValue() == 0 || state.getValue() == 1;
	}

	virtual std::string show() const override {
		return "{0,1}";
	}

	virtual std::string typeName() const override {
		return "bit";
	}
};
