if is_plat("linux") then
    add_requires("nzsl", { configs = { toolchains = "gcc", runtimes = "stdc++_shared", fs_watcher = false, link = {} } })
elseif is_plat("windows") then
    add_requires("nzsl", { configs = { toolchains = "msvc", runtimes = "MD", fs_watcher = false, links = {} } })
end

target("triangle", function()
    set_kind("binary")
    set_languages("cxxlatest", "clatest")

    add_rules("stormkit.flags")
    add_rules("platform.windows.subsystem.windows")

    add_rules("compile.shaders")
    add_deps("stormkit-core", "stormkit-main", "stormkit-log", "stormkit-wsi", "stormkit-gpu")

    if is_mode("debug") then
        add_defines("STORMKIT_BUILD_DEBUG")
        add_defines("STORMKIT_ASSERT=1")
        set_suffixname("-d")
    else
        add_defines("STORMKIT_ASSERT=0")
    end

    add_files("src/*.cpp")
    add_files("shaders/*.nzsl")
    if is_plat("windows") then add_files("win32/*.manifest") end

    add_includedirs("$(builddir)/shaders")

    if get_config("devmode") then
        add_defines("SHADER_DIR=\"$(builddir)/shaders\"")
        set_rundir("$(projectdir)")
    end

    -- add_cxflags("--embed-dir=$(builddir)/shaders", {tools = {"clang", "clangxx", "clang-cl", "gcc", "gxx"}})

    set_group("examples/stormkit-gpu")
end)
