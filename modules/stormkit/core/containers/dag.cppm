// Copyright (C) 2024 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

module;

#include <stormkit/core/platform_macro.hpp>

export module stormkit.core.containers.dag;

import std;

import stormkit.core.types;

import stormkit.core.typesafe.ref_ptr;
import stormkit.core.contract;
import stormkit.core.function_ref;
import stormkit.core.string.format;
import stormkit.core.errors;
import stormkit.core.meta.concepts;
import stormkit.core.meta.type_manipulation;
import stormkit.core.meta.type_query;

namespace stdr = std::ranges;

export namespace stormkit { inline namespace core {
    template<typename T>
    class dag {
      public:
        using value_type = T;
        using index_type = u32;

        struct edge {
            index_type from;
            index_type to;
        };

        struct vertex {
            index_type id;
            value_type value;
        };

        using colorize_closure     = std23::move_only_function<string(const value_type&)>;
        using format_value_closure = std23::move_only_function<string(const value_type&)>;

        struct closures {
            std::optional<colorize_closure>     colorize     = std::nullopt;
            std::optional<format_value_closure> format_value = []() static noexcept -> std::optional<format_value_closure> {
                if constexpr (std::formattable<value_type, char>)
                    return [](const auto& value) static noexcept { return std::format("{}", value); };
                else
                    return std::nullopt;
            }();
        };

        constexpr explicit dag(std::optional<usize> reserve = std::nullopt) noexcept;
        constexpr ~dag() noexcept;

        constexpr dag(dag&&) noexcept;
        constexpr dag(const dag&) noexcept = delete;

        constexpr auto operator=(dag&&) noexcept -> dag&;
        constexpr auto operator=(const dag&) noexcept -> dag& = delete;

        constexpr auto directed() const noexcept -> bool;

        constexpr auto clear() noexcept(meta::noexcept_destructible<value_type>) -> void;
        constexpr auto empty() const noexcept -> bool;

        constexpr auto add_vertex(meta::in<value_type> value) noexcept(meta::noexcept_copyable<value_type>) -> index_type
            requires(meta::copyable<value_type>);
        constexpr auto add_vertex(value_type&& value) noexcept(meta::noexcept_movable<value_type>) -> index_type
            requires(meta::movable<value_type> and not meta::prefer_pass_by_value<value_type>);
        template<typename... Ts>
        constexpr auto emplace_vertex(Ts&&... args) noexcept(meta::noexcept_constructible_from<value_type, Ts...>) -> index_type
            requires(meta::constructible_from<value_type, Ts...>);
        template<typename Self>
        constexpr auto get_vertex_value(this Self&& self, index_type id) noexcept -> meta::forward_like<Self, value_type>;
        constexpr auto has_vertex(meta::in<value_type> value) const noexcept -> bool
            requires(meta::has_equality_operator<value_type, value_type>);
        constexpr auto has_vertex(index_type vert) const noexcept -> bool;
        constexpr auto remove_vertex(meta::in<value_type> value) noexcept(meta::noexcept_destructible<value_type>) -> void
            requires(meta::has_equality_operator<value_type, value_type>);
        constexpr auto remove_vertex(index_type id) noexcept(meta::noexcept_destructible<value_type>) -> void;

        constexpr auto add_edge(index_type from, index_type to) noexcept -> void;
        constexpr auto has_edge(index_type from, index_type to) const noexcept -> bool;
        constexpr auto remove_edge(index_type from, index_type to) noexcept -> void;
        constexpr auto adjacent_edges(index_type vert) const noexcept -> const dynarray<edge>&;

        constexpr auto vertices() const noexcept -> const dynarray<vertex>&;
        constexpr auto vertices_count() const noexcept -> usize;

        constexpr auto edges() const noexcept -> const dynarray<edge>&;
        constexpr auto edges_count() const noexcept -> usize;

        constexpr auto topological_sort() const noexcept -> expected<dynarray<index_type>, dynarray<index_type>>;
        constexpr auto find_cycle() const noexcept -> std::optional<dynarray<index_type>>;

        constexpr auto reverse_view() const noexcept -> dag<ref_ptr<const value_type>>;
        constexpr auto reverse_clone() const noexcept -> dag<value_type>;

        constexpr auto dump(closures closures_ = {}) const noexcept -> string;

        // FIXME find a way to make it not accessible to user
        template<typename FromDag, bool AS_REF>
        static constexpr auto reverse_from(index_type,
                                           array_view<const typename FromDag::vertex>,
                                           array_view<const typename FromDag::edge>) noexcept -> dag<value_type>;

      private:
        index_type m_next_id = 0;

        dynarray<vertex>                                m_vertices;
        dynarray<edge>                                  m_edges;
        dynarray<std::pair<index_type, dynarray<edge>>> m_adjacent_edges;
    };

    template<typename CharT, typename FormatContext, typename T>
    [[nodiscard]]
    // constexpr auto tag_invoke(format_as_fn<CharT>, meta::in<typename dag<T>::edge> value, FormatContext& ctx) noexcept
    constexpr auto tag_invoke(format_as_fn<CharT>, typename dag<T>::edge value, FormatContext& ctx) noexcept
      -> decltype(ctx.out());

    template<typename CharT, typename FormatContext, typename T>
    [[nodiscard]]
    // constexpr auto tag_invoke(format_as_fn<CharT>, meta::in<typename dag<T>::vertex> value, FormatContext& ctx) noexcept
    constexpr auto tag_invoke(format_as_fn<CharT>, typename dag<T>::vertex value, FormatContext& ctx) noexcept
      -> decltype(ctx.out());
}} // namespace stormkit::core

////////////////////////////////////////////////////////////////////
///                      IMPLEMENTATION                          ///
////////////////////////////////////////////////////////////////////

namespace stormkit { inline namespace core {
    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename T>
    constexpr dag<T>::dag(std::optional<usize> reserve) noexcept {
        if (reserve) m_vertices.reserve(*reserve);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename T>
    constexpr dag<T>::~dag() noexcept = default;

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename T>
    constexpr dag<T>::dag(dag&&) noexcept = default;

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename T>
    constexpr auto dag<T>::operator=(dag&&) noexcept -> dag& = default;

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename T>
    constexpr auto dag<T>::clear() noexcept(meta::noexcept_destructible<value_type>) -> void {
        m_adjacent_edges.clear();
        m_edges.clear();
        m_vertices.clear();
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename T>
    STORMKIT_FORCE_INLINE
    constexpr auto dag<T>::empty() const noexcept -> bool {
        return stdr::empty(m_vertices);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename T>
    constexpr auto dag<T>::add_vertex(meta::in<value_type> value) noexcept(meta::noexcept_copyable<value_type>) -> index_type
        requires(meta::copyable<value_type>)
    {
        const auto id = m_next_id++;
        m_vertices.emplace_back(id, value);
        m_adjacent_edges.emplace_back(id, dynarray<edge> {});
        return id;
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename T>
    constexpr auto dag<T>::add_vertex(value_type&& value) noexcept(meta::noexcept_movable<value_type>) -> index_type
        requires(meta::movable<value_type> and not meta::prefer_pass_by_value<value_type>)
    {
        const auto id = m_next_id++;
        m_vertices.emplace_back(id, std::move(value));
        m_adjacent_edges.emplace_back(id, dynarray<edge> {});
        return id;
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename T>
    template<typename... Ts>
    constexpr auto dag<T>::emplace_vertex(Ts&&... args) noexcept(meta::noexcept_constructible_from<value_type, Ts...>)
      -> index_type
        requires(meta::constructible_from<value_type, Ts...>)
    {
        return add_vertex(T { std::forward<Ts>(args)... });
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename T>
    template<typename Self>
    constexpr auto dag<T>::get_vertex_value(this Self&& self, index_type id) noexcept -> meta::forward_like<Self, value_type> {
        expects(self.has_vertex(id), std::format("Unknown dag vertex id: {}!", id));

        return std::forward_like<Self>(stdr::find_if(self.m_vertices, [id](const auto& vert) noexcept {
                                           return vert.id == id;
                                       })->value);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename T>
    STORMKIT_FORCE_INLINE
    constexpr auto dag<T>::has_vertex(meta::in<value_type> value) const noexcept -> bool
        requires(meta::has_equality_operator<value_type, value_type>)
    {
        return stdr::any_of(m_vertices, [&value](const auto& vert) noexcept { return value == vert.value; });
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename T>
    STORMKIT_FORCE_INLINE
    constexpr auto dag<T>::has_vertex(index_type id) const noexcept -> bool {
        return stdr::any_of(m_vertices, [&id](const auto& vert) noexcept { return vert.id == id; });
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename T>
    constexpr auto dag<T>::remove_vertex(meta::in<value_type> value) noexcept(meta::noexcept_destructible<value_type>) -> void
        requires(meta::has_equality_operator<value_type, value_type>)
    {
        if constexpr (std::formattable<value_type, char>) expects(has_vertex(value), "Unknown dag vertex value: {}!", value);
        else
            expects(has_vertex(value), "Unknown dag vertex value!");

        auto       it = stdr::find_if(m_vertices, [&other = value](const auto& vert) noexcept { return vert.value == other; });
        const auto id = it->id;

        remove_vertex(id);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename T>
    constexpr auto dag<T>::remove_vertex(index_type id) noexcept(meta::noexcept_destructible<value_type>) -> void {
        expects(has_vertex(id), std::format("Unknown dag vertex id: {}!", id));

        auto touching = dynarray<edge> {};
        for (const auto& edge : m_edges)
            if (edge.from == id or edge.to == id) touching.emplace_back(edge);

        for (const auto& [from, to] : touching) remove_edge(from, to);

        auto&& [begin, end]   = stdr::remove_if(m_vertices, [id](const auto& vert) noexcept { return vert.id == id; });
        auto&& [begin2, end2] = stdr::remove_if(m_adjacent_edges, [id](const auto& pair) noexcept { return pair.first == id; });

        m_adjacent_edges.erase(begin2, end2);
        m_vertices.erase(begin, end);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename T>
    constexpr auto dag<T>::add_edge(index_type from, index_type to) noexcept -> void {
        expects(has_vertex(from), std::format("Unknown dag vertex from: {}", from));
        expects(has_vertex(to), std::format("Unknown dag vertex to: {}", to));
        if (has_edge(from, to)) return;

        const auto& edge = m_edges.emplace_back(from, to);

        auto& adjacent_edges = stdr::find_if(m_adjacent_edges, [id = from](const auto& pair) noexcept {
                                   return pair.first == id;
                               })->second;
        adjacent_edges.emplace_back(edge);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename T>
    constexpr auto dag<T>::has_edge(index_type from, index_type to) const noexcept -> bool {
        if (not has_vertex(from) or not has_vertex(to)) return false;

        return stdr::any_of(m_edges, [from, to](const auto& edge) noexcept { return edge.from == from and edge.to == to; });
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename T>
    constexpr auto dag<T>::remove_edge(index_type from, index_type to) noexcept -> void {
        expects(has_vertex(from), std::format("Unknown dag vertex from: {}", from));
        expects(has_vertex(to), std::format("Unknown dag vertex to: {}", to));

        if (not has_edge(from, to)) return;

        {
            auto&& [begin, end] = stdr::remove_if(m_edges, [from, to](const auto& edge) noexcept {
                return edge.from == from and edge.to == to;
            });

            m_edges.erase(begin, end);
        }
        for (auto& [_, edges] : m_adjacent_edges) {
            auto&& [begin, end] = stdr::remove_if(edges, [from, to](const auto& edge) noexcept {
                return edge.from == from and edge.to == to;
            });
            edges.erase(begin, end);
        }
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename T>
    constexpr auto dag<T>::adjacent_edges(index_type id) const noexcept -> const dynarray<edge>& {
        expects(has_vertex(id), std::format("Unknown dag vertex id: {}!", id));

        const auto& adjacent_edges = stdr::find_if(m_adjacent_edges, [id](const auto& pair) noexcept {
                                         return pair.first == id;
                                     })->second;
        return adjacent_edges;
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename T>
    STORMKIT_FORCE_INLINE
    constexpr auto dag<T>::vertices() const noexcept -> const dynarray<vertex>& {
        return m_vertices;
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename T>
    STORMKIT_FORCE_INLINE
    constexpr auto dag<T>::vertices_count() const noexcept -> usize {
        return stdr::size(m_vertices);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename T>
    STORMKIT_FORCE_INLINE
    constexpr auto dag<T>::edges() const noexcept -> const dynarray<edge>& {
        return m_edges;
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename T>
    STORMKIT_FORCE_INLINE
    constexpr auto dag<T>::edges_count() const noexcept -> usize {
        return stdr::size(m_edges);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename T>
    constexpr auto dag<T>::topological_sort() const noexcept -> expected<dynarray<index_type>, dynarray<index_type>> {
        if (auto result = find_cycle(); result.has_value()) return std::unexpected { std::move(*result) };

        struct degree {
            index_type id;
            u32        d = 0;
        };

        auto in_degree = dynarray<degree> {};
        in_degree.reserve(stdr::size(m_vertices));
        for (const auto& [id, _] : m_vertices) { in_degree.emplace_back(id, 0); }

        for (const auto& [id, _] : m_vertices) {
            const auto& edges = adjacent_edges(id);

            if (not stdr::empty(edges)) {
                for (const auto& [from, to] : edges) {
                    if (from != id) continue;
                    auto&& [_, d] = *stdr::find_if(in_degree, [to](auto&& other) noexcept { return other.id == to; });
                    d += 1;
                }
            }
        }

        auto queue = std::queue<index_type> {};

        for (const auto& [id, _] : m_vertices) {
            auto&& [_, d] = *stdr::find_if(in_degree, [id](auto&& other) noexcept { return other.id == id; });
            if (d == 0) queue.push(id);
        }

        auto ordered_vertices = dynarray<index_type> {};
        ordered_vertices.reserve(stdr::size(m_vertices));
        while (not stdr::empty(queue)) {
            auto id = queue.front();
            queue.pop();

            ordered_vertices.emplace_back(id);

            const auto& edges = adjacent_edges(id);
            if (not stdr::empty(edges)) {
                for (const auto& [from, to] : edges) {
                    if (from == id) {
                        auto&& [_, d] = *stdr::find_if(in_degree, [to](auto&& other) noexcept { return other.id == to; });
                        if (d > 0) d -= 1;
                        if (d == 0) queue.push(to);
                    }
                }
            }
        }

        return ordered_vertices;
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename T>
    constexpr auto dag<T>::find_cycle() const noexcept -> std::optional<dynarray<index_type>> {
        auto out = std::optional<dynarray<index_type>> { std::nullopt };

        auto visited = dynarray<index_type> {};
        visited.reserve(stdr::size(m_vertices));
        auto stack = dynarray<index_type> {};

        auto dfs = [&visited, &stack, &out, this](auto&& dfs, auto&& id) mutable noexcept -> bool {
            visited.emplace_back(id);

            const auto& edges = adjacent_edges(id);
            if (stdr::empty(edges)) return false;

            stack.emplace_back(id);

            for (const auto& [from, to] : edges) {
                const auto w = (from == id) ? to : from;
                if (not stdr::contains(visited, w)) {
                    if (dfs(dfs, w)) {
                        return true;
                    } else if (auto it = stdr::find(stack, w); it != stdr::cend(stack)) {
                        auto cycle = dynarray<index_type>(it, stdr::end(stack));
                        cycle.emplace_back(w);
                        out = std::move(cycle);
                        return true;
                    }
                } else if (auto it = stdr::find(stack, w); it != stdr::cend(stack)) {
                    auto cycle = dynarray<index_type>(it, stdr::end(stack));
                    cycle.emplace_back(w);
                    out = std::move(cycle);
                    return true;
                }
            }

            auto [begin, end] = stdr::remove(stack, id);
            stack.erase(begin, end);
            return false;
        };

        for (const auto& [id, _] : m_vertices) {
            if (stdr::contains(visited, id)) continue;

            if (dfs(dfs, id)) break;
        }

        return out;
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename T>
    constexpr auto dag<T>::reverse_view() const noexcept -> dag<ref_ptr<const value_type>> {
        return dag<ref_ptr<const value_type>>::template reverse_from<dag<value_type>, true>(m_next_id, m_vertices, m_edges);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename T>
    constexpr auto dag<T>::reverse_clone() const noexcept -> dag<value_type> {
        return reverse_from<dag<value_type>, false>(m_next_id, m_vertices, m_edges);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename T>
    constexpr auto dag<T>::dump(closures closures_) const noexcept -> string {
        auto out = string { "digraph G {\n"
                            "    rankdir = LR\n"
                            "    bgcolor = black\n"
                            "    node [shape=box, fontname=\"helvetica\", fontsize=12];\n\n" };

        if (closures_.format_value)
            for (const auto& [id, value] : m_vertices) {
                out += std::format("    \"node{}\" [label=\"id: {} value: {}\", style=filled,color=\"{}\"];\n",
                                   id,
                                   id,
                                   closures_.format_value.value()(value),
                                   closures_.colorize ? closures_.colorize.value()(value) : "white");
            }
        else
            for (const auto& [id, value] : m_vertices) {
                out += std::format("    \"node{}\" [label=\"id: {}\", style=filled,color=\"{}\"];\n",
                                   id,
                                   id,
                                   closures_.colorize ? closures_.colorize.value()(value) : "white");
            }

        for (const auto& [from, to] : m_edges) out += std::format("    \"node{}\" -> \"node{}\" [color=seagreen];\n", from, to);

        out += "}";

        return out;
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename T>
    template<typename FromDag, bool AS_REF>
    constexpr auto dag<T>::reverse_from(index_type                                 next_id,
                                        array_view<const typename FromDag::vertex> vertices,
                                        array_view<const typename FromDag::edge>   edges) noexcept -> dag<value_type> {
        auto out      = dag<value_type> {};
        out.m_next_id = next_id;

        out.m_adjacent_edges.reserve(stdr::size(vertices));
        if constexpr (AS_REF) {
            for (const auto& [id, vertice] : vertices) {
                out.m_vertices.emplace_back(id, vertice);
                out.m_adjacent_edges.emplace_back(id, dynarray<edge> {});
            }
        } else {
            for (const auto& [id, vertice] : vertices) {
                out.m_vertices.emplace_back(id, auto(vertice));
                out.m_adjacent_edges.emplace_back(id, dynarray<edge> {});
            }
        }

        out.m_edges.reserve(stdr::size(edges));
        for (const auto& [from, to] : edges) {
            const auto& edge = out.m_edges.emplace_back(to, from);

            auto& adjacent_edges = stdr::find_if(out.m_adjacent_edges, [id = to](const auto& pair) noexcept {
                                       return pair.first == id;
                                   })->second;
            adjacent_edges.emplace_back(edge);
        }

        return out;
    }

    /////////////////////////////////////
    /////////////////////////////////////
    template<typename CharT, typename FormatContext, typename T>
    STORMKIT_FORCE_INLINE
    // constexpr auto tag_invoke(format_as_fn<CharT>, meta::in<typename dag<T>::edge> edge, FormatContext& ctx) noexcept
    constexpr auto tag_invoke(format_as_fn<CharT>, typename dag<T>::edge edge, FormatContext& ctx) noexcept
      -> decltype(ctx.out()) {
        return std::format_to(ctx.out(), "[dag_edge from: {}, to: {}]", edge.from, edge.to);
    }

    ////////////////////////////////////////
    ////////////////////////////////////////
    template<typename CharT, typename FormatContext, typename T>
    STORMKIT_FORCE_INLINE
    // constexpr auto tag_invoke(format_as_fn<CharT>, meta::in<typename dag<T>::vertex> vertex, FormatContext& ctx) noexcept
    constexpr auto tag_invoke(format_as_fn<CharT>, typename dag<T>::vertex vertex, FormatContext& ctx) noexcept
      -> decltype(ctx.out()) {
        if constexpr (std::formattable<T, char>)
            return std::format_to(ctx.out(), "[dag_vertex id: {}, value: {}]", vertex.id, vertex.value);
        else
            return std::format_to(ctx.out(), "[dag_vertex id: {}, value: <no formatter>]", vertex.id);
    }
}} // namespace stormkit::core
