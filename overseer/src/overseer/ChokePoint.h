#ifndef _OVERSEER_CHOKEPOINT_H_
#define _OVERSEER_CHOKEPOINT_H_

#include "Region.h"
#include "Graph.h"
#include <tuple>
#include <utility>

namespace Overseer{
    typedef std::tuple<size_t, size_t, size_t> ChokePointId;

    /**
    * \class ChokePoint ChokePoint.h "ChokePoint.h"
    * \brief Class that is used as a chokepoint container with size and positioning on the map.
    */

    class ChokePoint {
        public:

            /**
            * \brief class Constructor
            *
            * \param region1 is a adjecent region to region2
            * \param region2 is a adjecent region to region1
            * \param The position of the chokepoint between region1 and region2 starts at zero.
            * \param tilePositions is the "limits" between region1 and region2
            */
            ChokePoint(const Region* region1, const Region* region2, size_t cp_id, std::vector<TilePosition> tilePositions);

            /**
            * \brief Gets the regions where a chokepoint exists between them.
            *
            * \return pair containing adjecent region pointers with chokepoint between them
            */
            std::pair<const Region*, const Region*> getRegions() const;

            /**
            * \brief Get the size of the found chokepoint
            *
            * \return The size of the chokepoint
            */
            size_t size() const;

            /**
            * \brief Gets the mid point of the chokepoint
            *
            * \return The mid point of the chokepoint
            */
            sc2::Point2D getMidPoint() const;

            /**
            * \brief Get the points within the chokepoint.
            *
            * \return vector containing chokepoint positions
            */
            std::vector<sc2::Point2D> getPoints() const;

            /**
            * \brief Get the tile positions inside the chokePoint
            *
            * \return vector containing tile positions.
            */
            std::vector<TilePosition> getTilePositions() const;

            /**
            * \brief Get the id of the chokepoint
            *
            * \return chokepoint id.
            */
            ChokePointId getId() const;

            /**
            * \brief How two chokepoints will be check if equal.
            */
            bool operator==(const ChokePoint& rhs) const;

            /**
            * \brief check if this chokepoint share a region with chokepoint cp.
            *
            * \param cp another chokepoint
            * \return true if they share a region, otherwise false.
            */
            bool adjacent(const ChokePoint& cp) const;

            /**
            * \brief Gets the chokepoint id as a string.
            *
            * \return string version of chokepoint id.
            */
            std::string getStringId() const;

            /**
            * \brief Gets the chokepoints position between two regions (INTERNAL USE ONLY).
            */
            size_t getChokePointId() const;

        private:
            size_t m_id;
            std::pair<const Region *, const Region *> m_regions;
            std::vector<UnitPosition> m_neutralUnitPositions;
            std::vector<TilePosition> m_tilePositions;
            TilePosition m_center;
    };
}

#endif /* _OVERSEER_CHOKEPOINT_H_ */
