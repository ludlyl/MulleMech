#ifndef _MAPIMPL_H_
#define _MAPIMPL_H_

#include "Graph.h"
#include "Map.h"
#include "NeutralSetObj.h"
#include "spatial/box_multimap.hpp"
#include "spatial/neighbor_iterator.hpp"
#include "spatial/ordered_iterator.hpp"

namespace Overseer{

    /**
    * \class MapImpl MapImpl.h "MapImpl.h"
    * \ brief hejsan svejsan
    */
    class MapImpl : public Map {
        public:

            ~MapImpl();

            /**
            * \brief constructor which adds the bot for temp control of map.
            *
            * \param bot Is the starcraft II bot currently run.
            */
            MapImpl(sc2::Agent* bot);

            /**
            * \brief Defualt constructor without member init.
            */
            MapImpl();

            /**
            * \brief Initialize overseer, should be done after the map been loaded.
            */
            void initialize();

            /**
            * \brief get the graph representation of the map.
            */
            Graph getGraph() const;

        private:

            // Create the tiles from the map.
            void createTiles();
            // Iterate over the tiles and compute the distance to nearest unpathable tile
            void computeAltitudes();
            // Used to check the type of "object" on the possition.
            TileTerrain checkTerrainType(NeutralImpl* checkWith, sc2::Point2D& pos, const sc2::ObservationInterface* obs);
            // Iterate over all tiles, starting with those furthest away from unpathables (probable candidates for region centers), and add to neighboring region
            // Create new region if no neighboring region is found, if two are found merge the smaller into the larger or create frontier
            std::vector<Region> computeTempRegions();
            // Find the regions with a real area and add them to map, resolve the frontiers
            void createRegions(std::vector<Region> tmp_regions);
            // Find all possitions not added to regions and create the frontiers.
            void createFrontiers();

            Graph m_graph;
    };
}
#endif /* _MAPIMPL_H_ */
