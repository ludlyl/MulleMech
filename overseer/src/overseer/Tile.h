#ifndef Tile_h
#define Tile_h

#include <cstdio>
#include "Definitions.h"

namespace Overseer{

    enum TileTerrain{
        buildAndPath,
        build,
        path,
        mineral,
        gas,
        nagaTower,
        destructable,
        flyOnly
    };

    /**
    * \class Tile Tile.h "Tile.h"
    * \brief A tile is area that has size 1x1 within SCII maps.
    */
    class Tile {
        public:
            /**
            * \brief Says what type of terrain it is on this tile, see TileTerrain for alteritives.
            *
            * \return the terrain determed for THIS tile.
            */
            TileTerrain getTileTerrain();

            /**
            * \brief Get the z-axis value for the tile.
            *
            * \return z-axis value.
            */
            float getTerrainHeight();

            /**
            * \brief Sets the z-axis value for the tile
            *
            * \param z-axis value of tile
            */
            void setTerrainHeight(float height);

            /**
            * /// Currentlyt no use \\\
            * \brief If there exist build or wall.
            *
            * \return true if wall or build exist, false otherwise.
            */
            // bool Doodad();

            /**
            * \brief Sets a terrain for THIS tile.
            *
            * \param terrain that will be set for the tile
            */
            void setTileTerrain(TileTerrain& terrain);

            /**
            * \brief Set a distance to the nearest unpathable tile.
            *
            * \param dist The distance til unpathable.
            */
            void setDistNearestUnpathable(float dist);

            /**
            * \brief Get the distance to nearest unpathable.
            *
            * \return distance.
            */
            float getDistNearestUnpathable() const;

            /**
            * \brief Set the region id this tile belong to.
            *
            * \param regionId the id to set.
            */
            void setRegionId(size_t regionId);

            /**
            * \brief Get the region id this tile is in.
            *
            * \return regionid
            */
            size_t getRegionId();

            /**
            * \brief Check if the tile contain neutral unit, e.g. gas, miniral and destroable
            */
            bool isNeutral();

        private:

            struct TileInfo {
                TileTerrain     terrain;
                float           terrainHeight;
            };

            TileInfo m_tileInfo;

            size_t m_regionId;
            float m_distNearestUnpathable;

    };
}

#endif /* Tile_h */
