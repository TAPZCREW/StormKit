---------------------------- bootstrap ----------------------------
set_policy("compatibility.version", "3.0")

local allowedmodes = {
    "debug",
    "release",
    "releasedbg",
    "check",
    "coverage",
    "profile",
    "check",
    "minsizerel",
}

add_repositories("tapzcrew-repo https://github.com/Tapzcrew/xmake-repo main")

set_xmakever("2.9.5")
set_project("StormKit")

set_version("0.1.0", { build = "%Y%m%d%H%M" })

includes("xmake/rules/*.lua")

---------------------------- global rules ----------------------------
if get_config("vsxmake") then add_rules("plugin.vsxmake.autoupdate") end

if get_config("compile_commands") then
    add_rules("plugin.compile_commands.autoupdate", { outputdir = "build", lsp = "clangd" })
end

add_rules(
    "mode.debug",
    "mode.release",
    "mode.releasedbg",
    "mode.check",
    "mode.coverage",
    "mode.profile",
    "mode.check",
    "mode.minsizerel"
)

if not is_plat("windows") or not is_plat("mingw") then add_rules("mode.valgrind") end

set_fpmodels("fast")
add_vectorexts("fma")
add_vectorexts("neon")
add_vectorexts("avx", "avx2")
add_vectorexts("sse", "sse2", "sse3", "ssse3", "sse4.2")

---------------------------- options ----------------------------
includes("xmake/options.lua")

if get_config("devmode") then
    set_policy("build.c++.modules.incremental.bmicheck", true)
    set_policy("build.c++.modules.hide_dependencies", true)

    -- StormKit only build with llvm and libc++ currently
    set_toolchains("llvm")

    set_kind("shared")
end

---------------------------- global configs ----------------------------
set_allowedmodes(allowedmodes)
set_allowedplats("windows", "mingw", "linux", "macosx", "wasm")
set_allowedarchs("windows|x64", "mingw|x86_64", "linux|x86_64", "linux|aarch64", "macosx|x86_64", "macosx|arm64")

includes("xmake/dependencies.lua")
includes("xmake/targets.lua")

---------------------------- dependencies ----------------------------
for _, module in pairs(modules) do
    for _, package in ipairs(module.public_packages) do
        add_requires_with_conf_transitive(package)
    end
    for _, package in ipairs(module.packages) do
        add_requires_with_conf(package)
    end
end

---------------------------- targets ----------------------------
for name, module in pairs(modules) do
    local modulename = module.modulename

    if name == "core" or name == "main" or name == "test" or get_config(name) then
        target("stormkit-" .. name, function()
            set_group("libraries")

            if module.custom then module.custom() end

            if name == "main" or name == "test" then
                set_kind("static")
            else
                set_kind("$(kind)")
            end

            set_languages("cxxlatest", "clatest")

            add_rules("stormkit.flags")
            add_defines("STORMKIT_BUILD")
            if is_mode("debug") then
                add_defines("STORMKIT_BUILD_DEBUG")
                set_suffixname("-d")
            end

            if is_kind("static") then add_defines("STORMKIT_STATIC", { public = true }) end

            local src_path = path.join("src", modulename)
            local module_path = path.join("modules", "stormkit", modulename)
            local include_path = path.join("include", "(stormkit", modulename)

            for _, file in ipairs(os.files(path.join(src_path, "**.mpp"))) do
                add_files(file)
            end
            for _, file in ipairs(os.files(path.join(src_path, "**.cpp"))) do
                add_files(file)
            end
            for _, file in ipairs(os.files(path.join(src_path, "**.mm"))) do
                add_files(file, { mxxflags = "-std=c++23" })
            end
            for _, file in ipairs(os.files(path.join(src_path, "**.m"))) do
                add_files(file)
            end
            for _, file in ipairs(os.files(path.join(src_path, "**.inl"))) do
                add_files(file)
            end

            if os.exists(module_path .. ".mpp") then add_files(module_path .. ".mpp", { public = true }) end

            if os.files(module_path) then
                for _, file in ipairs(os.files(path.join(module_path, "**.mpp"))) do
                    add_files(file, { public = true })
                end
                for _, file in ipairs(os.files(path.join(module_path, "**.inl"))) do
                    add_headerfiles(file)
                end
            end

            local _include_path = include_path:gsub("%(", "")
            if os.exists(_include_path) then
                add_headerfiles(path.join(include_path, "**.inl)"))
                add_headerfiles(path.join(include_path, "**.hpp)"))
            end

            if is_plat("windows") or is_plat("mingw") then
                for _, plat in ipairs({ "posix", "linux", "darwin", "macos", "ios", "bsd", "android" }) do
                    remove_files(path.join(src_path, plat, "**"))
                    remove_headerfiles(path.join(src_path, plat, "**"))
                end
            elseif is_plat("macosx") then
                for _, plat in ipairs({ "linux", "win32", "ios", "bsd", "android" }) do
                    remove_files(path.join(src_path, plat, "**"))
                    remove_headerfiles(path.join(src_path, plat, "**"))
                end
            elseif is_plat("ios") then
                for _, plat in ipairs({ "linux", "macOS", "win32", "bsd", "android" }) do
                    remove_files(path.join(src_path, plat, "**"))
                    remove_headerfiles(path.join(src_path, plat, "**"))
                end
            elseif is_plat("android") then
                for _, plat in ipairs({ "linux", "darwin", "macos", "ios", "bsd", "win32" }) do
                    remove_files(path.join(src_path, plat, "**"))
                    remove_headerfiles(path.join(src_path, plat, "**"))
                end
            elseif is_plat("linux") then
                for _, plat in ipairs({ "win32", "darwin", "macos", "ios", "bsd", "android" }) do
                    remove_files(path.join(src_path, plat, "**"))
                    remove_headerfiles(path.join(src_path, plat, "**"))
                end
            end

            add_includedirs("$(projectdir)/include", { public = true })

            if module.defines then add_defines(module.defines) end

            if module.public_defines then add_defines(module.public_defines, { public = true }) end

            if module.cxxflags then
                add_cxxflags(module.cxxflags)
                add_mxxflags(module.cxxflags)
            end

            if module.deps then add_deps(module.deps) end

            if module.public_deps then add_deps(module.public_deps, { public = true }) end

            if module.packages then
                local packages = {}
                for _, package in ipairs(module.packages) do
                    table.insert(packages, package:split(" ")[1])
                end

                add_packages(packages, { public = is_kind("static") })
            end

            if module.public_packages then
                local packages = {}
                for _, package in ipairs(module.public_packages) do
                    table.insert(packages, package:split(" ")[1])
                end
                add_packages(packages, { public = true })
            end

            if module.frameworks then add_frameworks(module.frameworks, { public = is_kind("static") }) end

            add_options("sanitizers")
        end)
    end
end

if not is_host("windows") then add_requireconfs("**.pkg-config", { override = true, system = true }) end
add_requireconfs("**.bison", { override = true, system = true })
add_requireconfs("**.m4", { override = true, system = true })
add_requireconfs("**.python", { override = true, system = true })
add_requireconfs("**.meson", { override = true, system = true })
add_requireconfs("**.autoconf", { override = true, system = true })
add_requireconfs("**.cmake", { override = true, system = true })
add_requireconfs("**.nasm", { override = true, system = true })

for name, _ in pairs(modules) do
    if get_config("examples_" .. name) then
        local example_dir = path.join("examples", name)
        if os.exists(example_dir) and has_config("" .. name) then
            includes(path.join(example_dir, "**", "xmake.lua"))
        end
    end
end

if get_config("tests") then includes("xmake/tests.lua") end
if get_config("tools") then includes("tools/**.lua") end
