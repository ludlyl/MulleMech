// The MIT License (MIT)
//
// Copyright (c) 2017-2018 Alexander Kurbatov

#pragma once

#include "Historican.h"

#include <chrono>

struct Timer {
    void Start();

    // Returns milliseconds passed between Start() and Finish()
    float Finish();

 private:
    std::chrono::time_point<std::chrono::high_resolution_clock> m_start;
};
