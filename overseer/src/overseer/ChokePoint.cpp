#include "ChokePoint.h"

namespace Overseer{
	/*
	****************************
	*** Public members start ***
	****************************
	*/

	ChokePoint::ChokePoint(const Region* region1, const Region* region2, size_t cp_id, std::vector<TilePosition> tilePositions){
	    m_regions.first = region1;
	    m_regions.second = region2;
	    m_tilePositions = tilePositions;
	    std::vector<TilePosition>::iterator midTilePosition = std::max_element(m_tilePositions.begin(), m_tilePositions.end(),
	        [](TilePosition a, TilePosition b){ return a.second->getDistNearestUnpathable() < b.second->getDistNearestUnpathable(); });

	    m_center = *midTilePosition;
	    m_id = cp_id;
	}

	std::pair<const Region *, const Region *> ChokePoint::getRegions() const{
		return m_regions;
	}

	size_t ChokePoint::size() const {

		return m_tilePositions.size();
	}

	sc2::Point2D ChokePoint::getMidPoint() const {

	    return m_center.first;
	}

	std::vector<sc2::Point2D> ChokePoint::getPoints() const{
	    std::vector<sc2::Point2D> points;

	    for(auto const & tilePosition : m_tilePositions) {
	        points.push_back(tilePosition.first);
	    }

	    return points;
	}

	std::vector<TilePosition> ChokePoint::getTilePositions() const {
		return m_tilePositions;
	}

	std::string ChokePoint::getStringId() const {
        size_t region_a_id = m_regions.first->getId();
        size_t region_b_id = m_regions.second->getId();

        return "(" + std::to_string(region_a_id) + ", " + std::to_string(region_b_id) + ", " + std::to_string(m_id) + ")";
    }

    ChokePointId ChokePoint::getId() const {
    	return std::make_tuple(getRegions().first->getId(), getRegions().second->getId(), m_id);
    }

    size_t ChokePoint::getChokePointId() const { return m_id; }

    bool ChokePoint::operator==(const ChokePoint& rhs) const {
        return (this->getChokePointId() == rhs.getChokePointId()) && (this->getRegions() == rhs.getRegions());
    }

    bool ChokePoint::adjacent(const ChokePoint& cp) const {
        return (
            (getRegions().first == cp.getRegions().first)
            || (getRegions().first == cp.getRegions().second)
        ) || (
            (getRegions().second == cp.getRegions().first)
            || (getRegions().second == cp.getRegions().second)
        );
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



	/*
	****************************
	*** Priavte members stop ***
	****************************
	*/
}
