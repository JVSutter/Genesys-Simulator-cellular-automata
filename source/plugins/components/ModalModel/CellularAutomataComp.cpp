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
#include "../../../kernel/simulator/model/Model.h"
#include "plugins/components/ModalModel/CellularAutomata/Boundary_Closed.h"
#include "plugins/components/ModalModel/CellularAutomata/Boundary_Fixed.h"
#include "plugins/components/ModalModel/CellularAutomata/CellularAutomata_Classic.h"
#include "plugins/components/ModalModel/CellularAutomata/CellularAutomata_1DTimed.h"
#include "plugins/components/ModalModel/CellularAutomata/LocalRule_Elementary.h"
#include "plugins/components/ModalModel/CellularAutomata/LocalRule_GameOfLife.h"
#include "plugins/components/ModalModel/CellularAutomata/LocalRule_Growty.h"
#include "plugins/components/ModalModel/CellularAutomata/Neighborhood_Center.h"
#include "plugins/components/ModalModel/CellularAutomata/Neighborhood_Moore.h"
#include "plugins/components/ModalModel/CellularAutomata/Neighborhood_VonNeumann.h"
#include "plugins/components/ModalModel/CellularAutomata/LocalRule_UserDefined.h"
#include "plugins/data/ExternalIntegration/CppCompiler.h"

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

CellularAutomataComp::~CellularAutomataComp() {
	// These sub-objects are plain classes (not ModelDataDefinition), so this component owns them and
	// must free them. Deleting _localRule first runs the LocalRule_UserDefined destructor, which
	// unloads its dynamic library. _ruleCompiler is a ModelDataDefinition owned by the model, so it
	// is intentionally NOT deleted here (the model frees it).
	delete _localRule;
	delete _cellularAutomata;
	delete _lattice;
	delete _neighboorhood;
	delete _boundary;
	delete _stateSet;
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
		// Recreate the sub-objects from the persisted type enums. The cellular-automata type must be
		// restored first because the lattice/neighborhood/boundary/state-set/rule reference it.
		setCellularAutomataType(static_cast<CellularAutomataType>(
			fields->loadField("cellularAutomataType", static_cast<int>(DEFAULT.cellularAutomataType))));
		setLatticeType(static_cast<LatticeType>(
			fields->loadField("latticeType", static_cast<int>(DEFAULT.latticeType))));
		setNeighboorhoodType(static_cast<NeighboorhoodType>(
			fields->loadField("neighborhoodType", static_cast<int>(DEFAULT.neighboorhoodType))));
		setBoundaryType(static_cast<BoundaryType>(
			fields->loadField("boundaryType", static_cast<int>(DEFAULT.boundaryType))));
		setStateSetType(static_cast<StateSetType>(
			fields->loadField("stateSetType", static_cast<int>(DEFAULT.stateSetType))));
		// Load the user source before the rule type, so a USERDEFINED rule has its source available.
		_userDefinedRuleSource = fields->loadField("userDefinedRuleSource", DEFAULT.userDefinedRuleSource);
		setLocalRuleType(static_cast<LocalRuleType>(
			fields->loadField("localRuleType", static_cast<int>(DEFAULT.localRuleType))));
	}
	return res;
}

void CellularAutomataComp::_saveInstance(PersistenceRecord *fields, bool saveDefaultValues) {
	ModelComponent::_saveInstance(fields, saveDefaultValues);
	fields->saveField("cellularAutomataType", static_cast<int>(_cellularAutomataType),
		static_cast<int>(DEFAULT.cellularAutomataType), saveDefaultValues);
	fields->saveField("latticeType", static_cast<int>(_latticeType),
		static_cast<int>(DEFAULT.latticeType), saveDefaultValues);
	fields->saveField("neighborhoodType", static_cast<int>(_neighboorhoodType),
		static_cast<int>(DEFAULT.neighboorhoodType), saveDefaultValues);
	fields->saveField("boundaryType", static_cast<int>(_boundaryType),
		static_cast<int>(DEFAULT.boundaryType), saveDefaultValues);
	fields->saveField("stateSetType", static_cast<int>(_stateSetType),
		static_cast<int>(DEFAULT.stateSetType), saveDefaultValues);
	fields->saveField("localRuleType", static_cast<int>(_localRuleType),
		static_cast<int>(DEFAULT.localRuleType), saveDefaultValues);
	fields->saveField("userDefinedRuleSource", _userDefinedRuleSource,
		DEFAULT.userDefinedRuleSource, saveDefaultValues);
}

bool CellularAutomataComp::_buildUserDefinedRule(std::string* errorMessage) {
	if (_userDefinedRuleSource.empty()) {
		*errorMessage += "USERDEFINED local rule requires source code (use setUserDefinedRuleSource). ";
		return false;
	}
	if (_cellularAutomata == nullptr) {
		*errorMessage += "USERDEFINED local rule needs a cellular automata (set its type first). ";
		return false;
	}
	if (_ruleCompiler == nullptr) {
		_ruleCompiler = new CppCompiler(_parentModel, getName() + ".LocalRuleCompiler");
	}
	_ruleCompiler->setOutputDir(".temp/");
	_ruleCompiler->setTempDir(".temp/");
	_ruleCompiler->setFlagsGeneral("-w -std=c++14");
	if (_localRule != nullptr) {
		delete _localRule;
		_localRule = nullptr;
	}
	LocalRule_UserDefined* userRule = new LocalRule_UserDefined(_cellularAutomata, _ruleCompiler, _stateSet);
	std::string buildError;
	if (!userRule->build(_userDefinedRuleSource, buildError)) {
		*errorMessage += "USERDEFINED local rule failed to compile/load: " + buildError + " ";
		delete userRule;
		// userRule registered itself in the automaton (LocalRule ctor); clear the now-dangling pointer.
		_cellularAutomata->setLocalRule(nullptr);
		return false;
	}
	_localRule = userRule;
	// Don't leave the throwaway per-build temp paths in the compiler's persistent fields.
	_ruleCompiler->setSourceFilename("");
	_ruleCompiler->setOutputFilename("");
	return true;
}

bool CellularAutomataComp::_check(std::string* errorMessage) {
	// Semantic checks: every structural sub-object must have been chosen, and the (possibly
	// user-defined) local rule must be available before the connections are wired.
	if (_cellularAutomata == nullptr) {
		*errorMessage += "Cellular automata type is not set or is not supported. ";
		return false;
	}
	if (_lattice == nullptr) {
		*errorMessage += "Lattice type is not set. ";
		return false;
	}
	if (_neighboorhood == nullptr) {
		*errorMessage += "Neighborhood type is not set. ";
		return false;
	}
	if (_stateSet == nullptr) {
		*errorMessage += "State set type is not set. ";
		return false;
	}
	if (_boundary == nullptr) {
		*errorMessage += "Boundary type is not set. ";
		return false;
	}
	if (_localRuleType == LocalRuleType::USERDEFINED) {
		if (!_buildUserDefinedRule(errorMessage)) {
			return false;
		}
	}
	if (_localRule == nullptr) {
		*errorMessage += "Local rule is not set (or its type is not supported yet). ";
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
	if (_localRule != nullptr) {
		delete _localRule;
		_localRule = nullptr; // avoid a dangling/double-freed pointer for types that build no rule here
		if (_cellularAutomata != nullptr) {
			_cellularAutomata->setLocalRule(nullptr);
		}
	}
	if (_localRuleType == LocalRuleType::ELEMENTAR_CA) {
		_localRule = new LocalRule_Elementary(_cellularAutomata, 30);
	} else if (_localRuleType == LocalRuleType::GAME_OF_LIFE) {
		_localRule = new LocalRule_GameOfLife(_cellularAutomata);
	} else if (_localRuleType == LocalRuleType::BIASED_COMPETITION) {
		_localRule = new LocalRule_Growty(_cellularAutomata);
	}
	// USERDEFINED is compiled and loaded lazily in _check(), where the cellular automata, state set
	// and user source are all available (see _buildUserDefinedRule).
}

void CellularAutomataComp::setUserDefinedRuleSource(const std::string& userDefinedRuleSource) {
	_userDefinedRuleSource = userDefinedRuleSource;
}

std::string CellularAutomataComp::getUserDefinedRuleSource() const {
	return _userDefinedRuleSource;
}

void CellularAutomataComp::setStateSetType(CellularAutomataComp::StateSetType newStateSetType)
{
	_stateSetType = newStateSetType;
	if (_stateSet == nullptr)
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
	if (_cellularAutomata != nullptr) {
		delete _cellularAutomata; // was an explicit destructor call (~CellularAutomataBase), which left a dangling pointer
		_cellularAutomata = nullptr;
	}
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
	if (_neighboorhood != nullptr)
		delete _neighboorhood;
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
	if (_boundaryType == BoundaryType::CLOSED)
		_boundary = new Boundary_Closed();
	else if (_boundaryType == BoundaryType::FIXED)
		_boundary = new Boundary_Fixed();
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
