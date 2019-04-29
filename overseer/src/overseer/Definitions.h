
#ifndef _OVERSEER_DEFINITIONS_H_
#define _OVERSEER_DEFINITIONS_H_

#include "spatial/box_multimap.hpp"
#include "sc2api/sc2_api.h"

namespace Overseer{
	/**
	* \brief Namespace constants is though as something users can change to effect Overseer's regions/ChokePoints detection.
	*/
	namespace constants {
		// Defined distance between ChokePoints
	    static const size_t min_cluster_distance = 10;

	    // Region defined area
	    static const size_t min_region_area = 30;

	    // Defined merge threshold for region area ratio between neighboring regions
	    static const float merge_thresholds = 0.70f;
	}
	/**
	* \brief Namespace accessors used for comparing in spatial maps. Should not be changed.
	*/
	namespace accessors {
		struct point2d_accessor {
	        int operator() (spatial::dimension_type dim, const sc2::Point2D p) const {

		        switch(dim) {
		            case 0: return int(p.x);
		            case 1: return int(p.y);
		            default: throw std::out_of_range("dim");
		        }
		    }
    	};
	}
}


#endif /* _OVERSEER_DEFINITIONS_H_ */
