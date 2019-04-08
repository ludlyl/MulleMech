#pragma once

#include "MicroPlugin.h"

class SCV : public MicroPlugin {
public:
    explicit SCV(Unit* unit);

    void OnStep();
};
