#pragma once

#include <functional>
#include <memory>
#include "UCTSearch.h"
#include "Network.h"

namespace leelapy 
{

class Advisor {
private:
    std::function<void(const char *)> m_publish_analysis;
    std::string m_weights_file;
    int m_playouts;

    GameState m_game;
    std::shared_ptr<Network> m_network;
    std::shared_ptr<UCTSearch> m_search;

    std::shared_ptr<std::thread> m_thread;

public:
    Advisor(const std::function<void(const char*)>& publish_analysis, const std::string& weights_file, int playouts=UCTSearch::UNLIMITED_PLAYOUTS);

    bool clear_cache();

    std::string name();

    bool loadsgf(const std::string& sgfStr);

    bool lz_analyze(int interval_msec);

    bool stop_analysis();

    bool is_analyzing();

private:
    void start_ponder();
};

} // namespace leelapy
