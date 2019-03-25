//
// Created by joelp on 2019-03-21.
//

#include "core/API.h"
#include "../BuildingPlacer.h"
#include <c++/8.2.1/iostream>
#include "BuildInfo.h"
#include "../Historican.h"

void BuildInfo::OnGameStart(Builder*) {

    std::cout << "Before GetStartLocation()" << "\n";
    //find point to building line
    auto start = m_observer->GetStartLocation(); //TODO why cant we get starting position?
    std::cout << "after GetStartLocation()" << start.x << "\n\n";
    buildingPoint = BuildingPlacer::GetPointFrontOfCC(m_observer->GetStartLocation());
    //find direction for building line
    baseKValue = BuildingPlacer::GetBaseKValue();
}

void BuildInfo::OnStep(Builder*) {
}
