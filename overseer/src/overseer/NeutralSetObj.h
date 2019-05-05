#ifndef _NEUTRAL_SET_OBJ_H_
#define _NEUTRAL_SET_OBJ_H_

#include "sc2api/sc2_api.h"
#include "Definitions.h"

namespace Overseer{
	typedef spatial::box_multimap<2, sc2::Point2D, sc2::UNIT_TYPEID, spatial::accessor_less<accessors::point2d_accessor, sc2::Point2D>> NeutralContainer;

	/**
	* \struct NeutralImpl NeutralImpl.h "src/NeutralImpl.h"
	* \brief Used to find neutral objects on the map.
	*/
	struct NeutralImpl{
		/**
		* \brief Constructor sets all neutral positions to a map.
		*
		* \param obs used to discover neutrals on the map.
		*/
		NeutralImpl(const sc2::ObservationInterface* obs);

		/**
		* \brief Check if a point is a mineral or not.
		*
		* \param pos the point to check.
		* \return true if point is mineral, otherwise false.
		*/
		bool isMineral(sc2::Point2D& pos);

		/**
		* \brief Check if a point is a geyser or not.
		*
		* \param  pos the point to check.
		* \return true if it is geyser, otherwise false.
		*/
		bool isGas(sc2::Point2D& pos);

		/**
		* \brief Check if a point is a destructable object or not.
		*
		* \param  pos the point to check.
		* \return true if it is destructable object, otherwise false.
		*/
		bool isDestructible(sc2::Point2D& pos);

		/**
		* \brief Check if a point is a Xelnaga tower or not.
		*
		* \param  pos the point to check.
		* \return true if it is Xelnaga tower, otherwise false.
		*/
		bool isNagaTower(sc2::Point2D& pos);

		private:
			// check if UNIT_TYPEID is neutral of intresst
			bool isNeutral(const sc2::UNIT_TYPEID& check);

			NeutralContainer m_NeutralUnits;
	};
}

#endif /*_NEUTRAL_SET_OBJ_H_*/
