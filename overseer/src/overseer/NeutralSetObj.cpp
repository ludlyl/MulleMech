#include "NeutralSetObj.h"


namespace Overseer{
	/*
	****************************
	*** Public members start ***
	****************************
	*/

	NeutralImpl::NeutralImpl(const sc2::ObservationInterface* obs):m_NeutralUnits(){

		for(auto& u: obs->GetUnits(sc2::Unit::Alliance::Neutral)){

			if(isNeutral(u->unit_type.ToType())){
				sc2::Point2D pos(u->pos.x, u->pos.y);
				m_NeutralUnits.insert(std::make_pair(pos, u->unit_type.ToType()));
			}
		}
	}

	bool NeutralImpl::isMineral(sc2::Point2D& pos){

		switch(m_NeutralUnits.find(pos)->second){
			case sc2::UNIT_TYPEID::NEUTRAL_BATTLESTATIONMINERALFIELD: return true;
			case sc2::UNIT_TYPEID::NEUTRAL_BATTLESTATIONMINERALFIELD750: return true;
			case sc2::UNIT_TYPEID::NEUTRAL_LABMINERALFIELD: return true;
			case sc2::UNIT_TYPEID::NEUTRAL_LABMINERALFIELD750: return true;
			case sc2::UNIT_TYPEID::NEUTRAL_MINERALFIELD: return true;
			case sc2::UNIT_TYPEID::NEUTRAL_MINERALFIELD750: return true;
			case sc2::UNIT_TYPEID::NEUTRAL_PURIFIERMINERALFIELD: return true;
			case sc2::UNIT_TYPEID::NEUTRAL_PURIFIERMINERALFIELD750: return true;
			case sc2::UNIT_TYPEID::NEUTRAL_PURIFIERRICHMINERALFIELD: return true;
			case sc2::UNIT_TYPEID::NEUTRAL_PURIFIERRICHMINERALFIELD750: return true;
			case sc2::UNIT_TYPEID::NEUTRAL_RICHMINERALFIELD: return true;
			case sc2::UNIT_TYPEID::NEUTRAL_RICHMINERALFIELD750: return true;
			default: return false;
		}
	}

	bool NeutralImpl::isGas(sc2::Point2D& pos){

		switch(m_NeutralUnits.find(pos)->second){
			case sc2::UNIT_TYPEID::NEUTRAL_PROTOSSVESPENEGEYSER: return true;
			case sc2::UNIT_TYPEID::NEUTRAL_PURIFIERVESPENEGEYSER: return true;
			case sc2::UNIT_TYPEID::NEUTRAL_RICHVESPENEGEYSER: return true;
			case sc2::UNIT_TYPEID::NEUTRAL_SHAKURASVESPENEGEYSER: return true;
			case sc2::UNIT_TYPEID::NEUTRAL_SPACEPLATFORMGEYSER: return true;
			case sc2::UNIT_TYPEID::NEUTRAL_VESPENEGEYSER: return true;
			default: return false;
		}
	}

	bool NeutralImpl::isDestructible(sc2::Point2D& pos){

		switch(m_NeutralUnits.find(pos)->second){
			case sc2::UNIT_TYPEID::NEUTRAL_COLLAPSIBLEROCKTOWERDEBRIS: return true;
			case sc2::UNIT_TYPEID::NEUTRAL_COLLAPSIBLEROCKTOWERDIAGONAL: return true;
			case sc2::UNIT_TYPEID::NEUTRAL_COLLAPSIBLEROCKTOWERPUSHUNIT: return true;
			case sc2::UNIT_TYPEID::NEUTRAL_COLLAPSIBLETERRANTOWERDEBRIS: return true;
			case sc2::UNIT_TYPEID::NEUTRAL_COLLAPSIBLETERRANTOWERDIAGONAL: return true;
			case sc2::UNIT_TYPEID::NEUTRAL_COLLAPSIBLETERRANTOWERPUSHUNIT: return true;
			case sc2::UNIT_TYPEID::NEUTRAL_COLLAPSIBLETERRANTOWERPUSHUNITRAMPLEFT: return true;
			case sc2::UNIT_TYPEID::NEUTRAL_COLLAPSIBLETERRANTOWERPUSHUNITRAMPRIGHT: return true;
			case sc2::UNIT_TYPEID::NEUTRAL_DEBRISRAMPLEFT: return true;
			case sc2::UNIT_TYPEID::NEUTRAL_DEBRISRAMPRIGHT: return true;
			case sc2::UNIT_TYPEID::NEUTRAL_DESTRUCTIBLEDEBRIS6X6: return true;
			case sc2::UNIT_TYPEID::NEUTRAL_DESTRUCTIBLEDEBRISRAMPDIAGONALHUGEBLUR: return true;
			case sc2::UNIT_TYPEID::NEUTRAL_DESTRUCTIBLEDEBRISRAMPDIAGONALHUGEULBR: return true;
			case sc2::UNIT_TYPEID::NEUTRAL_DESTRUCTIBLEROCK6X6: return true;
			case sc2::UNIT_TYPEID::NEUTRAL_DESTRUCTIBLEROCKEX1DIAGONALHUGEBLUR: return true;
			case sc2::UNIT_TYPEID::NEUTRAL_UNBUILDABLEBRICKSDESTRUCTIBLE: return true;
			case sc2::UNIT_TYPEID::NEUTRAL_UNBUILDABLEPLATESDESTRUCTIBLE: return true;
			case sc2::UNIT_TYPEID::NEUTRAL_COLLAPSIBLETERRANTOWERRAMPLEFT: return true;
			case sc2::UNIT_TYPEID::NEUTRAL_COLLAPSIBLETERRANTOWERRAMPRIGHT: return true;
			default: return false;
		}
	}

	bool NeutralImpl::isNagaTower(sc2::Point2D& pos){

		if(m_NeutralUnits.find(pos)->second == sc2::UNIT_TYPEID::NEUTRAL_XELNAGATOWER){
			return true;
		}

		return false;
	}

	/*
	***************************
	*** Public members stop ***
	***************************

	***************************
	***************************
	***************************

	*****************************
	*** Priavte members start ***
	*****************************
	*/

	bool NeutralImpl::isNeutral(const sc2::UNIT_TYPEID& check){

		switch(check){
			case sc2::UNIT_TYPEID::NEUTRAL_BATTLESTATIONMINERALFIELD: return true;
			case sc2::UNIT_TYPEID::NEUTRAL_BATTLESTATIONMINERALFIELD750: return true;
			case sc2::UNIT_TYPEID::NEUTRAL_LABMINERALFIELD: return true;
			case sc2::UNIT_TYPEID::NEUTRAL_LABMINERALFIELD750: return true;
			case sc2::UNIT_TYPEID::NEUTRAL_MINERALFIELD: return true;
			case sc2::UNIT_TYPEID::NEUTRAL_MINERALFIELD750: return true;
			case sc2::UNIT_TYPEID::NEUTRAL_PURIFIERMINERALFIELD: return true;
			case sc2::UNIT_TYPEID::NEUTRAL_PURIFIERMINERALFIELD750: return true;
			case sc2::UNIT_TYPEID::NEUTRAL_PURIFIERRICHMINERALFIELD: return true;
			case sc2::UNIT_TYPEID::NEUTRAL_PURIFIERRICHMINERALFIELD750: return true;
			case sc2::UNIT_TYPEID::NEUTRAL_RICHMINERALFIELD: return true;
			case sc2::UNIT_TYPEID::NEUTRAL_RICHMINERALFIELD750: return true;
			case sc2::UNIT_TYPEID::NEUTRAL_PROTOSSVESPENEGEYSER: return true;
			case sc2::UNIT_TYPEID::NEUTRAL_PURIFIERVESPENEGEYSER: return true;
			case sc2::UNIT_TYPEID::NEUTRAL_RICHVESPENEGEYSER: return true;
			case sc2::UNIT_TYPEID::NEUTRAL_SHAKURASVESPENEGEYSER: return true;
			case sc2::UNIT_TYPEID::NEUTRAL_SPACEPLATFORMGEYSER:	return true;
			case sc2::UNIT_TYPEID::NEUTRAL_VESPENEGEYSER: return true;
			case sc2::UNIT_TYPEID::NEUTRAL_COLLAPSIBLEROCKTOWERDEBRIS: return true;
			case sc2::UNIT_TYPEID::NEUTRAL_COLLAPSIBLEROCKTOWERDIAGONAL: return true;
			case sc2::UNIT_TYPEID::NEUTRAL_COLLAPSIBLEROCKTOWERPUSHUNIT: return true;
			case sc2::UNIT_TYPEID::NEUTRAL_COLLAPSIBLETERRANTOWERDEBRIS: return true;
			case sc2::UNIT_TYPEID::NEUTRAL_COLLAPSIBLETERRANTOWERDIAGONAL: return true;
			case sc2::UNIT_TYPEID::NEUTRAL_COLLAPSIBLETERRANTOWERPUSHUNIT: return true;
			case sc2::UNIT_TYPEID::NEUTRAL_COLLAPSIBLETERRANTOWERPUSHUNITRAMPLEFT: return true;
			case sc2::UNIT_TYPEID::NEUTRAL_COLLAPSIBLETERRANTOWERPUSHUNITRAMPRIGHT: return true;
			case sc2::UNIT_TYPEID::NEUTRAL_DEBRISRAMPLEFT: return true;
			case sc2::UNIT_TYPEID::NEUTRAL_DEBRISRAMPRIGHT: return true;
			case sc2::UNIT_TYPEID::NEUTRAL_DESTRUCTIBLEDEBRIS6X6: return true;
			case sc2::UNIT_TYPEID::NEUTRAL_DESTRUCTIBLEDEBRISRAMPDIAGONALHUGEBLUR: return true;
			case sc2::UNIT_TYPEID::NEUTRAL_DESTRUCTIBLEDEBRISRAMPDIAGONALHUGEULBR: return true;
			case sc2::UNIT_TYPEID::NEUTRAL_DESTRUCTIBLEROCK6X6: return true;
			case sc2::UNIT_TYPEID::NEUTRAL_DESTRUCTIBLEROCKEX1DIAGONALHUGEBLUR: return true;
			case sc2::UNIT_TYPEID::NEUTRAL_UNBUILDABLEBRICKSDESTRUCTIBLE: return true;
			case sc2::UNIT_TYPEID::NEUTRAL_UNBUILDABLEPLATESDESTRUCTIBLE: return true;
			case sc2::UNIT_TYPEID::NEUTRAL_COLLAPSIBLETERRANTOWERRAMPLEFT: return true;
			case sc2::UNIT_TYPEID::NEUTRAL_COLLAPSIBLETERRANTOWERRAMPRIGHT: return true;
			case sc2::UNIT_TYPEID::NEUTRAL_XELNAGATOWER: return true;
			default: return false;
		}
	}
	/*
    ****************************
    *** Priavte members stop ***
    ****************************
    */
}
