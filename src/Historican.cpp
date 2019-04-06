// The MIT License (MIT)
//
// Copyright (c) 2017-2018 Alexander Kurbatov

#include "Historican.h"

#include <iomanip>

Historican::Historican() : m_severity(LogSeverity::info) {

}

Historican::~Historican() {
    if (m_file.is_open())
        m_file.close();
}

void Historican::Init(const std::string& filename_) {
    if (m_file.is_open())
        m_file.close();

    m_file.open(filename_.c_str());

    std::cout << std::fixed << std::setprecision(3);
    if (m_file.is_open())
        m_file << std::fixed << std::setprecision(3);
}

void Historican::AddFilter(LogChannel filter) {
    m_filters.insert(filter);;
}

void Historican::RemoveFilter(LogChannel filter) {
    m_filters.erase(filter);
}

bool Historican::IsFiltered(LogChannel channel) const {
    return m_filters.count(channel) > 0;
}

void Historican::SetSeverity(LogSeverity severity) {
    m_severity = severity;
}

Historican::HistoricianOut Historican::debug(LogChannel channel) {
    if (m_severity != LogSeverity::debug || IsFiltered(channel))
        return HistoricianOut(m_file, true);
    HistoricianOut out(m_file, false);
    out << "#" << gAPI->observer().GetGameLoop() << " [DEBUG]" << LogChannel_str(channel) << " ";
    return out;
}

Historican::HistoricianOut Historican::info(LogChannel channel) {
    if ((m_severity != LogSeverity::debug && m_severity != LogSeverity::info) || IsFiltered(channel))
        return HistoricianOut(m_file, true);
    HistoricianOut out(m_file, false);
    out << "#" << gAPI->observer().GetGameLoop() << " [INFO]" << LogChannel_str(channel) << " ";
    return out;
}

Historican::HistoricianOut Historican::warning() {
    HistoricianOut out(m_file, false);
    out << "#" << gAPI->observer().GetGameLoop() << " [WARNING] ";
    return out;
}

Historican::HistoricianOut Historican::error() {
    HistoricianOut out(m_file, false);
    out << "#" << gAPI->observer().GetGameLoop() << " [ERROR] ";
    return out;
}

Historican::HistoricianOut& Historican::HistoricianOut::operator<<(std::ostream& (*manipulator_)(std::ostream&)) {
    if (m_ignore_logging)
        return *this;

    if (m_file.is_open())
        m_file << manipulator_;

    std::cout << manipulator_;

    return *this;
}

Historican gHistory;
