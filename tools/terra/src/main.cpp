import std;

import stormkit.core;

#include <stormkit/main/main_macro.hpp>

#include <lua.hpp>

namespace stdr  = std::ranges;
namespace stdfs = std::filesystem;

using namespace std::literals;

using namespace stormkit;

auto main(const std::span<const std::string_view> args) noexcept -> int {
    if (stdr::size(args) < 2) {
        std::println(get_stderr(), "No template filename provided");
        return -1;
    }
    const auto template_path = stdfs::path { args[1] };
    if (not stdfs::exists(template_path)) {
        std::println(get_stderr(), "Template file {} doesn't exists", template_path.c_str());
        return -1;
    } else if (not stdfs::is_regular_file(template_path)) {
        std::println(get_stderr(),
                     "Template file {} path is not a regular file",
                     template_path.c_str());
        return -1;
    }

    const auto out_path = [&]() {
        if (stdr::size(args) < 3) return template_path.parent_path() / template_path.stem();
        return stdfs::path { args[2] };
    }();

    const auto template_data
      = io::readfile(io::text_file_tag, template_path)
          .transform_error(monadic::assert(std::format("Failed to read file {}, reason: ",
                                                       template_path.c_str())))
          .value();

    auto out = std::string {};
    out.reserve(stdr::size(template_data));

    // TODO replace with std::hive when supported
    auto buff = std::vector<char> {};
    buff.reserve(50);

    out += std::format(R"(
outfile = io.open("{}", "w")
)",
                       out_path.c_str());

    bool last_char_was_bracket = false;
    bool is_parsing_lua        = false;
    auto i                     = 0uz;
    for (auto&& c : template_data) {
        if (is_parsing_lua) {
            if (c == '%' and i + 1 < stdr::size(template_data) and template_data[i + 1] == '}') {
            } else if (c == '}' and i > 0 and template_data[i - 1] == '%') {
                is_parsing_lua = false;
            } else {
                out += c;
            }
        } else if (last_char_was_bracket) {
            is_parsing_lua = true;
            // flush tmp buff to render buffer
            out += "outfile:write(\"";
            for (auto c_buf : buff) {
                auto str_char = *to_string(as<i32>(c_buf), 16).transform_error(monadic::assert());
                if (stdr::size(str_char) == 1) str_char.insert(stdr::begin(str_char), '0');
                out += "\\x" + str_char;
            }
            out += "\")\n";
            buff.clear();
            last_char_was_bracket = false;
        } else {
            if (c == '{' and i + 1 < stdr::size(template_data) and template_data[i + 1] == '%')
                last_char_was_bracket = true;
            else
                buff.emplace_back(c);
        }

        ++i;
    }

    out += "outfile:write(\"";

    for (auto c_buf : buff) {
        auto str_char = *to_string(as<i32>(c_buf), 16).transform_error(monadic::assert());
        if (stdr::size(str_char) == 1) str_char.insert(stdr::begin(str_char), '0');
        out += "\\x" + str_char;
    }

    out += "\")";
    out += "outfile:close()";

    std::println("{}", out);

    return 0;
}
