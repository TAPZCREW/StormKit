---------------------------- bootstrap ----------------------------
local allowed_modes = {
    "debug",
    "release",
    "releasedbg",
    "profile",
}

add_repositories("nazara-repo https://github.com/NazaraEngine/xmake-repo")
add_repositories("tapzcrew-repo https://github.com/Tapzcrew/xmake-repo main")

set_xmakever("3.0.0")
set_project("StormKit")

set_version("0.1.0", { build = "%Y%m%d%H%M" })

---------------------------- options ----------------------------
includes("xmake/options.xmake.lua")

if get_config("devmode") then set_policy("build.c++.modules.hide_dependencies", true) end

---------------------------- global rules ----------------------------
includes("xmake/rules/*.xmake.lua")

if get_config("vsxmake") then add_rules("plugin.vsxmake.autoupdate") end

if get_config("compile_commands") then
    add_rules("plugin.compile_commands.autoupdate", { outputdir = "build", lsp = "clangd" })
end

for _, mode in ipairs(allowed_modes) do
    add_rules("mode." .. mode)
end

if not is_host("windows") then add_rules("mode.valgrind") end

---------------------------- global configs ----------------------------
set_allowedmodes(allowed_modes)
set_allowedplats("windows", "mingw", "linux", "macosx", "wasm")
set_allowedarchs("windows|x64", "windows|arm64", "linux|x86_64", "linux|aarch64", "macosx|x86_64", "macosx|arm64")

set_fpmodels("fast")
add_vectorexts("fma")
add_vectorexts("neon")
add_vectorexts("avx", "avx2")
add_vectorexts("sse", "sse2", "sse3", "ssse3", "sse4.2")

local suffix
if is_kind("static") then suffix = "-static" end

if is_mode("debug") then
    suffix = (suffix or "") .. "-debug"
elseif is_mode("reldbg") then
    suffix = (suffix or "") .. "-reldebug"
end

if suffix then set_suffixname(suffix) end

---------------------------- configvar ----------------------------
for _, name in ipairs({ "log", "entities", "image", "wsi", "gpu", "lua" }) do
    if get_config(name) then set_configvar("STORMKIT_LIB_" .. string.upper(name) .. "_ENABLED", true) end
end

---------------------------- targets ----------------------------
namespace("stormkit", function()
    module_dir = "$(projectdir)/modules/stormkit"
    src_dir = "$(projectdir)/src"
    include_dir = "$(projectdir)/include"

    includes("xmake/targets/core.xmake.lua")
    includes("xmake/targets/main.xmake.lua")

    for _, name in ipairs({ "math", "log", "entities", "gpu", "image", "wsi", "lua" }) do
        if get_config(name) then includes("xmake/targets/" .. name .. ".xmake.lua") end
    end

    includes("xmake/targets/examples.xmake.lua")

    if get_config("tests") then
        includes("xmake/targets/test.xmake.lua")
        includes("xmake/targets/tests.xmake.lua")
    end
    if get_config("tools") then includes("xmake/targets/tools.xmake.lua") end

    target("stormkit", function()
        set_kind("static")

        set_languages("cxxlatest", "clatest")

        set_suffixname("")

        add_rules("stormkit::flags")

        add_files("modules/stormkit.cppm", { public = true })

        add_deps("stormkit::core")
        for _, name in ipairs({ "log", "entities", "gpu", "image", "wsi", "lua" }) do
            if get_config(name) then
                add_deps("stormkit::" .. name)
                add_defines("STORMKIT_IMPORT_" .. string.upper(name), { public = true })
            end
        end

        set_group("libraries")
    end)
end)
