#include "Graph.h"

namespace Overseer{

	/*
	***********************************
	*** Struct implementation start ***
	***********************************
	*/
	AstarNode::AstarNode(ChokePointId cp_id, float f){
		choke_point_id = cp_id;
		f_score = f;
	}

	bool AstarNode::operator==(const AstarNode& rhs) const {
		return choke_point_id == rhs.choke_point_id;
	}

	bool LowerScore::operator()(const AstarNode& lhs, const AstarNode& rhs) const {
		return lhs.f_score > rhs.f_score;
	}
	/*
	*********************************
	*** Struct implementation end ***
	*********************************
	*
	*********************************
	*
	****************************
	*** Public members start ***
	****************************
	*/

	Graph::Graph(){}

	Graph::Graph(Map* map):p_map(map){}

	std::vector<ChokePoint> Graph::getChokePoints(size_t region_id_a, size_t region_id_b) const {
        std::vector<ChokePoint> result_vector;
        if(validId(region_id_a) && validId(region_id_b) && (region_id_a != region_id_b)) {
            if (region_id_a > region_id_b) {
                std::swap(region_id_a, region_id_b);
            }

            result_vector =  m_ChokePointsMatrix[region_id_b][region_id_a];
        }
        return result_vector;
    }

    std::vector<ChokePoint> Graph::getChokePoints() const {
        std::vector<ChokePoint> choke_points;
        std::vector<ChokePointId> ids;
        for (size_t i = 1; i <= num_regions; ++i) {
            for (size_t j = i + 1; j <= num_regions; ++j) {
                std::vector<ChokePoint> ij_choke_points = getChokePoints(i, j);
                for(auto const& ij_cp : ij_choke_points) {
                    bool id_exists = false;
                    for(auto const& id : ids) {
                        ChokePointId ij_id = ij_cp.getId();
                        if(id == ij_id) {
                            id_exists = true;
                        }
                    }
                    if(!id_exists) {
                        choke_points.push_back(ij_cp);
                        ids.push_back(ij_cp.getId());
                    }
                }
            }
        }

        return choke_points;
    }

    ChokePoint Graph::getChokePoint(ChokePointId cp_id) const {
        size_t region_id_a = std::get<0>(cp_id);
        size_t region_id_b = std::get<1>(cp_id);
        size_t cp_pos = std::get<2>(cp_id);

        assert(validId(region_id_a) && validId(region_id_b));

        if (region_id_a > region_id_b) {
            std::swap(region_id_a, region_id_b);
        }

        return m_ChokePointsMatrix[region_id_b][region_id_a][cp_pos];
    }

	CPPath& Graph::getPath(ChokePoint cp_a, ChokePoint cp_b) {
        return m_ChokePointPaths[cp_a.getId()][cp_b.getId()];
    }

    void Graph::createChokePoints() {
        num_regions = p_map->getRegions().size();

        m_ChokePointsMatrix.resize(num_regions + 1);

        for(size_t i = 1; i <= num_regions; ++i) {
            m_ChokePointsMatrix[i].resize(i);
        }

        for(const auto& rawFrontier : p_map->getRawFrontier()) {
            std::vector<std::deque<TilePosition>>* clusters = createClustersFromFrontiers(rawFrontier);
            size_t region_id_a = rawFrontier.first.first;
            size_t region_id_b = rawFrontier.first.second;

            if (region_id_a > region_id_b) {
                std::swap(region_id_a, region_id_b);
            }

            //Create each cluster as a separate ChokePoint
            size_t num_clusters = 0;
            for(auto cluster : *clusters) {
                std::vector<TilePosition> clusterPositions;

                while (!cluster.empty()) {
                    TilePosition clusterPosition = cluster.front();
                    clusterPositions.push_back(clusterPosition);
                    cluster.pop_front();
                }

                ChokePoint cp(p_map->getRegion(region_id_a), p_map->getRegion(region_id_b), num_clusters, clusterPositions);

                m_ChokePointsMatrix[region_id_b][region_id_a].push_back(cp);
                m_RegionChokePoints[region_id_a].push_back(cp);
                m_RegionChokePoints[region_id_b].push_back(cp);

                num_clusters++;
            }
            delete clusters;
        }
    }

    void Graph::computePaths(){
        initializeChokePointDistanceMap();

        for(auto& cp_a : getChokePoints()) {
            for(auto& cp_b : getChokePoints()) {
                setChokePointPath(cp_a, cp_b, aStar(cp_a, cp_b));
            }
        }
    }

    void Graph::setMap(Map *map) {
    	p_map = map;
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

	bool Graph::validId(size_t id_arg) const{
		return (id_arg >= 1) && (id_arg <= (num_regions + 1));
	}

    std::vector<std::deque<TilePosition>>* Graph::createClustersFromFrontiers(std::pair<std::pair<size_t,size_t>, std::vector<TilePosition>> frontierByRegionPair) {

        std::vector<std::deque<TilePosition>>* clusters = new std::vector<std::deque<TilePosition>>;
        std::vector<TilePosition> frontierPositions = frontierByRegionPair.second;

        std::sort(frontierPositions.begin(), frontierPositions.end(), GreaterTileInstance());

        for(auto frontierPosition : frontierPositions) {
            bool added = false;
            for(auto & cluster : *clusters) {
                float dist_front = sc2::Distance2D(frontierPosition.first, cluster.front().first);
                float dist_back = sc2::Distance2D(frontierPosition.first, cluster.back().first);

                if(std::min(dist_front, dist_back) <= constants::min_cluster_distance) {
                    if(dist_front < dist_back) {
                        cluster.push_front(frontierPosition);
                    } else {
                        cluster.push_back(frontierPosition);
                    }
                    added = true;
                    break;
                }
            }
            if(!added) {
                //Record new ChokePoint
                std::deque<TilePosition> clusterPositions;
                clusterPositions.push_back(frontierPosition);
                clusters->push_back(clusterPositions);
            }
        }

        return clusters;
    }

    void Graph::initializeChokePointDistanceMap() {
        std::vector<ChokePoint> chokes = getChokePoints();
        std::vector<std::vector<sc2::Point2D>> chokesIndividualPoints;
		
        for (int i = 0; i < chokes.size(); ++i){
			chokesIndividualPoints.push_back(chokes[i].getPoints());
		}

        for (int a = 0; a < chokes.size(); ++a) {
            ChokePoint& cp_a = chokes[a];
            for (int b = a; b < chokes.size(); ++b) {
                ChokePoint& cp_b = chokes[b];
                float dist = std::numeric_limits<float>::infinity();
                if(cp_a == cp_b) {
                    dist = 0.0;
                } else if(cp_a.adjacent(cp_b)) {

                    //create an array of queries
                    std::vector<sc2::QueryInterface::PathingQuery> queries;
                    for (auto& point_a : chokesIndividualPoints[a]){
						for (auto& point_b : chokesIndividualPoints[b]){
							queries.push_back({ sc2::NullTag, point_a, point_b });
						}
					}

                    std::vector<float> distances = p_map->getBot()->Query()->PathingDistance(queries);

                    for (float alt_dist : distances){
						if(alt_dist < dist){
							dist = alt_dist;
						}
					}

                    m_ChokePointNeighbors[cp_a.getId()].push_back(cp_b);
                    m_ChokePointNeighbors[cp_b.getId()].push_back(cp_a);
                }
                setChokePointDistance(cp_a.getId(), cp_b.getId(), dist);
                setChokePointDistance(cp_b.getId(), cp_a.getId(), dist);
            }
        }
    }



    void Graph::setChokePointDistance(ChokePointId cp_a, ChokePointId cp_b, float d) {
        m_ChokePointDistanceMap[cp_a][cp_b] = d;
    }

    float Graph::getChokePointDistance(const ChokePoint& cp_a, const ChokePoint& cp_b) {
        return m_ChokePointDistanceMap[cp_a.getId()][cp_b.getId()];
    }

    void Graph::setChokePointPath(const ChokePoint& cp_a, const ChokePoint& cp_b, CPPath path) {
		std::reverse(path.begin(), path.end());
        m_ChokePointPaths[cp_a.getId()][cp_b.getId()] = path;
    }

    CPPath Graph::aStar(ChokePoint cp_a, ChokePoint cp_b) {
        CPPath total_path;
        AstarNode start(cp_a.getId(), sc2::Distance2D(cp_a.getMidPoint(), cp_b.getMidPoint()));

        std::map<ChokePointId, ChokePointId> came_from;
        ChokePoint current = cp_a;

        //Use reversed comparator to raise the node with lowest f_score to the top
        std::priority_queue<AstarNode, std::vector<AstarNode>, LowerScore> open_set;
        std::vector<ChokePointId> visited;
        std::map<ChokePointId, float> g_score;

        open_set.push(start);

        for(auto& choke_point : getChokePoints()) {
            g_score[choke_point.getId()] = std::numeric_limits<float>::infinity();
        }
        g_score[cp_a.getId()] = 0;

        while(!open_set.empty()) {
            ChokePoint current = getChokePoint(open_set.top().choke_point_id);
            if(current == cp_b) {
                return reconstructPath(came_from, current);
            }
            open_set.pop();
            visited.push_back(current.getId());

            for(auto& neighbor : m_ChokePointNeighbors[current.getId()]) {
                if(std::find(visited.begin(), visited.end(), neighbor.getId()) != visited.end()) {
                    continue;
                }

                float t_score = g_score[current.getId()] + getChokePointDistance(current, neighbor);
                if(t_score >= g_score[neighbor.getId()]) {
                    continue;
                }

                came_from[neighbor.getId()] = current.getId();
                g_score[neighbor.getId()] = t_score;
                float heuristic = sc2::Distance2D(neighbor.getMidPoint(), cp_b.getMidPoint());
                float f = g_score[neighbor.getId()] + heuristic;
                AstarNode neighbor_node(neighbor.getId(), f);

                open_set.push(neighbor_node);
            }
        }

        return total_path;
    }

    float Graph::bestHeuristic(ChokePoint cp_a, ChokePoint cp_b) {
        float dist = std::numeric_limits<float>::infinity();
        for(auto& point_a : cp_a.getPoints()) {
            for(auto& point_b : cp_b.getPoints()) {
                float alt_dist = sc2::Distance2D(point_a, point_b);
                if(alt_dist < dist) {
                    dist = alt_dist;
                }
            }
        }
        return dist;
    }

    CPPath Graph::reconstructPath(std::map<ChokePointId, ChokePointId> came_from, ChokePoint current) {
        CPPath total_path;
        total_path.push_back(current);

        std::vector<ChokePointId> keys = extractKeys(came_from);

        while(std::find(keys.begin(), keys.end(), current.getId()) != keys.end()) {
            current = getChokePoint(came_from[current.getId()]);
            total_path.push_back(current);
        }

        return total_path;
    }

    std::vector<ChokePointId> Graph::extractKeys(std::map<ChokePointId, ChokePointId> const& input_map) {
        std::vector<ChokePointId> retval;
        for (auto const& element : input_map) {
            retval.push_back(element.first);
        }
        return retval;
    }

	/*
	****************************
	*** Priavte members stop ***
	****************************
	*/
}
