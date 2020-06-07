#include "advisor.h"
#include <sstream>
#include <iostream>
#include <chrono>
#include <thread>

#include "config.h"
#include "GTP.h"
#include "SGFTree.h"
#include "SGFParser.h"

namespace leelapy
{

Advisor::Advisor(const std::function<void(const char*)>& publish_analysis, const std::string& weights_file, int playouts)
: m_publish_analysis(publish_analysis)
, m_weights_file(weights_file)
, m_playouts(playouts)
, m_network(std::make_shared<Network>()) {
    m_game.init_game(BOARD_SIZE, KOMI);
    m_network->initialize(playouts, weights_file);
}

bool Advisor::clear_cache() {
    m_network->nncache_clear();
    return true;
}

std::string Advisor::name() {
    return PROGRAM_NAME;
}

bool Advisor::loadsgf(const std::string& sgfStr) {
    std::istringstream ss(sgfStr);
    auto buffer = SGFParser::chop_stream(ss, 0)[0];

    SGFTree sgftree;
    sgftree.load_from_string(buffer);

    m_game = sgftree.follow_mainline_state(999);
    m_game.display_state();

    return true;
}

bool Advisor::lz_analyze(int interval_msec) {
    m_search = std::make_shared<UCTSearch>(m_game, *m_network, m_publish_analysis);

    cfg_analyze_tags = AnalyzeTags(interval_msec, m_game.board.get_to_move());

    // Now start pondering.
    if (!m_game.has_resigned()) {
        m_thread = std::make_shared<std::thread>([=] { m_search->ponder(); });
    }
    else {
        return false;
    }

    return true;
}

bool Advisor::stop_analysis() {
    if (m_search && m_search->is_running()) {
        m_search->set_playout_limit(0);
        m_search->set_visit_limit(0);

        m_thread->join();
        m_thread = nullptr;
    }
    return true;
}

bool Advisor::is_analyzing() {
    if (m_search) {
        return m_search->is_running();
    }
    return false;
}

} // namespace leelapy