#include "MapImpl.h"

namespace Overseer{
    /*
    ****************************
    *** Public members start ***
    ****************************
    */

    MapImpl::~MapImpl(){}

    MapImpl::MapImpl(sc2::Agent* bot):Map(bot){}

    MapImpl::MapImpl():Map(){}

    void MapImpl::initialize(){
        m_graph.setMap(this);
        createTiles();
        computeAltitudes();
        std::vector<Region> tmp_regions = computeTempRegions();
        createRegions(tmp_regions);
        createFrontiers();
        m_graph.createChokePoints();
        m_graph.computePaths();
    }

    Graph MapImpl::getGraph() const { return m_graph; }

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

    void MapImpl::createTiles() {
        NeutralImpl* neutralCheck = new NeutralImpl(m_bot->Observation());

        for (size_t x(0); x < m_width; ++x) {

            for (size_t y(0); y < m_height; ++y) {
                sc2::Point2D pos{ float(x),float(y) };
                TileTerrain set = checkTerrainType(neutralCheck, pos, m_bot->Observation());
                float height = m_bot->Observation()->TerrainHeight(pos);

                std::shared_ptr<Tile> tile = std::make_shared<Tile>();

                tile->setTerrainHeight(height);
                tile->setTileTerrain(set);
                tile->setRegionId(0);

                if(set != TileTerrain::flyOnly) {

                    m_buildableTiles.push_back(std::shared_ptr<TilePosition>(new TilePosition(std::make_pair(pos, tile))));

                } else {
                    //Add flying only tiles to k-d tree, for rapid retrieval when it's time to compute altitudes
                    tile->setDistNearestUnpathable(0);
                    addTile(pos, tile);
                }
            }
        }

        delete neutralCheck;
    }

    void MapImpl::computeAltitudes() {

        //For each buildable tile, find the distance to the nearest unbuildable tile
        for(auto& buildableTile: m_buildableTiles) {
            sc2::Point2D pos = buildableTile->first;

            for(spatial::neighbor_iterator<TilePositionContainer> iter = neighbor_begin(m_tilePositions, pos); iter != neighbor_end(m_tilePositions, pos); iter++) {

                if(iter->second->getTileTerrain() == TileTerrain::flyOnly){
                    buildableTile->second->setDistNearestUnpathable(distance(iter));

                    break;
                }
            }
        }

        std::sort(m_buildableTiles.begin(), m_buildableTiles.end(), GreaterTile());

        //Push buildable tiles into k-d tree
        for(const auto& buildableTile: m_buildableTiles) {
            addTile(buildableTile->first, buildableTile->second);
        }
    }

    TileTerrain MapImpl::checkTerrainType(NeutralImpl* checkWith, sc2::Point2D& pos, const sc2::ObservationInterface* obs){
        TileTerrain ret;

        if(checkWith->isMineral(pos)){
            ret = TileTerrain::mineral;
        } else if(checkWith->isGas(pos)){
            ret = TileTerrain::gas;
        } else if(checkWith->isDestructible(pos)){
            ret = TileTerrain::destructable;
        } else if(checkWith->isNagaTower(pos)){
            ret = TileTerrain::nagaTower;
        } else if(obs->IsPlacable(pos) && obs->IsPathable(pos)){
            ret = TileTerrain::buildAndPath;
        } else if(obs->IsPathable(pos)){
            ret = TileTerrain::path;
        } else if(obs->IsPlacable(pos)){
            ret = TileTerrain::build;
        } else {
            ret = TileTerrain::flyOnly;
        }

        return ret;
    }

    std::vector<Region> MapImpl::computeTempRegions() {
        std::vector<Region> tmp_regions(1);

        for(auto& buildableTile: m_buildableTiles) {
            std::pair<size_t, size_t> neighboringRegions = findNeighboringRegions(buildableTile);

            if(!neighboringRegions.first) {
                //std::cout << "New region " << tmp_regions.size() << std::endl;
                tmp_regions.emplace_back((size_t) tmp_regions.size(), buildableTile);

            } else if(!neighboringRegions.second) {
                //std::cout << "Add to region " << neighboringRegions.first << std::endl;
                tmp_regions[neighboringRegions.first].addTilePosition(buildableTile);

            } else {
                size_t smaller = neighboringRegions.first;
                size_t larger = neighboringRegions.second;
                if(tmp_regions[larger].getArea() < tmp_regions[smaller].getArea()) {
                    std::swap(smaller, larger);
                }

                if(tmp_regions[smaller].getArea() < constants::min_region_area ||
                   (buildableTile->second->getDistNearestUnpathable() / tmp_regions[smaller].getLargestDistanceToUnpathable() >= constants::merge_thresholds) ||
                   (buildableTile->second->getDistNearestUnpathable() / tmp_regions[larger].getLargestDistanceToUnpathable() >= constants::merge_thresholds)) {
                    //std::cout << "Merge " << smaller << " into " << larger << std::endl;
                    tmp_regions[larger].addTilePosition(buildableTile);
                    tmp_regions[larger].merge(tmp_regions[smaller]);
                    tmp_regions[smaller].clear();

                } else {
                    //std::cout << "No merge" << std::endl;
                    m_frontierPositions.push_back(buildableTile);
                }
            }
        }

        return tmp_regions;
    }

    void MapImpl::createRegions(std::vector<Region> tmp_regions) {
        size_t index = 1;

        for(auto& tmp_region: tmp_regions) {

            if(tmp_region.getArea() > 0) {
                tmp_region.setId(index);
                addRegion(tmp_region);
                index++;
            }
        }
    }

    void MapImpl::createFrontiers(){

        //Create frontier positions between regions
        for(auto& frontierPosition : getFrontierPositions()) {
            std::pair<size_t, size_t> neighboringRegions = findNeighboringRegions(frontierPosition);

            if(!neighboringRegions.second) {
                getRegion(neighboringRegions.first)->addTilePosition(frontierPosition);

            } else {
                size_t smaller = neighboringRegions.first;
                size_t larger = neighboringRegions.second;

                if(getRegion(larger)->getArea() < getRegion(smaller)->getArea()) {
                    std::swap(smaller, larger);
                    neighboringRegions = std::make_pair(smaller, larger);
                }

                if(m_rawFrontier.count(neighboringRegions)) {
                    m_rawFrontier[neighboringRegions].push_back(*frontierPosition);

                } else {
                    std::vector<TilePosition> regionFrontier = {*frontierPosition};
                    m_rawFrontier[neighboringRegions] = regionFrontier;
                }
            }
        }
    }

    /*
    ****************************
    *** Priavte members stop ***
    ****************************
    */
}
