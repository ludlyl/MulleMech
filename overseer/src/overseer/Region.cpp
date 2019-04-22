#include "Region.h"

namespace Overseer{

	bool ComparePointPairs::operator()(const PointPair &l, const PointPair &r) const {
		return (l.first.x == r.first.x) && (l.first.y == r.first.y) && (l.second.x == r.second.x) && (l.second.y == r.second.y);
	}

	bool GreaterTile::operator()(std::shared_ptr<TilePosition> &a, std::shared_ptr<TilePosition> &b) const {

		return a->second->getDistNearestUnpathable() > b->second->getDistNearestUnpathable();
	}


	bool GreaterTileInstance::operator()(TilePosition &a, TilePosition &b) const {

		return a.second->getDistNearestUnpathable() > b.second->getDistNearestUnpathable();
	}

	/*
	****************************
	*** Public members start ***
	****************************
	*/

	Region::Region(){
		m_largestDistUnpathable = 0;
	}

	Region::Region(size_t regionId, std::shared_ptr<TilePosition> tilePosition){
        m_id = regionId;
        m_largestDistUnpathable = tilePosition->second->getDistNearestUnpathable();
        m_midPoint = tilePosition->first;
        addTilePosition(tilePosition);
    }

    size_t Region::getArea() const {

    	return m_tilePositions.size();
    }

    // std::vector<RegionEdge> Region::getEdges(){ return m_edges; }

    const std::vector<UnitPosition> Region::getNeutralUnitPositions(){

    	return m_neutralUnitPositions;
    }

    size_t Region::getId() const {

    	return m_id;
    }

    void Region::setId(size_t regionId) {
	    m_id = regionId;

	    for(auto & tilePosition : m_tilePositions) {
	        tilePosition.second->setRegionId(regionId);
	    }
	}

	std::vector<TilePosition> Region::getTilePositions() const {

		return m_tilePositions;
	}

	std::vector<sc2::Point2D> Region::getPoints() const {
        std::vector<sc2::Point2D> points;

        for(auto const & tilePosition : m_tilePositions) {
            points.push_back(tilePosition.first);
        }

        return points;
    }

    void Region::addTilePosition(std::shared_ptr<TilePosition> tilePosition) {
        tilePosition->second->setRegionId(m_id);

        float tileDistNearestUnpathable = tilePosition->second->getDistNearestUnpathable();

        if(m_largestDistUnpathable < tileDistNearestUnpathable) {
            m_largestDistUnpathable = tileDistNearestUnpathable;
            m_midPoint = tilePosition->first;
        }

        m_tilePositions.push_back(std::pair<sc2::Point2D, std::shared_ptr<Tile>>(tilePosition->first, tilePosition->second));
    }


    void Region::addTilePosition(TilePosition& tilePosition) {
        tilePosition.second->setRegionId(m_id);

        float tileDistNearestUnpathable = tilePosition.second->getDistNearestUnpathable();

        if(m_largestDistUnpathable < tileDistNearestUnpathable) {
            m_largestDistUnpathable = tileDistNearestUnpathable;
            m_midPoint = tilePosition.first;
        }

        m_tilePositions.push_back(tilePosition);
    }

    double Region::getLargestDistanceToUnpathable() const {

        return m_largestDistUnpathable;
    }

    sc2::Point2D Region::getMidPoint() const {

        return m_midPoint;
    }

    void Region::merge(Region region) {

        for (auto& tilePosition : region.getTilePositions()) {
            addTilePosition(tilePosition);
        }
    }

    void Region::clear() {
        m_tilePositions.clear();
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
