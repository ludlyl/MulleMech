#ifndef _OVERSEER_GRAPH_H_
#define _OVERSEER_GRAPH_H_

#include "Map.h"
#include "ChokePoint.h"
#include "Region.h"
#include <algorithm>

namespace Overseer{

    class ChokePoint;

    typedef std::vector<ChokePoint> CPPath;
    typedef std::tuple<size_t, size_t, size_t> ChokePointId;

    /**
    * \struct astar_node Graph.h "src/Graph.h"
    * \brief Container for an chokepoint id and its fscore for A*
    */
    struct AstarNode {
        /**
        * \brief constructor sets chokepoint id and fscore.
        *
        * \param cp_id is the wanted chokepoint id for the node.
        * \param f the fscore for the node
        */
        AstarNode(ChokePointId cp_id, float f);

        ChokePointId choke_point_id;
        float f_score;

        /**
        * \brief comparator for A* node is equal if chokepoint ids are equal
        */
        bool operator==(const AstarNode& rhs) const;
    };

    /**
    * \struct LowerScore Graph.h "src/Graph.h"
    * \brief comparator for priority queue lowest fscore at the top.
    */
    struct LowerScore {
        bool operator()(const AstarNode& lhs, const AstarNode& rhs) const;
    };

    /**
    * \class Graph Graph.h "src/Graph.h"
    * \brief Is a mathematical "graphmap" for analysing.
    */
    class Graph {

        public:

            /**
            * \brief empty constructor
            */
            Graph();
            /**
            * \brief object constructor.
            *
            * \param map The map to make the graph on.
            */
            Graph(Map* map);

            /**
            * \brief Gets the chokepoints been descovered between two adjacent regions.
            *
            * \param region_id_a a region which is adjacent to region_id_b
            * \param region_id_b a region which is adjacent to region_id_a
            * \return A vector containing all the found chokepoint between the two regions.
            */
            std::vector<ChokePoint> getChokePoints(size_t region_id_a, size_t region_id_b) const;

            /**
            * \brief Gets all the chokepoints on the map.
            *
            * \return vector containing the chokepoints
            */
            std::vector<ChokePoint> getChokePoints() const;

            /**
            * \brief Get the spacific chokepoint from a chokepoint id.
            *
            * \param cp_id The id to the wanted chokepoint.
            * \return The chokepoint mapped to the cp_id.
            */
            ChokePoint getChokePoint(ChokePointId cp_id) const;

            /**
            * \brief Gets a sorted list of chokepoint between chokepoint cp_a and cp_b
            *
            * \param cp_a the first chokepoint.
            * \param cp_b the second chokepoint.
            * \return The chokepoint path between cp_a and cp_b.
            */
            CPPath& getPath(ChokePoint cp_a, ChokePoint cp_b);

            /**
            * \brief find and create the chokepoint on the map.
            */
            void createChokePoints();

            /**
            * \brief Calculate all paths between all chokepoints.
            */
            void computePaths();

            /**
            * \brief set the sc2 map to this class.
            *
            * \param map is a pointer to the map.
            */
            void setMap(Map *map);

        private:
            size_t num_regions;
            Map *p_map;
            std::vector<std::vector<std::vector<ChokePoint>>> m_ChokePointsMatrix;
            std::map<ChokePointId, std::map<ChokePointId, float>> m_ChokePointDistanceMap;
            std::map<ChokePointId, std::vector<ChokePoint>> m_ChokePointNeighbors;
            std::map<size_t, std::vector<ChokePoint>> m_RegionChokePoints;
            std::map<ChokePointId, std::map<ChokePointId, CPPath>> m_ChokePointPaths;

            // Check if region id is valid
            bool validId(size_t id_arg) const;
            // Cluster the point within a chokepoint
            std::vector<std::deque<TilePosition>>* createClustersFromFrontiers(std::pair<std::pair<size_t,size_t>, std::vector<TilePosition>> frontierByRegionPair);
            // Caculate the distances between adjacent chokepoints
            void initializeChokePointDistanceMap();
            // Sets the distance between cp_a and cp_b, if not adjacent distance -> infinity.
            void setChokePointDistance(ChokePointId cp_a, ChokePointId cp_b, float d);
            // Gets the distance between chokepoints cp_a and cp_b
            float getChokePointDistance(const ChokePoint& cp_a, const ChokePoint& cp_b);
            // Sets the CPPath path to the chokepoints cp_a and cp_b
            void setChokePointPath(const ChokePoint& cp_a, const ChokePoint& cp_b, CPPath path);
            // Uses A* to calculate the shortest path between Chokepoints a and b
            CPPath aStar(ChokePoint cp_a, ChokePoint cp_b);
            // calculate the shortest euclidean distance between cp_a and cp_b
            float bestHeuristic(ChokePoint cp_a, ChokePoint cp_b);
            // Reconstructs the path from A* output
            CPPath reconstructPath(std::map<ChokePointId, ChokePointId> came_from, ChokePoint current);
            // Returns the chokepoints id is the output from A*
            std::vector<ChokePointId> extractKeys(std::map<ChokePointId, ChokePointId> const& input_map);
    };
}

#endif /* _GRAPH_H_ */
