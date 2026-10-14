// Copyright (C) 2026 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

import std;

import stormkit.core;
import stormkit.test;

#include <stdio.h>

#include <stormkit/test/test_macro.hpp>

using namespace stormkit;
using namespace std::literals;

using test_handle = void*;
#define TEST_INVALID_HANDLE nullptr

struct test_handle_impl {
    int a = 0;
};

auto create_test() -> test_handle {
    return reinterpret_cast<test_handle>(new test_handle_impl);
}

auto create_test2(test_handle* out) -> int {
    *out = reinterpret_cast<test_handle>(new test_handle_impl);
    return 0;
}

auto free_test(test_handle handle) -> void {
    delete reinterpret_cast<test_handle_impl*>(handle);
}

auto set_opt(test_handle handle, int opt) -> void {
    reinterpret_cast<test_handle_impl*>(handle)->a = opt;
}

auto get_opt(test_handle handle) -> int {
    return reinterpret_cast<test_handle_impl*>(handle)->a;
}

auto do_work(test_handle handle) {
    std::println("TEST {}", reinterpret_cast<test_handle_impl*>(handle)->a);
}

struct test_handle_trait1 {
    using element_type                = test_handle;
    static constexpr auto null_handle = TEST_INVALID_HANDLE;

    static constexpr auto constructor = create_test;
    static constexpr auto deleter     = free_test;
};

struct test_handle_trait2 {
    using element_type = test_handle;

    static constexpr auto constructor = [] static noexcept {
        auto handle = element_type { TEST_INVALID_HANDLE };
        create_test2(&handle);
        return handle;
    };
    static constexpr auto deleter = [](auto handle) static noexcept { free_test(handle); };
};

using test_handle_trait3 = owned_trait<test_handle, create_test, free_test, TEST_INVALID_HANDLE>;

using owned_test_handle1 = owned<test_handle_trait1>;
using owned_test_handle2 = owned<test_handle_trait2>;
using owned_test_handle3 = owned<test_handle_trait3>;

struct test_handle_interface {
    auto set_opt(this auto& self, int opt) -> void { ::set_opt(self.handle(), opt); }

    auto get_opt(this auto& self) -> int { return ::get_opt(self.handle()); }

    auto do_work(this auto& self) -> void { ::do_work(self.handle()); }
};

struct test_handle_trait4 {
    using element_type                = test_handle;
    using interface_type              = test_handle_interface;
    static constexpr auto null_handle = TEST_INVALID_HANDLE;

    static constexpr auto constructor = create_test;
    static constexpr auto deleter     = free_test;
};

struct test_handle_trait5 {
    using element_type   = test_handle;
    using interface_type = test_handle_interface;

    static constexpr auto constructor = [] static noexcept {
        auto handle = element_type { TEST_INVALID_HANDLE };
        create_test2(&handle);
        return handle;
    };
    static constexpr auto deleter = [](auto handle) static noexcept { free_test(handle); };
};

using test_handle_trait6 = owned_trait<test_handle, create_test, free_test, TEST_INVALID_HANDLE, test_handle_interface>;

using owned_test_handle4 = owned<test_handle_trait4>;
using owned_test_handle5 = owned<test_handle_trait5>;
using owned_test_handle6 = owned<test_handle_trait6>;

static_assert(meta::derived_from<owned_test_handle4, test_handle_interface>);

auto foo(core::view<test_handle_trait4> handle) {
}

auto foo(core::view<test_handle_trait5> handle) {
}

auto foo(core::view<test_handle_trait6> handle) {
}

namespace {
    auto _ = test::test_suite {
        "core.owned",
        {
          { "named_constructors",
            [] {
                {
                    auto value = owned_test_handle1::create();
                    EXPECTS(value.handle() != TEST_INVALID_HANDLE);

                    auto handle = create_test();
                    auto value2 = owned_test_handle1::take(std::move(handle));

                    EXPECTS(value2.handle() != TEST_INVALID_HANDLE);
                }
                {
                    auto value = owned_test_handle2::create();
                    EXPECTS(value.handle() != TEST_INVALID_HANDLE);

                    auto value2 = owned_test_handle2::take(create_test());
                    EXPECTS(value2.handle() != TEST_INVALID_HANDLE);
                }
                {
                    auto value = owned_test_handle3::create();
                    EXPECTS(value.handle() != TEST_INVALID_HANDLE);

                    auto handle = create_test();
                    auto value2 = owned_test_handle3::take(std::move(handle));

                    EXPECTS(value2.handle() != TEST_INVALID_HANDLE);
                }
                {
                    auto value = owned_test_handle4::create();
                    EXPECTS(value.handle() != TEST_INVALID_HANDLE);

                    auto handle = create_test();
                    auto value2 = owned_test_handle4::take(std::move(handle));

                    EXPECTS(value2.handle() != TEST_INVALID_HANDLE);
                }
                {
                    auto value = owned_test_handle5::create();
                    EXPECTS(value.handle() != TEST_INVALID_HANDLE);

                    auto value2 = owned_test_handle5::take(create_test());
                    EXPECTS(value2.handle() != TEST_INVALID_HANDLE);
                }
                {
                    auto value = owned_test_handle6::create();
                    EXPECTS(value.handle() != TEST_INVALID_HANDLE);

                    auto handle = create_test();
                    auto value2 = owned_test_handle6::take(std::move(handle));

                    EXPECTS(value2.handle() != TEST_INVALID_HANDLE);
                }
            } },
          { "view_construction",
            [] {
                {
                    auto value = owned_test_handle1::create();
                    auto view  = core::view { value };
                    EXPECTS(view.handle() == value.handle());
                }
                {
                    auto value = owned_test_handle2::create();
                    auto view  = core::view { value };
                    EXPECTS(view.handle() == value.handle());
                }
                {
                    auto value = owned_test_handle3::create();
                    auto view  = core::view { value };
                    EXPECTS(view.handle() == value.handle());
                }
                {
                    auto value = owned_test_handle4::create();
                    auto view  = core::view { value };
                    EXPECTS(view.handle() == value.handle());

                    foo(value);
                    foo(view);
                    foo(std::move(view));
                }
                {
                    auto value = owned_test_handle5::create();
                    auto view  = core::view { value };
                    EXPECTS(view.handle() == value.handle());

                    foo(value);
                    foo(view);
                    foo(std::move(view));
                }
                {
                    auto value = owned_test_handle6::create();
                    auto view  = core::view { value };
                    EXPECTS(view.handle() == value.handle());

                    foo(value);
                    foo(view);
                    foo(std::move(view));
                }
            } },
          { "interfaces",
            [] {
                {
                    auto value = owned_test_handle4::create();
                    auto view  = core::view { value };

                    value.set_opt(3);
                    value.do_work();

                    EXPECTS(value.get_opt() == 3);
                    EXPECTS(value.get_opt() == view.get_opt());
                }
                {
                    auto value = owned_test_handle5::create();
                    auto view  = core::view { value };

                    value.set_opt(3);
                    value.do_work();

                    EXPECTS(value.get_opt() == 3);
                    EXPECTS(value.get_opt() == view.get_opt());
                }
                {
                    auto value = owned_test_handle6::create();
                    auto view  = core::view { value };

                    value.set_opt(3);
                    value.do_work();

                    EXPECTS(value.get_opt() == 3);
                    EXPECTS(value.get_opt() == view.get_opt());
                }
            } },

          },
    };
} // namespace
