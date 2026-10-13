// Copyright (C) 2026 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/api.hpp>
#include <stormkit/core/contract_macro.hpp>
#include <stormkit/core/platform_macro.hpp>

export module stormkit.core.containers.tree;

import std;

import stormkit.core.typesafe;
import stormkit.core.contract;
import stormkit.core.function_ref;
import stormkit.core.filesystem;
import stormkit.core.types;
import stormkit.core.errors;

namespace stdr  = std::ranges;
namespace stdfs = std::filesystem;

export namespace stormkit { inline namespace core {
    class STORMKIT_CORE_API tree_node {
      public:
        using index_type    = u32;
        using bitfield_type = u32;

        static constexpr auto invalid_index = std::numeric_limits<index_type>::max();

        [[nodiscard]]
        auto name() const noexcept -> const string&;
        auto set_name(const string& name) noexcept -> void;
        auto set_name(string&& name) noexcept -> void;

        [[nodiscard]]
        auto parent() const noexcept -> index_type;
        auto set_parent(index_type index) noexcept -> void;

        auto next_sibling() const noexcept -> index_type;
        auto set_next_sibling(index_type index) noexcept -> void;

        auto first_child() const noexcept -> index_type;
        auto set_first_child(index_type index) noexcept -> void;

        auto dirty_bits() const noexcept -> const bitfield_type&;
        auto set_dirty_bits(bitfield_type bits) noexcept -> void;

        auto invalidate() noexcept -> void;

      private:
        index_type    parent_       = invalid_index;
        index_type    next_sibling_ = invalid_index;
        index_type    first_child_  = invalid_index;
        bitfield_type dirty_bits_   = 0;

        string name_;
    };

    template<typename T = tree_node>
    class tree {
      public:
        static constexpr auto default_preallocated_tree_size = usize { 10 };

        using tree_node_type          = T;
        using tree_node_index_type    = typename tree_node_type::index_type;
        using tree_node_bitfield_type = typename tree_node_type::bitfield_type;

        tree();
        ~tree();

        tree(const tree&);
        auto operator=(const tree&) -> tree&;

        tree(tree&&);
        auto operator=(tree&&) -> tree&;

        auto get_free_node() -> tree_node_index_type;

        auto insert(tree_node_type&& node, tree_node_index_type parent_index, tree_node_index_type previous_sibling)
          -> tree_node_index_type;
        auto remove(tree_node_index_type index) -> void;

        auto mark_dirty(tree_node_index_type index, tree_node_bitfield_type bits) -> void;

        auto operator[](tree_node_index_type index) noexcept -> tree_node_type&;
        auto operator[](tree_node_index_type index) const noexcept -> const tree_node_type&;

        [[nodiscard]]
        auto size() const noexcept -> usize;

        [[nodiscard]]
        auto begin() noexcept;
        [[nodiscard]]
        auto begin() const noexcept;
        [[nodiscard]]
        auto cbegin() const noexcept;

        [[nodiscard]]
        auto end() noexcept;
        [[nodiscard]]
        auto end() const noexcept;
        [[nodiscard]]
        auto cend() const noexcept;

        auto clear_dirties() noexcept -> void;
        [[nodiscard]]
        auto dirties() const noexcept -> array_view<const tree_node_index_type>;

        auto gen_dot_file(stdfs::path filepath, std23::function_ref<string_view(string_view)> colorize_node) const noexcept
          -> system_result<usize>;

        auto gen_dot_file(stdfs::path                                   filepath,
                          core::u32                                     highlight,
                          std23::function_ref<string_view(string_view)> colorize_node) const noexcept -> system_result<usize>;

      private:
        tree_node_index_type           first_free_index_ = 0;
        dynarray<tree_node_type>       tree_;
        dynarray<tree_node_index_type> dirties_;
    };
}} // namespace stormkit::core

////////////////////////////////////////////////////////////////////
///                      IMPLEMENTATION                          ///
////////////////////////////////////////////////////////////////////

namespace stormkit { inline namespace core {
    ////////////////////////////////////////
    ////////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto tree_node::name() const noexcept -> const string& {
        return name_;
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto tree_node::set_name(const string& name) noexcept -> void {
        name_ = string { name };
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto tree_node::set_name(string&& name) noexcept -> void {
        name_ = std::move(name);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto tree_node::parent() const noexcept -> tree_node::index_type {
        return parent_;
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto tree_node::set_parent(index_type index) noexcept -> void {
        parent_ = index;
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto tree_node::next_sibling() const noexcept -> tree_node::index_type {
        return next_sibling_;
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto tree_node::set_next_sibling(index_type index) noexcept -> void {
        next_sibling_ = index;
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto tree_node::first_child() const noexcept -> tree_node::index_type {
        return first_child_;
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto tree_node::set_first_child(index_type index) noexcept -> void {
        first_child_ = index;
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto tree_node::dirty_bits() const noexcept -> const tree_node::bitfield_type& {
        return dirty_bits_;
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    STORMKIT_FORCE_INLINE
    inline auto tree_node::set_dirty_bits(bitfield_type bits) noexcept -> void {
        dirty_bits_ = bits;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    inline auto tree_node::invalidate() noexcept -> void {
        parent_       = { invalid_index };
        next_sibling_ = { invalid_index };
        first_child_  = { invalid_index };
        dirty_bits_   = 0;
        name_         = "";
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename tree_nodeClass>
    tree<tree_nodeClass>::tree() {
        tree_.resize(default_preallocated_tree_size);

        for (auto i : range<tree_node_index_type>(stdr::size(tree_) - 1u)) tree_[i].set_next_sibling(i + 1u);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename tree_nodeClass>
    STORMKIT_FORCE_INLINE
    tree<tree_nodeClass>::~tree() = default;

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename tree_nodeClass>
    STORMKIT_FORCE_INLINE
    tree<tree_nodeClass>::tree(const tree&) = default;

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename tree_nodeClass>
    STORMKIT_FORCE_INLINE
    tree<tree_nodeClass>::tree(tree&&) = default;

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename tree_nodeClass>
    STORMKIT_FORCE_INLINE
    auto tree<tree_nodeClass>::operator=(const tree&) -> tree& = default;

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename tree_nodeClass>
    STORMKIT_FORCE_INLINE
    auto tree<tree_nodeClass>::operator=(tree&&) -> tree& = default;

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename tree_nodeClass>
    auto tree<tree_nodeClass>::get_free_node() -> tree_node_index_type {
        if (tree_[first_free_index_].next_sibling() == tree_node::invalid_index) {
            const auto size      = as<f32>(stdr::size(tree_));
            const auto first_new = as<tree_node_index_type>(stdr::size(tree_));

            tree_.resize(as<usize>(size * 1.5f));
            const auto new_size = stdr::size(tree_);

            // generate a new chain of free objects, with the last one pointing to
            // ~0
            tree_[first_free_index_].set_next_sibling(first_new);
            for (auto i : range(first_new, new_size - 1u)) tree_[i].set_next_sibling(i + 1u);
        }

        auto index        = first_free_index_;
        first_free_index_ = tree_[first_free_index_].next_sibling();

        return index;
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename tree_nodeClass>
    auto tree<tree_nodeClass>::insert(tree_node_type&&     node,
                                      tree_node_index_type parent_index,
                                      tree_node_index_type previous_sibling) -> tree_node_index_type {
        const auto index = get_free_node();

        auto& _node = tree_[index];
        _node       = std::forward<tree_node_type>(node);

        _node.set_parent(parent_index);

        // check if parent is real node
        if (parent_index != tree_node::invalid_index) {
            auto& parent_node = *(std::ranges::begin(tree_) + parent_index);

            // new node is first child
            if (parent_node.first_child() == tree_node::invalid_index) parent_node.set_first_child(index);
            else if (previous_sibling == tree_node::invalid_index) { // insert a beginning of childs
                _node.set_next_sibling(parent_node.first_child());
                parent_node.set_first_child(index);
            } else { // insert at the end
                auto& prev_sibling_node = tree_[previous_sibling];
                _node.set_next_sibling(prev_sibling_node.next_sibling());
                prev_sibling_node.set_next_sibling(index);
            }
        }

        return index;
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename tree_nodeClass>
    auto tree<tree_nodeClass>::remove(tree_node_index_type index) -> void {
        auto& node = tree_[index];

        if (node.parent() != tree_node::invalid_index) {
            auto& parent = tree_[node.parent()];

            // Remove sibling
            auto current_index = parent.first_child();
            while (current_index != tree_node::invalid_index) {
                auto& current_node = tree_[current_index];

                if (current_node.next_sibling() == index) {
                    current_node.set_next_sibling(node.next_sibling());
                    break;
                }
                current_index = current_node.next_sibling();
            }

            // remove parent
            if (parent.first_child() == index) parent.set_first_child(node.next_sibling());

            node.set_parent(tree_node::invalid_index);
        }

        auto last_index = tree_node::invalid_index;
        auto queue      = std::deque<tree_node_index_type> {};
        queue.emplace_back(index);
        while (not queue.empty()) {
            auto  current_index = queue.front();
            auto& current_node  = tree_[current_index];
            queue.pop_front();

            auto child_index = current_node.first_child();
            while (child_index != tree_node::invalid_index) {
                queue.emplace_back(child_index);
                child_index = tree_[child_index].next_sibling();
            }

            node.invalidate();

            if (last_index != tree_node::invalid_index) tree_[last_index].set_next_sibling(current_index);

            last_index = current_index;
        }

        tree_[last_index].set_next_sibling(first_free_index_);
        first_free_index_ = index;
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename tree_nodeClass>
    auto tree<tree_nodeClass>::mark_dirty(tree_node_index_type index, tree_node_bitfield_type bits) -> void {
        auto& node = tree_[index];
        if (not node.dirty_bits()) {
            dirties_.emplace_back(index);
            node.set_dirty_bits(bits);
            return;
        }

        node.set_dirty_bits(bits);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename tree_nodeClass>
    STORMKIT_FORCE_INLINE
    inline auto tree<tree_nodeClass>::operator[](tree_node_index_type index) noexcept -> tree_node_type& {
        EXPECTS(index < stdr::size(tree_));

        return tree_[index];
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename tree_nodeClass>
    STORMKIT_FORCE_INLINE
    inline auto tree<tree_nodeClass>::operator[](tree_node_index_type index) const noexcept -> const tree_node_type& {
        EXPECTS(index < stdr::size(tree_));

        return tree_[index];
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename tree_nodeClass>
    STORMKIT_FORCE_INLINE
    inline auto tree<tree_nodeClass>::size() const noexcept -> usize {
        return stdr::size(tree_);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename tree_nodeClass>
    STORMKIT_FORCE_INLINE
    inline auto tree<tree_nodeClass>::begin() noexcept {
        return std::ranges::begin(tree_);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename tree_nodeClass>
    STORMKIT_FORCE_INLINE
    inline auto tree<tree_nodeClass>::begin() const noexcept {
        return std::cbegin(tree_);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename tree_nodeClass>
    STORMKIT_FORCE_INLINE
    inline auto tree<tree_nodeClass>::cbegin() const noexcept {
        return std::cbegin(tree_);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename tree_nodeClass>
    STORMKIT_FORCE_INLINE
    inline auto tree<tree_nodeClass>::end() noexcept {
        return std::ranges::end(tree_);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename tree_nodeClass>
    STORMKIT_FORCE_INLINE
    inline auto tree<tree_nodeClass>::end() const noexcept {
        return std::cend(tree_);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename tree_nodeClass>
    STORMKIT_FORCE_INLINE
    inline auto tree<tree_nodeClass>::cend() const noexcept {
        return std::cend(tree_);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename tree_nodeClass>
    auto tree<tree_nodeClass>::clear_dirties() noexcept -> void {
        if (std::empty(dirties_)) return;

        for (auto i : dirties_) { tree_[i].set_dirty_bits(0); }

        dirties_.clear();
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename tree_nodeClass>
    STORMKIT_FORCE_INLINE
    inline auto tree<tree_nodeClass>::dirties() const noexcept -> array_view<const tree_node_index_type> {
        return dirties_;
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename tree_nodeClass>
    auto tree<tree_nodeClass>::gen_dot_file(stdfs::path                                   filepath,
                                            std23::function_ref<string_view(string_view)> colorize_node) const noexcept
      -> system_result<usize> {
        using namespace stormkit::literals;
        auto out = string {};
        out.reserve(1_kb);

        out += "digraph G { \n"
               "    rankdir = LR\n"
               "    bgcolor = black\n\n"
               "    node [shape=box, fontname=\"helvetica\", fontsize=12];\n\n";

        for (auto i : range(first_free_index_)) {
            const auto name  = operator[](i).name();
            const auto dirty = bool(operator[](i).dirty_bits());

            out += std::format("    \"node{}\" [label=\"id: {} type: {} dirty: {} \", style=filled,color=\"{}\"];\n",
                               i,
                               i,
                               name,
                               dirty,
                               colorize_node(name));
        }

        for (auto i : range(first_free_index_)) {
            if (operator[](i).first_child() == tree_nodeClass::invalid_index) continue;

            for (auto current = operator[](i).first_child(); current != tree_nodeClass::invalid_index;
                 current      = operator[](current).next_sibling()) {
                out += std::format("    \"node{}\" -> \"node{}\" [color=seagreen] ;\n", i, current);
            }
        }

        out += "}";

        return io::writefile(filepath, out);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename tree_nodeClass>
    auto tree<tree_nodeClass>::gen_dot_file(stdfs::path                                   filepath,
                                            core::u32                                     highlight,
                                            std23::function_ref<string_view(string_view)> colorize_node) const noexcept
      -> system_result<usize> {
        using namespace stormkit::literals;
        auto out = string {};
        out.reserve(1_kb);

        out += "digraph G { \n"
               "    rankdir = LR\n"
               "    bgcolor = black\n\n"
               "    node [shape=box, fontname=\"helvetica\", fontsize=12];\n\n";

        for (auto i : range(first_free_index_)) {
            const auto name  = operator[](i).name();
            const auto dirty = bool(operator[](i).dirty_bits());
            if (i != highlight)
                out += std::format("    \"node{}\" [label=\"id: {} type: {} dirty: {} \", style=filled,color=\"{}\"];\n",
                                   i,
                                   i,
                                   name,
                                   dirty,
                                   colorize_node(name));
            else
                out += std::format("    \"node{}\" [shape = polygon,sides=5,peripheries=3, label=\"id: {} type: {} dirty: {} \", "
                                   "style=filled,color=\"{}\"];\n",
                                   i,
                                   i,
                                   name,
                                   dirty,
                                   colorize_node(name));
        }

        for (auto i : range(first_free_index_)) {
            if (operator[](i).first_child() == tree_nodeClass::invalid_index) continue;

            for (auto current = operator[](i).first_child(); current != tree_nodeClass::invalid_index;
                 current      = operator[](current).next_sibling()) {
                out += std::format("    \"node{}\" -> \"node{}\" [color=seagreen] ;\n", i, current);
            }
        }

        out += "}";

        return io::writefile(filepath, out);
    }
}} // namespace stormkit::core
