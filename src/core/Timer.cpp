// The MIT License (MIT)
//
// Copyright (c) 2017-2018 Alexander Kurbatov

#include "Historican.h"
#include "Timer.h"

void Timer::Start() {
    m_start = std::chrono::high_resolution_clock::now();
}

float Timer::Finish() {
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::high_resolution_clock::now() - m_start).count();
    float milliseconds = static_cast<float>(duration) / 1000.0f;

    return milliseconds;
}
