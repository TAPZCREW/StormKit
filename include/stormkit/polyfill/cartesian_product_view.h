#ifndef CARTESIAN_PRODUCT_VIEW_H
#define CARTESIAN_PRODUCT_VIEW_H

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <ranges>
#include <tuple>
#include <type_traits>
#include <utility>

template <bool deep_const, typename... Views>
class cartesian_product_view final : public std::ranges::view_base {
    static_assert((... && (std::ranges::forward_range<Views> && std::ranges::view<Views>)));

    template <typename T>
    struct arrow_proxy {
        T t;

        T operator*() && {
            return t;
        }

        T* operator->() {
            return &t;
        }
    };

    template <bool propagate_const, std::size_t index, std::size_t size, typename ViewsTuple>
    class cartesian_iterator : protected cartesian_iterator<propagate_const, index + 1, size, ViewsTuple> {
        using base = cartesian_iterator<propagate_const, index + 1, size, ViewsTuple>;
        using aux = cartesian_iterator<propagate_const, size, size, ViewsTuple>;

    public:
        using aux::iterator_category;
        using aux::iterator_concept;
        using aux::difference_type;
        using aux::value_type;
        using aux::reference;
        using aux::pointer;

        cartesian_iterator() = default;

        cartesian_iterator(bool const begin_end, ViewsTuple* views)
          : base{ begin_end, views }
          , it{
                !begin_end
              ? std::ranges::begin(std::get<index>(*views))
              : [&] {
                    auto& views = std::get<index>(*aux::views);
                    if constexpr (std::ranges::common_range<std::tuple_element_t<index, ViewsTuple>>) {
                        return std::ranges::end(views);
                    } else {
                        return std::ranges::next(std::ranges::begin(views), std::ranges::end(views));
                    }
                }()
            }
        {}

        cartesian_iterator& operator++() {
            assert(!empty() && "Attempting to increment empty cartesian_product_view<> iterator!");
            assert(it != std::ranges::end(std::get<index>(*aux::views)) && "Attempting to increment end cartesian_product_view<> iterator!");

            static_cast<void>(next());
            return *this;
        }

        cartesian_iterator operator++(int) {
            auto ret = *this;
            ++*this;
            return ret;
        }

        typename aux::reference operator*() const {
            assert(!empty() && "Attempting to dereference empty cartesian_product_view<> iterator!");
            assert(it != std::ranges::end(std::get<index>(*aux::views)) && "Attempting to dereference end cartesian_product_view<> iterator!");

            return read();
        }

        typename aux::pointer operator->() const {
            return { **this };
        }

        bool operator==(cartesian_iterator const& other) const {
#ifndef NDEBUG
            return base::operator==(other) && it == other.it;
#else
            return it == other.it && base::operator==(other);
#endif
        }

        cartesian_iterator& operator--() requires (... && std::ranges::bidirectional_range<Views>) {
            assert(!empty() && "Attempting to decrement empty cartesian_product_view<> iterator!");

            if (it == std::ranges::end(std::get<index>(*aux::views))) {
                reset_rbegin();
            } else {
                auto const underflow = prev();
                assert(!underflow && "Attempting to decrement begin cartesian_product_view<> iterator!");
            }

            return *this;
        }

        cartesian_iterator operator--(int) requires (... && std::ranges::bidirectional_range<Views>) {
            auto ret = *this;
            --*this;
            return ret;
        }

    protected:
        bool next() {
            if (base::next()){
                if (++it != std::ranges::end(std::get<index>(*aux::views))) {
                    base::reset_begin();
                } else {
                    return true;
                }
            }

            return false;
        }

        bool prev() requires (... && std::ranges::bidirectional_range<Views>) {
            if (base::prev()) {
                if (it != std::ranges::begin(std::get<index>(*aux::views))) {
                    --it;
                    base::reset_rbegin();
                } else {
                    return true;
                }
            }

            return false;
        }

        auto read() const {
            return std::tuple_cat(std::tuple<std::tuple_element_t<index, typename aux::reference>>{ *it }, base::read());
        }

        void reset_begin() {
            base::reset_begin();
            it = std::ranges::begin(std::get<index>(*aux::views));
        }

        void reset_rbegin() requires (... && std::ranges::bidirectional_range<Views>) {
            base::reset_rbegin();
            it =
                [&] {
                    auto& views = std::get<index>(*aux::views);
                    if constexpr (std::ranges::common_range<std::tuple_element_t<index, ViewsTuple>>) {
                        return std::ranges::prev(std::ranges::end(views));
                    } else {
                        return std::ranges::prev(std::ranges::next(std::ranges::begin(views), std::ranges::end(views)));
                    }
                }()
            ;
        }

#ifndef NDEBUG
        bool empty() const {
            return std::ranges::empty(std::get<index>(*aux::views)) || base::empty();
        }
#endif

    private:
        std::ranges::iterator_t<std::tuple_element_t<index, ViewsTuple>> it;
    };

    template <bool propagate_const, std::size_t size, typename ViewsTuple>
    class cartesian_iterator<propagate_const, size, size, ViewsTuple> {
        class iterator_traits {
            template <std::ranges::range Range>
            class element {
                using reference = std::ranges::range_reference_t<Range>;

            public:
                using type =
                    std::conditional_t<
                        propagate_const && std::is_reference_v<reference>,
                        std::conditional_t<
                            std::is_lvalue_reference_v<reference>,
                            std::remove_cvref_t<reference> const&,
                            std::remove_cvref_t<reference> const&&
                        >,
                        reference
                    >
                ;
            };

            template <std::ranges::range Range>
            using element_t = typename element<Range>::type;

            template <typename View>
            using maybe_const = std::conditional_t<std::is_const_v<ViewsTuple>, View const, View>;

        public:
            using iterator_category = std::input_iterator_tag;

            using iterator_concept =
                std::conditional_t<
                    (... && std::ranges::bidirectional_range<maybe_const<Views>>),
                    std::bidirectional_iterator_tag,
                    std::forward_iterator_tag
                >
            ;

            using difference_type =
                std::common_type_t<
                    std::intmax_t,
                    std::ranges::range_difference_t<maybe_const<Views>>...
                >
            ;

            using value_type = std::tuple<std::ranges::range_value_t<maybe_const<Views>>...>;

            using reference = std::tuple<element_t<maybe_const<Views>>...>;

            using pointer = arrow_proxy<reference>;
        };

    public:
        using iterator_category = typename iterator_traits::iterator_category;
        using iterator_concept  = typename iterator_traits::iterator_concept;
        using difference_type   = typename iterator_traits::difference_type;
        using value_type        = typename iterator_traits::value_type;
        using reference         = typename iterator_traits::reference;
        using pointer           = typename iterator_traits::pointer;

        cartesian_iterator() = default;

        cartesian_iterator(bool, ViewsTuple* views)
            : views{ views }
        {}

        cartesian_iterator& operator++() {
            assert(!"Attempting to increment empty cartesian_product_view<> iterator!");

            return *this;
        }

        cartesian_iterator operator++(int) {
            auto ret = *this;
            ++*this;
            return ret;
        }

        auto operator*() const {
            assert(!"Attempting to dereference empty cartesian_product_view<> iterator!");

            return std::tuple{};
        }

        arrow_proxy<std::tuple<>> operator->() const {
            return { **this };
        }

        bool operator==(cartesian_iterator const& other) const {
            assert(views == other.views && "Attempting to compare incompatible cartesian_product_view<> iterators!");

            return true;
        }

        cartesian_iterator& operator--() {
            assert(!"Attempting to decrement empty cartesian_product_view<> iterator!");

            return *this;
        }

        cartesian_iterator operator--(int) {
            auto ret = *this;
            --*this;
            return ret;
        }

    protected:
        bool prev() const {
            return true;
        }

        bool next() const {
            return true;
        }

        auto read() const {
            return std::tuple{};
        }

        void reset_begin() const {}

        void reset_rbegin() const {}

#ifndef NDEBUG
        bool empty() const {
            return false; // despite seeming to be controversial
        }
#endif

        ViewsTuple* views;
    };

public:
    cartesian_product_view() = default;

    cartesian_product_view(Views... views) requires (sizeof...(Views) != 0)
        : views{ std::move(views)... }
    {}

    auto begin() {
        return iterator(false, views);
    }

    auto end() {
        return iterator(true, views);
    }

    auto begin() const {
        return iterator(false, views);
    }

    auto end() const {
        return iterator(true, views);
    }

    auto cbegin() const {
        return begin();
    }

    auto cend() const {
        return end();
    }

    auto size() requires (... && std::ranges::sized_range<Views>) {
        return size(views);
    }

    auto size() const requires (... && std::ranges::sized_range<Views const>) {
        return size(views);
    }

private:
    template <typename T>
    static auto iterator(bool const begin_end, T& views) {
        return
            cartesian_iterator<
                deep_const && std::is_const_v<T>,
                0,
                sizeof...(Views),
                std::remove_reference_t<decltype(views)>
            > {
                begin_end
             || std::apply(
                    [](auto&... views) {
                        return (... || std::ranges::empty(views));
                    },
                    views
                ),
                &views
            }
        ;
    }

    template <typename T>
    static auto size(T& views) {
        return
            std::apply(
                [](auto&... views) {
                    return (std::size_t{ sizeof...(views) != 0 } * ... * std::ranges::size(views));
                },
                views
            )
        ;
    }

    std::tuple<Views...> views;
};

template <typename... Ranges>
cartesian_product_view(Ranges&&...) -> cartesian_product_view<true, std::views::all_t<Ranges>...>;

template <bool deep_const>
struct cartesian_product_fn {
    template <typename... Ranges>
    requires (... && (std::ranges::forward_range<Ranges> && std::ranges::viewable_range<Ranges>))
    constexpr auto operator()(Ranges&&... ranges) const {
        return cartesian_product_view<deep_const, std::views::all_t<Ranges>...>{ std::views::all(std::forward<Ranges>(ranges))... };
    }
};

inline constexpr cartesian_product_fn<true> cartesian_product{};

#endif