module;

#include <cassert>
#include <version>

#include <stormkit/core/platform_macro.hpp>

export module stormkit.core.coroutines;

#if defined(__cpp_lib_generator) and __cpp_lib_generator < 202207L
export import std;
#else
import std;
#endif

#if not defined(__cpp_lib_generator) or __cpp_lib_generator < 202207L
export namespace std {
    struct use_allocator_arg {};

    template<class _Ref, typename _Value = remove_cvref_t<_Ref>, typename _Alloc = use_allocator_arg>
    class generator;

    template<class _T>
    class __manual_lifetime {
      public:
        __manual_lifetime() noexcept {}

        ~__manual_lifetime() {}

        template<class... _Ts>
        _T& construct(_Ts&&... __args) noexcept(is_nothrow_constructible_v<_T, _Ts...>) {
            return *::new (static_cast<void*>(addressof(__value_))) _T((_Ts&&)__args...);
        }

        void destruct() noexcept(is_nothrow_destructible_v<_T>) { __value_.~_T(); }

        _T& get() & noexcept { return __value_; }

        _T&& get() && noexcept { return static_cast<_T&&>(__value_); }

        const _T& get() const & noexcept { return __value_; }

        const _T&& get() const && noexcept { return static_cast<const _T&&>(__value_); }

      private:
        union {
            remove_const_t<_T> __value_;
        };
    };

    template<class _T>
    class __manual_lifetime<_T&> {
      public:
        __manual_lifetime() noexcept : __value_(nullptr) {}

        ~__manual_lifetime() {}

        _T& construct(_T& __value) noexcept {
            __value_ = addressof(__value);
            return __value;
        }

        void destruct() noexcept {}

        _T& get() const noexcept { return *__value_; }

      private:
        _T* __value_;
    };

    template<class _T>
    class __manual_lifetime<_T&&> {
      public:
        __manual_lifetime() noexcept : __value_(nullptr) {}

        ~__manual_lifetime() {}

        _T&& construct(_T&& __value) noexcept {
            __value_ = addressof(__value);
            return static_cast<_T&&>(__value);
        }

        void destruct() noexcept {}

        _T&& get() const noexcept { return static_cast<_T&&>(*__value_); }

      private:
        _T* __value_;
    };

    #if defined(__clang__) and (__clang_major__ < 22)
    namespace ranges {
        template<class _Rng, typename _Allocator = use_allocator_arg>
        struct elements_of {
            explicit constexpr elements_of(_Rng&& __rng) noexcept
                requires std::is_default_constructible_v<_Allocator>
                : __range(static_cast<_Rng&&>(__rng)) {}

            constexpr elements_of(_Rng&& __rng, _Allocator&& __alloc) noexcept
                : __range((_Rng&&)__rng), __alloc((_Allocator&&)__alloc) {}

            constexpr elements_of(elements_of&&) noexcept = default;

            constexpr elements_of(const elements_of&)            = delete;
            constexpr elements_of& operator=(const elements_of&) = delete;
            constexpr elements_of& operator=(elements_of&&)      = delete;

            constexpr _Rng&& get() noexcept { return static_cast<_Rng&&>(__range); }

            constexpr _Allocator get_allocator() const noexcept { return __alloc; }

          private:
            [[no_unique_address]]
            _Allocator __alloc; // \expos
            _Rng&&     __range; // \expos
        };

        template<class _Rng>
        elements_of(_Rng&&) -> elements_of<_Rng>;

        template<class _Rng, typename Allocator>
        elements_of(_Rng&&, Allocator&&) -> elements_of<_Rng, Allocator>;
    } // namespace ranges
    #endif

    template<class _Alloc>
    inline constexpr bool __allocator_needs_to_be_stored = !allocator_traits<_Alloc>::is_always_equal::value
                                                           || !is_default_constructible_v<_Alloc>;

    // Round s up to next multiple of a.
    constexpr size_t __aligned_allocation_size(size_t s, size_t a) {
        return (s + a - 1) & ~(a - 1);
    }

    template<class _Alloc>
    class __promise_base_alloc {
        static constexpr size_t __offset_of_allocator(size_t __frameSize) noexcept {
            return __aligned_allocation_size(__frameSize, alignof(_Alloc));
        }

        static constexpr size_t __padded_frame_size(size_t __frameSize) noexcept {
            return __offset_of_allocator(__frameSize) + sizeof(_Alloc);
        }

        static _Alloc& __get_allocator(void* __frame, size_t __frameSize) noexcept {
            return *reinterpret_cast<_Alloc*>(static_cast<char*>(__frame) + __offset_of_allocator(__frameSize));
        }

      public:
        template<class... _Ts>
        static void* operator new(size_t __frameSize, allocator_arg_t, _Alloc __alloc, _Ts&...) {
            void* __frame = __alloc.allocate(__padded_frame_size(__frameSize));

            // Store allocator at end of the coroutine frame.
            // Assuming the allocator's move constructor is non-throwing (a requirement for
            // allocators)
            ::new (static_cast<void*>(addressof(__get_allocator(__frame, __frameSize)))) _Alloc(move(__alloc));

            return __frame;
        }

        template<class _This, typename... _Ts>
        static void* operator new(size_t __frameSize, _This&, allocator_arg_t, _Alloc __alloc, _Ts&...) {
            return __promise_base_alloc::operator new(__frameSize, allocator_arg, move(__alloc));
        }

        static void operator delete(void* __ptr, size_t __frameSize) noexcept {
            _Alloc& __alloc = __get_allocator(__ptr, __frameSize);
            _Alloc  __localAlloc(move(__alloc));
            __alloc.~Alloc();
            __localAlloc.deallocate(static_cast<byte*>(__ptr), __padded_frame_size(__frameSize));
        }
    };

    template<class _Alloc>
        requires(!__allocator_needs_to_be_stored<_Alloc>)
    class __promise_base_alloc<_Alloc> {
      public:
        static void* operator new(size_t __size) {
            _Alloc __alloc;
            return __alloc.allocate(__size);
        }

        static void operator delete(void* __ptr, size_t __size) noexcept {
            _Alloc __alloc;
            __alloc.deallocate(static_cast<byte*>(__ptr), __size);
        }
    };

    template<class _Ref>
    struct __generator_promise_base {
        template<class _Ref2, typename _Value, typename _Alloc>
        friend class generator;

        __generator_promise_base* __root_;
        coroutine_handle<>        __parentOrLeaf_;
        // Note: Using manual_lifetime here to avoid extra calls to exception_ptr
        // constructor/destructor in cases where it is not needed (i.e. where this
        // generator coroutine is not used as a nested coroutine).
        // This member is lazily constructed by the __yield_sequence_awaiter::await_suspend()
        // method if this generator is used as a nested generator.
        __manual_lifetime<exception_ptr> __exception_;
        __manual_lifetime<_Ref>          __value_;

        explicit __generator_promise_base(coroutine_handle<> thisCoro) noexcept : __root_(this), __parentOrLeaf_(thisCoro) {}

        ~__generator_promise_base() {
            if (__root_ != this) {
                // This coroutine was used as a nested generator and so will
                // have constructed its __exception_ member which needs to be
                // destroyed here.
                __exception_.destruct();
            }
        }

        suspend_always initial_suspend() noexcept { return {}; }

        void return_void() noexcept {}

        void unhandled_exception() {
            if (__root_ != this) {
                __exception_.get() = current_exception();
            } else {
                throw;
            }
        }

        // Transfers control back to the parent of a nested coroutine
        struct __final_awaiter {
            bool await_ready() noexcept { return false; }

            template<class _Promise>
            coroutine_handle<> await_suspend(coroutine_handle<_Promise> __h) noexcept {
                _Promise&                 __promise = __h.promise();
                __generator_promise_base& __root    = *__promise.__root_;
                if (&__root != &__promise) {
                    auto __parent          = __promise.__parentOrLeaf_;
                    __root.__parentOrLeaf_ = __parent;
                    return __parent;
                }
                return noop_coroutine();
            }

            void await_resume() noexcept {}
        };

        __final_awaiter final_suspend() noexcept { return {}; }

        suspend_always yield_value(_Ref&& __x) noexcept(is_nothrow_move_constructible_v<_Ref>) {
            __root_->__value_.construct((_Ref&&)__x);
            return {};
        }

        template<class _T>
            requires(!is_reference_v<_Ref>) && is_convertible_v<_T, _Ref>
        suspend_always yield_value(_T&& __x) noexcept(is_nothrow_constructible_v<_Ref, _T>) {
            __root_->__value_.construct((_T&&)__x);
            return {};
        }

        template<class _Gen>
        struct __yield_sequence_awaiter {
            _Gen __gen_;

            __yield_sequence_awaiter(_Gen&& __g) noexcept
                // Taking ownership of the generator ensures frame are destroyed
                // in the reverse order of their execution.
                : __gen_((_Gen&&)__g) {}

            bool await_ready() noexcept { return false; }

            // set the parent, root and exceptions pointer and
            // resume the nested
            template<class _Promise>
            coroutine_handle<> await_suspend(coroutine_handle<_Promise> __h) noexcept {
                __generator_promise_base& __current = __h.promise();
                __generator_promise_base& __nested  = *__gen_.__get_promise();
                __generator_promise_base& __root    = *__current.__root_;

                __nested.__root_         = __current.__root_;
                __nested.__parentOrLeaf_ = __h;

                // Lazily construct the __exception_ member here now that we
                // know it will be used as a nested generator. This will be
                // destroyed by the promise destructor.
                __nested.__exception_.construct();
                __root.__parentOrLeaf_ = __gen_.__get_coro();

                // Immediately resume the nested coroutine (nested generator)
                return __gen_.__get_coro();
            }

            void await_resume() {
                __generator_promise_base& __nestedPromise = *__gen_.__get_promise();
                if (__nestedPromise.__exception_.get()) { rethrow_exception(std::move(__nestedPromise.__exception_.get())); }
            }
        };

        template<class _OValue, typename _OAlloc>
        __yield_sequence_awaiter<generator<_Ref, _OValue, _OAlloc>> yield_value(ranges::elements_of<
                                                                                generator<_Ref, _OValue, _OAlloc>> __g) noexcept {
            return move(__g).get();
        }

        template<ranges::range _Rng, typename _Allocator>
        __yield_sequence_awaiter<generator<_Ref, remove_cvref_t<_Ref>, _Allocator>> yield_value(ranges::elements_of<_Rng,
                                                                                                                    _Allocator>&&
                                                                                                  __x) {
            return [](allocator_arg_t,
                      [[maybe_unused]]
                      _Allocator alloc,
                      auto&&     __rng) -> generator<_Ref, remove_cvref_t<_Ref>, _Allocator> {
                for (const auto& e : __rng) co_yield static_cast<decltype(e)>(e);
            }(allocator_arg, __x.get_allocator(), forward<_Rng>(__x.get()));
        }

        void resume() { __parentOrLeaf_.resume(); }

        // Disable use of co_await within this coroutine.
        void await_transform() = delete;
    };

    template<class _Generator, typename _byteAllocator, bool _ExplicitAllocator = false>
    struct __generator_promise;

    template<class _Ref, typename _Value, typename _Alloc, typename _byteAllocator, bool _ExplicitAllocator>
    struct __generator_promise<generator<_Ref, _Value, _Alloc>, _byteAllocator, _ExplicitAllocator> final
        : public __generator_promise_base<_Ref>,
          public __promise_base_alloc<_byteAllocator> {
        __generator_promise() noexcept
            : __generator_promise_base<_Ref>(coroutine_handle<__generator_promise>::from_promise(*this)) {}

        generator<_Ref, _Value, _Alloc> get_return_object() noexcept {
            return generator<_Ref, _Value, _Alloc> { coroutine_handle<__generator_promise>::from_promise(*this) };
        }

        using __generator_promise_base<_Ref>::yield_value;

        template<ranges::range _Rng>
        typename __generator_promise_base<_Ref>::template __yield_sequence_awaiter<generator<_Ref, _Value, _Alloc>>
          yield_value(ranges::elements_of<_Rng>&& __x) {
            static_assert(!_ExplicitAllocator,
                          "This coroutine has an explicit allocator specified with "
                          "allocator_arg so an allocator needs to be passed "
                          "explicitely to elements_of");
            return [](auto&& __rng) -> generator<_Ref, _Value, _Alloc> {
                for (const auto& e : __rng) co_yield static_cast<decltype(e)>(e);
            }(forward<_Rng>(__x.get()));
        }
    };

    template<class _Alloc>
    using __byte_allocator_t = typename allocator_traits<remove_cvref_t<_Alloc>>::template rebind_alloc<byte>;

    // Type-erased allocator with default allocator behaviour.
    template<class _Ref, typename _Value, typename... _Ts>
    struct coroutine_traits<generator<_Ref, _Value>, _Ts...> {
        using promise_type = __generator_promise<generator<_Ref, _Value>, allocator<byte>>;
    };

    // Type-erased allocator with allocator_arg parameter
    template<class _Ref, typename _Value, typename _Alloc, typename... _Ts>
    struct coroutine_traits<generator<_Ref, _Value>, allocator_arg_t, _Alloc, _Ts...> {
      private:
        using __byte_allocator = __byte_allocator_t<_Alloc>;

      public:
        using promise_type = __generator_promise<generator<_Ref, _Value>, __byte_allocator, true /*explicit Allocator*/>;
    };

    // Type-erased allocator with allocator_arg parameter (non-static member functions)
    template<class _Ref, typename _Value, typename _This, typename _Alloc, typename... _Ts>
    struct coroutine_traits<generator<_Ref, _Value>, _This, allocator_arg_t, _Alloc, _Ts...> {
      private:
        using __byte_allocator = __byte_allocator_t<_Alloc>;

      public:
        using promise_type = __generator_promise<generator<_Ref, _Value>, __byte_allocator, true /*explicit Allocator*/>;
    };

    // Generator with specified allocator type
    template<class _Ref, typename _Value, typename _Alloc, typename... _Ts>
    struct coroutine_traits<generator<_Ref, _Value, _Alloc>, _Ts...> {
        using __byte_allocator = __byte_allocator_t<_Alloc>;

      public:
        using promise_type = __generator_promise<generator<_Ref, _Value, _Alloc>, __byte_allocator>;
    };

    // TODO :  make layout compatible promise casts possible
    template<class _Ref, typename _Value, typename _Alloc>
    class generator {
        using __byte_allocator = __byte_allocator_t<_Alloc>;

      public:
        using promise_type = __generator_promise<generator<_Ref, _Value, _Alloc>, __byte_allocator>;
        friend promise_type;

      private:
        using __coroutine_handle = coroutine_handle<promise_type>;

      public:
        generator() noexcept = default;

        generator(generator&& __other) noexcept
            : __coro_(exchange(__other.__coro_, {})), __started_(exchange(__other.__started_, false)) {}

        ~generator() noexcept {
            if (__coro_) {
                if (__started_ && !__coro_.done()) { __coro_.promise().__value_.destruct(); }
                __coro_.destroy();
            }
        }

        generator& operator=(generator&& g) noexcept {
            swap(g);
            return *this;
        }

        void swap(generator& __other) noexcept {
            swap(__coro_, __other.__coro_);
            swap(__started_, __other.__started_);
        }

        struct sentinel {};

        class iterator {
          public:
            using iterator_category = input_iterator_tag;
            using difference_type   = ptrdiff_t;
            using value_type        = _Value;
            using reference         = _Ref;
            using pointer           = add_pointer_t<_Ref>;

            iterator() noexcept       = default;
            iterator(const iterator&) = delete;

            iterator(iterator&& __other) noexcept : __coro_(exchange(__other.__coro_, {})) {}

            iterator& operator=(iterator&& __other) {
                std::swap(__coro_, __other.__coro_);
                return *this;
            }

            ~iterator() {}

            friend bool operator==(const iterator& it, sentinel) noexcept { return it.__coro_.done(); }

            iterator& operator++() {
                __coro_.promise().__value_.destruct();
                __coro_.promise().resume();
                return *this;
            }

            void operator++(int) { (void)operator++(); }

            reference operator*() const noexcept { return static_cast<reference>(__coro_.promise().__value_.get()); }

          private:
            friend generator;

            explicit iterator(__coroutine_handle __coro) noexcept : __coro_(__coro) {}

            __coroutine_handle __coro_;
        };

        iterator begin() {
            assert(__coro_);
            assert(!__started_);
            __started_ = true;
            __coro_.resume();
            return iterator { __coro_ };
        }

        sentinel end() noexcept { return {}; }

      private:
        explicit generator(__coroutine_handle __coro) noexcept : __coro_(__coro) {}

      public: // to get around access restrictions for __yield_sequence_awaitable
        coroutine_handle<> __get_coro() noexcept { return __coro_; }

        promise_type* __get_promise() noexcept { return addressof(__coro_.promise()); }

      private:
        __coroutine_handle __coro_;
        bool               __started_ = false;
    };

    // Specialisation for type-erased allocator implementation.
    template<class _Ref, typename _Value>
    class generator<_Ref, _Value, use_allocator_arg> {
        using __promise_base = __generator_promise_base<_Ref>;

      public:
        generator() noexcept : __promise_(nullptr), __coro_(), __started_(false) {}

        generator(generator&& __other) noexcept
            : __promise_(exchange(__other.__promise_, nullptr)),
              __coro_(exchange(__other.__coro_, {})),
              __started_(exchange(__other.__started_, false)) {}

        ~generator() noexcept {
            if (__coro_) {
                if (__started_ && !__coro_.done()) { __promise_->__value_.destruct(); }
                __coro_.destroy();
            }
        }

        generator& operator=(generator g) noexcept {
            swap(g);
            return *this;
        }

        void swap(generator& __other) noexcept {
            swap(__promise_, __other.__promise_);
            swap(__coro_, __other.__coro_);
            swap(__started_, __other.__started_);
        }

        struct sentinel {};

        class iterator {
          public:
            using iterator_category = input_iterator_tag;
            using difference_type   = ptrdiff_t;
            using value_type        = _Value;
            using reference         = _Ref;
            using pointer           = add_pointer_t<_Ref>;

            iterator() noexcept       = default;
            iterator(const iterator&) = delete;

            iterator(iterator&& __other) noexcept
                : __promise_(exchange(__other.__promise_, nullptr)), __coro_(exchange(__other.__coro_, {})) {}

            iterator& operator=(iterator&& __other) {
                __promise_ = exchange(__other.__promise_, nullptr);
                __coro_    = exchange(__other.__coro_, {});
                return *this;
            }

            ~iterator() = default;

            friend bool operator==(const iterator& it, sentinel) noexcept { return it.__coro_.done(); }

            iterator& operator++() {
                __promise_->__value_.destruct();
                __promise_->resume();
                return *this;
            }

            void operator++(int) { (void)operator++(); }

            reference operator*() const noexcept { return static_cast<reference>(__promise_->__value_.get()); }

          private:
            friend generator;

            explicit iterator(__promise_base* __promise, coroutine_handle<> __coro) noexcept
                : __promise_(__promise), __coro_(__coro) {}

            __promise_base*    __promise_;
            coroutine_handle<> __coro_;
        };

        iterator begin() {
            assert(__coro_);
            assert(!__started_);
            __started_ = true;
            __coro_.resume();
            return iterator { __promise_, __coro_ };
        }

        sentinel end() noexcept { return {}; }

      private:
        template<class _Generator, typename _byteAllocator, bool _ExplicitAllocator>
        friend struct __generator_promise;

        template<class _Promise>
        explicit generator(coroutine_handle<_Promise> __coro) noexcept
            : __promise_(addressof(__coro.promise())), __coro_(__coro) {}

      public: // to get around access restrictions for __yield_sequence_awaitable
        coroutine_handle<> __get_coro() noexcept { return __coro_; }

        __promise_base* __get_promise() noexcept { return __promise_; }

      private:
        __promise_base*    __promise_;
        coroutine_handle<> __coro_;
        bool               __started_ = false;
    };
} // namespace std
#endif
