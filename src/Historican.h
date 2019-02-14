// The MIT License (MIT)
//
// Copyright (c) 2017-2018 Alexander Kurbatov

#pragma once

#include "core/API.h"

#include <fstream>
#include <iostream>
#include <string>
#include <unordered_set>

enum class LogSeverity {
    debug,
    info,
    warning,
    error
};

enum class LogChannel {
    general,
    scouting
};

inline std::string LogChannel_str(LogChannel channel) {
    switch (channel) {
    case LogChannel::general:       return "[GENERAL]";
    case LogChannel::scouting:      return "[SCOUTING]";
    default: break;
    }
    throw std::runtime_error("Invalid channel");
}

class Historican {
public:
    Historican();
    ~Historican();

    class HistoricianOut {
    public:
        HistoricianOut(std::ofstream& file, bool ignore_logging) :
            m_file(file), m_ignore_logging(ignore_logging) { }

        template <class T>
        HistoricianOut& operator<<(const T& data_) {
            if (m_ignore_logging)
                return *this;

            if (m_file.is_open())
                m_file << data_;

            std::cout << data_;

            return *this;
        }

        HistoricianOut& operator<<(std::ostream& (*manipulator_)(std::ostream&));

    private:
        std::ofstream& m_file;
        bool m_ignore_logging;
    };

    void Init(const std::string &filename_);

    // Filter away debug & info comments from said channel
    void AddFilter(LogChannel filter);

    void RemoveFilter(LogChannel filter);

    void SetSeverity(LogSeverity severity);

    HistoricianOut debug(LogChannel channel = LogChannel::general);

    HistoricianOut info(LogChannel channel = LogChannel::general);

    HistoricianOut warning();

    HistoricianOut error();

 private:
     bool IsFiltered(LogChannel channel) const;

    std::ofstream m_file;
    LogSeverity m_severity;
    std::unordered_set<LogChannel> m_filters;
};

extern Historican gHistory;
