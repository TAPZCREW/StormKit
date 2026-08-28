add_requires("frozen", { system = false, configs = { modules = true, std_import = true, cpp = "latest" } })
add_requires("unordered_dense", { system = false, configs = { modules = true, std_import = true } })
add_requires("nontype_functional main")

local src_core_dir = path.join(src_dir, "core")
local module_core_dir = path.join(module_dir, "core")

target("core", function()
    set_kind("$(kind)")
    add_rules("flags")

    set_languages("cxxlatest", "clatest")

    set_basename("stormkit-core")

    add_defines("ANKERL_UNORDERED_DENSE_STD_MODULE=1", "FROZEN_STD_MODULE=1", { public = true })
    add_defines("STORMKIT_CORE_BUILD", { public = false })
    if is_mode("debug") then add_defines("STORMKIT_BUILD_DEBUG", { public = true }) end

    if is_kind("static") then add_defines("STORMKIT_STATIC", { public = true }) end

    add_files(path.join(module_dir, "core.cppm"), path.join(module_core_dir, "**.cppm"), { public = true })
    add_files(path.join(src_core_dir, "*.cpp"), path.join(src_core_dir, "*.cppm"))

    if is_plat("linux", "macosx", "iphoneos", "tvos", "android") then
        add_files(path.join(src_core_dir, "posix/**.cpp"))
    end
    if is_plat("linux") then add_files(path.join(src_core_dir, "linux/**.cpp")) end
    if is_plat("windows") then add_files(path.join(src_core_dir, "win32/**.cpp")) end
    if is_plat("macosx", "iphoneos", "tvos", "watchos") then
        add_files(path.join(src_core_dir, "darwin/**.cpp"), path.join(src_core_dir, "darwin/**.m"))
    end

    set_configdir("$(builddir)/.gens/include/")
    add_configfiles(path.join(include_dir, "(stormkit/core/config.hpp.in)"))
    add_includedirs("$(builddir)/.gens/include", { public = true })

    add_headerfiles(path.join(include_dir, "(status-code/**.hpp)"))
    add_headerfiles(path.join(include_dir, "(status-code/**.ipp)"))
    add_headerfiles(path.join(include_dir, "(stormkit/core/**.hpp)"), "$(builddir)/.gens/include/(stormkit/core/*.hpp)")
    add_includedirs(include_dir, { public = true })

    add_packages("frozen", "unordered_dense", "nontype_functional", { public = true })

    on_config(function(target)
        local output, errors = os.iorunv("git", { "rev-parse", "--abbrev-ref", "HEAD" })

        if not errors == "" then
            print("Failed to get git hash and branch, reason: ", errors, output)
            target:set("configvar", "STORMKIT_GIT_BRANCH", " ")
            target:set("configvar", "STORMKIT_GIT_COMMIT_HASH", " ")
            return
        end

        target:set("configvar", "STORMKIT_GIT_BRANCH", output:trim())
        output, errors = os.iorunv("git", { "rev-parse", "--verify", "HEAD" })

        target:set("configvar", "STORMKIT_GIT_COMMIT_HASH", output:trim())
    end)

    add_options("sanitizers")

    set_group("libraries")
end)
