#include "Tile.h"

namespace Overseer{
	/*
	****************************
	*** Public members start ***
	****************************
	*/

	TileTerrain Tile::getTileTerrain(){

		return m_tileInfo.terrain;
	}

	float Tile::getTerrainHeight(){

		return m_tileInfo.terrainHeight;
	}

	void Tile::setTerrainHeight(float height) {
		m_tileInfo.terrainHeight = height;
	}

	void Tile::setTileTerrain(TileTerrain& terrain){

		m_tileInfo.terrain = terrain;
	}

	void Tile::setDistNearestUnpathable(float dist){
		m_distNearestUnpathable = dist;
	}

	float Tile::getDistNearestUnpathable() const {

		return m_distNearestUnpathable;
	}

	void Tile::setRegionId(size_t regionId) {
        m_regionId = regionId;
    }

    size_t Tile::getRegionId(){

        return m_regionId;
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
