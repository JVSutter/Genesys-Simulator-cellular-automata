/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   CelularAutomata.cpp
 * Author: rlcancian
 * 
 * Created on 03 de Junho de 2019, 15:14
 */

#include "plugins/components/ModalModel/CellularAutomataComp.h"
#include "kernel/simulator/Model.h"
#include "plugins/components/ModalModel/CellularAutomata/Boundary_Adiabatic.h"
#include "plugins/components/ModalModel/CellularAutomata/Boundary_Closed.h"
#include "plugins/components/ModalModel/CellularAutomata/Boundary_Fixed.h"
#include "plugins/components/ModalModel/CellularAutomata/Boundary_Reflexive.h"
#include "plugins/components/ModalModel/CellularAutomata/CellularAutomata_Classic.h"
#include "plugins/components/ModalModel/CellularAutomata/CellularAutomata_1DTimed.h"
#include "plugins/components/ModalModel/CellularAutomata/LocalRule_Elementary.h"
#include "plugins/components/ModalModel/CellularAutomata/LocalRule_GameOfLife.h"
#include "plugins/components/ModalModel/CellularAutomata/LocalRule_Growty.h"
#include "plugins/components/ModalModel/CellularAutomata/Neighborhood_Center.h"
#include "plugins/components/ModalModel/CellularAutomata/Neighborhood_Moore.h"
#include "plugins/components/ModalModel/CellularAutomata/Neighborhood_VonNeumann.h"
#include "plugins/components/ModalModel/CellularAutomata/State.h"
#include "plugins/components/ModalModel/CellularAutomata/StateSet_Enumerable.h"

#include <sstream>

#ifdef PLUGINCONNECT_DYNAMIC

extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &CellularAutomataComp::GetPluginInformation;
}
#endif

ModelDataDefinition* CellularAutomataComp::NewInstance(Model* model, std::string name) {
	return new CellularAutomataComp(model, name);
}

CellularAutomataComp::CellularAutomataComp(Model* model, std::string name) : ModelComponent(model, Util::TypeOf<CellularAutomataComp>(), name) {
}

std::string CellularAutomataComp::show() {
	return ModelComponent::show() + "";
}


ModelComponent* CellularAutomataComp::LoadInstance(Model* model, PersistenceRecord *fields) {
	CellularAutomataComp* newComponent = new CellularAutomataComp(model);
	try {
		newComponent->_loadInstance(fields);
	} catch (const std::exception& e) {

	}
	return newComponent;
}

void CellularAutomataComp::_onDispatchEvent(Entity* entity, unsigned int inputPortNumber) {
	_cellularAutomata->step();
	_parentModel->sendEntityToComponent(entity, this->getConnectionManager()->getFrontConnection());
}

bool CellularAutomataComp::_loadInstance(PersistenceRecord *fields) {
	bool res = ModelComponent::_loadInstance(fields);
	if (res) {
		// @TODO: not implemented yet
	}
	return res;
}

void CellularAutomataComp::_saveInstance(PersistenceRecord *fields, bool saveDefaultValues) {
	ModelComponent::_saveInstance(fields, saveDefaultValues);
	// @TODO: not implemented yet
}

bool CellularAutomataComp::_check(std::string* errorMessage) {
	if (_cellularAutomata == nullptr) {
		if (errorMessage != nullptr)
			*errorMessage += "Cellular automata type was not configured. ";
		return false;
	}
	if (_lattice == nullptr) {
		if (errorMessage != nullptr)
			*errorMessage += "Lattice type was not configured. ";
		return false;
	}
	if (_localRule == nullptr) {
		if (errorMessage != nullptr)
			*errorMessage += "Local rule type was not configured. ";
		return false;
	}
	if (_neighboorhood == nullptr) {
		if (errorMessage != nullptr)
			*errorMessage += "Neighborhood type was not configured. ";
		return false;
	}
	if (_boundary == nullptr) {
		if (errorMessage != nullptr)
			*errorMessage += "Boundary type was not configured. ";
		return false;
	}
	if (_stateSet == nullptr) {
		if (errorMessage != nullptr)
			*errorMessage += "State set type was not configured. ";
		return false;
	}
	_cellularAutomata->setLattice(_lattice);
	_cellularAutomata->setLocalRule(_localRule);
	_cellularAutomata->setNeighborhood(_neighboorhood);
	_cellularAutomata->setStateSet(_stateSet);
	_localRule->setStateSet(_stateSet);
	_neighboorhood->setBoundary(_boundary);
	_boundary->setLattice(_lattice);
	_boundary->setNeighborhood(_neighboorhood);
	return true;
}

void CellularAutomataComp::_initBetweenReplications() {
	_cellularAutomata->init();
}

bool CellularAutomataComp::initializeCellularAutomata(std::string* errorMessage) {
	std::string localErrorMessage;
	std::string* message = errorMessage != nullptr ? errorMessage : &localErrorMessage;
	if (!_check(message))
		return false;
	return _cellularAutomata->init();
}

void CellularAutomataComp::stepCellularAutomata() {
	if (_cellularAutomata != nullptr)
		_cellularAutomata->step();
}

bool CellularAutomataComp::setCellState(long cellNumber, long value) {
	if (_lattice == nullptr)
		return false;
	State state(value);
	return _lattice->setCellState(cellNumber, &state);
}

std::string CellularAutomataComp::showCellularAutomata() const {
	if (_lattice == nullptr)
		return "";
	std::ostringstream output;
	for (unsigned long cellNumber = 0; cellNumber < _lattice->getCellsSize(); ++cellNumber) {
		output << _lattice->getCell(static_cast<long>(cellNumber))->getCurrentState().getValue();
	}
	return output.str();
}

void CellularAutomataComp::setElementaryRuleNumber(uint8_t ruleNumber) {
	_elementaryRuleNumber = ruleNumber;
	if (_localRuleType == LocalRuleType::ELEMENTAR_CA && _localRule != nullptr) {
		if (auto* elementaryRule = dynamic_cast<LocalRule_Elementary*>(_localRule))
			elementaryRule->setRuleNumber(ruleNumber);
	}
}

LocalRule *CellularAutomataComp::getlocalRule() const
{
	return _localRule;
}

CellularAutomataComp::LocalRuleType CellularAutomataComp::getlocalRuleType() const
{
	return _localRuleType;
}

void CellularAutomataComp::setLocalRuleType(CellularAutomataComp::LocalRuleType newLocalRuleType)
{
	_localRuleType = newLocalRuleType;
	_ensureCellularAutomata();
	if (_localRule != nullptr)
		delete _localRule;
	_localRule = nullptr;
	if (_localRuleType == LocalRuleType::ELEMENTAR_CA) {
		_localRule = new LocalRule_Elementary(_cellularAutomata, _elementaryRuleNumber);
	} else if (_localRuleType == LocalRuleType::GAME_OF_LIFE) {
		_localRule = new LocalRule_GameOfLife(_cellularAutomata);
	} else if (_localRuleType == LocalRuleType::BIASED_COMPETITION) {
		_localRule = new LocalRule_Growty(_cellularAutomata);
	}
}

void CellularAutomataComp::setStateSetType(CellularAutomataComp::StateSetType newStateSetType)
{
	_stateSetType = newStateSetType;
	_ensureCellularAutomata();
	if (_stateSet != nullptr)
		delete _stateSet;
	_stateSet = nullptr;
	if (_stateSetType == StateSetType::ENUMERATED)
		_stateSet = new StateSet_Enumerable(_cellularAutomata, {new State(0), new State(1)});
	else
		_stateSet = new StateSet(_cellularAutomata);
}


CellularAutomataComp::StateSetType CellularAutomataComp::getStateSetType() const
{
	return _stateSetType;
}

//CellularAutomataBase *CellularAutomataComp::getcellularAutomata() const{
//	return _cellularAutomata;
//}

Lattice *CellularAutomataComp::getlattice() const
{
	return _lattice;
}

Neighborhood *CellularAutomataComp::getNeighboorhood() const
{
	return _neighboorhood;
}

//BoundaryCondition *CellularAutomataComp::getBoundary() const{
//	return _boundary;
//}

StateSet *CellularAutomataComp::getStateSet() const
{
	return _stateSet;
}

CellularAutomataComp::CellularAutomataType CellularAutomataComp::getCellularAutomataType() const
{
	return _cellularAutomataType;
}

void CellularAutomataComp::setCellularAutomataType(CellularAutomataComp::CellularAutomataType newCellularAutomataType)
{
	_cellularAutomataType = newCellularAutomataType;
	if (_cellularAutomata != nullptr)
		delete _cellularAutomata;
	_cellularAutomata = nullptr;
	if (_cellularAutomataType == CellularAutomataType::CLASSIC)
		_cellularAutomata = new CellularAutomata_Classic();
	else if (_cellularAutomataType == CellularAutomataType::TIMED_1D)
		_cellularAutomata = new CellularAutomata_1DTimed();
}

CellularAutomataComp::LatticeType CellularAutomataComp::getLatticeType() const
{
	return _latticeType;
}

void CellularAutomataComp::setLatticeType(CellularAutomataComp::LatticeType newLatticeStructure)
{
	_latticeType = newLatticeStructure;
	_ensureCellularAutomata();
	if (_lattice == nullptr)
		_lattice = new Lattice(_cellularAutomata);
}

CellularAutomataComp::NeighboorhoodType CellularAutomataComp::getNeighboorhoodType() const
{
	return _neighboorhoodType;
}

void CellularAutomataComp::setNeighboorhoodType(CellularAutomataComp::NeighboorhoodType newNeighboorhood)
{
	_neighboorhoodType = newNeighboorhood;
	_ensureCellularAutomata();
	if (_neighboorhood != nullptr)
		delete _neighboorhood;
	_neighboorhood = nullptr;
	if (_neighboorhoodType == NeighboorhoodType::CENTERED)
		_neighboorhood = new Neighborhood_Center(_cellularAutomata);
	else if (_neighboorhoodType == NeighboorhoodType::MOORE)
		_neighboorhood = new Neighborhood_Moore(_cellularAutomata);
	else if (_neighboorhoodType == NeighboorhoodType::VONNEUMANN)
		_neighboorhood = new Neighborhood_VonNeumann(_cellularAutomata);
}

CellularAutomataComp::BoundaryType CellularAutomataComp::geBoundaryType() const
{
	return _boundaryType;
}

void CellularAutomataComp::setBoundaryType(CellularAutomataComp::BoundaryType newBoundary)
{
	_boundaryType = newBoundary;
	if (_boundary != nullptr)
		delete _boundary;
	_boundary = nullptr;
	if (_boundaryType == BoundaryType::CLOSED)
		_boundary = new Boundary_Closed();
	else if (_boundaryType == BoundaryType::FIXED)
		_boundary = new Boundary_Fixed();
	else if (_boundaryType == BoundaryType::REFLEXIVE)
		_boundary = new Boundary_Reflexive();
	else if (_boundaryType == BoundaryType::ADIABATIC)
		_boundary = new Boundary_Adiabatic();
}

void CellularAutomataComp::_ensureCellularAutomata() {
	if (_cellularAutomata == nullptr)
		setCellularAutomataType(_cellularAutomataType);
}

PluginInformation* CellularAutomataComp::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<CellularAutomataComp>(), &CellularAutomataComp::LoadInstance, &CellularAutomataComp::NewInstance);
	info->setCategory("ModalModel");
	info->setDescriptionHelp("//@TODO");
	return info;
}

// void CellularAutomataComp::_createInternalStatisticReporters() { }

// void CellularAutomataComp::_createEditableDataDefinitions() { }

// void CellularAutomataComp::_createAttachedAttributes() { }
