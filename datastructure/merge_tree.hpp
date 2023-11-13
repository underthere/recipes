//
// Created by  on 2023/11/13.
//

#ifndef RECIPES_MERGE_TREE_HPP
#define RECIPES_MERGE_TREE_HPP

#include <cstdint>
#include <optional>
#include <string>
#include <utility>

namespace U {

    enum class OverlapState {
        LEFT_OUTSIDE = 0,
        LEFT_CONNECT,
        OVERLAP,
        RIGHT_CONNECT,
        RIGHT_OUTSIDE,
    };

    template<typename T, typename merge_op = T(const T &, const T &), typename compare = OverlapState(const T &, const T &)>
    class MergeTree {
        using node_t = MergeTree<T, merge_op, compare>;

    public:
        explicit MergeTree(const T &data) : data(data) {}

        void merge(std::unique_ptr<MergeTree>&& other) {
            switch (compare{}(data, other->data)) {
                case OverlapState::LEFT_CONNECT:
                case OverlapState::RIGHT_CONNECT:
                case OverlapState::OVERLAP:
                    this->data = merge_op{}(data, other->data);
                    break;
                case OverlapState::LEFT_OUTSIDE:
                    if (left) {
                        left->merge(std::move(other));
                    } else {
                        left = std::move(other);
                    }
                    return;
                case OverlapState::RIGHT_OUTSIDE:
                    if (right) {
                        right->merge(std::move(other));
                    } else {
                        right = std::move(other);
                    }
                    return;
            }


            if (left) {
                switch (compare{}(data, left->data)) {
                    case OverlapState::LEFT_CONNECT:
                    case OverlapState::OVERLAP:
                        auto new_left = std::move(left->left);
                        this->merge(std::move(left));
                        this->left = std::move(new_left);
                        break;
                }
            }

            if (right) {
                switch (compare{}(data, right->data)) {
                    case OverlapState::RIGHT_CONNECT:
                    case OverlapState::OVERLAP:
                        auto new_right = std::move(right->right);
                        this->merge(std::move(right));
                        this->right = std::move(new_right);
                        break;
                }
            }
        }

        auto get_data() const { return data; }

    private:
        T data;
        std::unique_ptr<node_t> parent{nullptr};
        std::unique_ptr<node_t> left{nullptr};
        std::unique_ptr<node_t> right{nullptr};
    };

}// namespace U

#endif//RECIPES_MERGE_TREE_HPP
