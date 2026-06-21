/* 
 * File:   Neighborhood_Moore.h
 * Author: cancian
 *
 * Created on 6 de abril de 2023, 14:27
 */

#pragma once

#include <cstdlib>
#include <utility>
#include <vector>

#include "plugins/components/ModalModel/CellularAutomata/Lattice.h"
#include "plugins/components/ModalModel/CellularAutomata/Neighborhood.h"
#include "plugins/components/ModalModel/CellularAutomata/Cell.h"

class Neighborhood_Moore : public Neighborhood {
public:
    Neighborhood_Moore(CellularAutomataBase* parentCellularAutomata, unsigned short radius = 1, BoundaryCondition* boundary = nullptr)
        :Neighborhood(parentCellularAutomata, radius, boundary) { this->name = "Moore"; }
    Neighborhood_Moore(const Neighborhood_Moore& orig):Neighborhood(orig) {    }
    virtual ~Neighborhood_Moore() = default;
public:
    virtual std::string show() override {
        return "Moore";
    }
    virtual std::vector<Cell*> getNeighbors(Cell* cell)override {
        std::vector<Cell*> neighbors;
		std::vector<int> cellPosition = cell->getPosition();
		std::vector<int> offsets(cellPosition.size(), 0);
		_generateNeighbors(cellPosition, offsets, 0, neighbors);
        if (includeCellItself)
            neighbors.emplace_back(cell);
        return neighbors;
    }
private:
	void _generateNeighbors(const std::vector<int>& cellPosition, std::vector<int>& offsets, unsigned short dimension, std::vector<Cell*>& neighbors) {
		if (dimension == offsets.size()) {
			std::vector<std::pair<unsigned short, int>> dimensionChanges;
			for (unsigned short dim = 0; dim < offsets.size(); ++dim) {
				if (offsets.at(dim) != 0)
					dimensionChanges.emplace_back(dim, offsets.at(dim));
			}
			if (!dimensionChanges.empty())
				neighbors.emplace_back(getNeighborCell(cellPosition, dimensionChanges));
			return;
		}

		for (int offset = -static_cast<int>(radius); offset <= static_cast<int>(radius); ++offset) {
			offsets.at(dimension) = offset;
			_generateNeighbors(cellPosition, offsets, dimension + 1, neighbors);
		}
	}
};
