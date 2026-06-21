#pragma once

#include "plugins/components/ModalModel/CellularAutomata/StateSet.h"

#include <string>

class StateSet_Double : public StateSet {
public:
	StateSet_Double(CellularAutomataBase* parentCellularAutomata)
		: StateSet(parentCellularAutomata) {
	}

	virtual bool contains(const State& state) const override {
		return true;
	}

	virtual std::string show() const override {
		return "double-compatible long values";
	}

	virtual std::string typeName() const override {
		return "double";
	}
};
