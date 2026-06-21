/* 
 * File:   Neighborhood_VonNeumann.h
 * Author: cancian
 *
 * Created on 6 de abril de 2023, 14:27
 */

#pragma once

#include <cstdlib>
#include <string>
#include <utility>
#include <vector>

#include "plugins/components/ModalModel/CellularAutomata/Neighborhood.h"
#include "plugins/components/ModalModel/CellularAutomata/Lattice.h"

class Neighborhood_VonNeumann : public Neighborhood {
public:

    Neighborhood_VonNeumann(CellularAutomataBase* parentCellularAutomata, unsigned short radius = 1, BoundaryCondition* boundary = nullptr)
    :Neighborhood(parentCellularAutomata, radius, boundary) {
         this->name = "Von Neumann";
    }

    Neighborhood_VonNeumann(const Neighborhood_VonNeumann& orig)
    :Neighborhood(orig) {
    }
    virtual ~Neighborhood_VonNeumann() = default;
public:

    virtual std::string show() override {
        return "Von Neumann";
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
			unsigned int manhattanDistance = 0;
			std::vector<std::pair<unsigned short, int>> dimensionChanges;
			for (unsigned short dim = 0; dim < offsets.size(); ++dim) {
				manhattanDistance += static_cast<unsigned int>(std::abs(offsets.at(dim)));
				if (offsets.at(dim) != 0)
					dimensionChanges.emplace_back(dim, offsets.at(dim));
			}
			if (!dimensionChanges.empty() && manhattanDistance <= radius)
				neighbors.emplace_back(getNeighborCell(cellPosition, dimensionChanges));
			return;
		}

		for (int offset = -static_cast<int>(radius); offset <= static_cast<int>(radius); ++offset) {
			offsets.at(dimension) = offset;
			_generateNeighbors(cellPosition, offsets, dimension + 1, neighbors);
		}
	}
};
