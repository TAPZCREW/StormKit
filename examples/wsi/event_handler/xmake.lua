target("event_handler", function()
    set_kind("binary")
    set_languages("cxxlatest", "clatest")

    add_rules("stormkit.flags")
    add_rules("platform.windows.subsystem.windows")

    add_deps("stormkit-core", "stormkit-main", "stormkit-log", "stormkit-wsi")

    if is_mode("debug") then
        add_defines("STORMKIT_BUILD_DEBUG")
        add_defines("STORMKIT_ASSERT=1")
        set_suffixname("-d")
    else
        add_defines("STORMKIT_ASSERT=0")
    end

    add_files("src/main.cpp")
    if is_plat("windows") then add_files("win32/*.manifest") end

    set_group("examples/stormkit-wsi")
end)
