//
// Created by  on 2023/11/13.
//

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "../datastructure/merge_tree.hpp"
#include "doctest.h"

using namespace U;
using data_t = std::pair<int, int>;

struct MergeOp {
    auto operator()(const data_t &a, const data_t &b) -> data_t {
        return std::make_pair(std::min(a.first, b.first), std::max(a.second, b.second));
    }
};

struct compare {
    auto operator()(const data_t &a, const data_t &b) -> OverlapState {
        if (a.second < b.first) {
            return OverlapState::LEFT_OUTSIDE;
        } else if (b.second < a.first) {
            return OverlapState::RIGHT_OUTSIDE;
        } else if (a.first <= b.first && b.second <= a.second || b.first <= a.first && a.second <= b.second) {
            return OverlapState::OVERLAP;
        } else if (a.first <= b.first && a.second <= b.second) {
            return OverlapState::LEFT_CONNECT;
        } else {
            return OverlapState::RIGHT_CONNECT;
        }
    }
};

using MergeTreeP = MergeTree<data_t, MergeOp, compare>;


TEST_CASE("test merge tree") {
    auto tree = MergeTreeP(std::make_pair(0, 10));
    CHECK(tree.get_data() == std::make_pair(0, 10));
    auto node0 = std::make_unique<MergeTreeP>(std::make_pair(-10, 0));
    tree.merge(std::move(node0));
    CHECK(tree.get_data() == std::make_pair(-10, 10));

    SUBCASE("left side") {
        tree.merge(std::make_unique<MergeTreeP>(std::make_pair(-50, -30)));
        tree.merge(std::make_unique<MergeTreeP>(std::make_pair(-20, -15)));
        tree.merge(std::make_unique<MergeTreeP>(std::make_pair(-60, -55)));
        CHECK(tree.get_data() == std::make_pair(-10, 10));

        tree.merge(std::make_unique<MergeTreeP>(std::make_pair(-15, -10)));
        CHECK(tree.get_data() == std::make_pair(-20, 10));

        tree.merge(std::make_unique<MergeTreeP>(std::make_pair(-70, -65)));
        tree.merge(std::make_unique<MergeTreeP>(std::make_pair(-60, -50)));
        CHECK(tree.get_data() == std::make_pair(-70, 10));
    }
}

