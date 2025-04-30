if has_config("enable_gpu") and has_config("enable_wsi") and has_config("enable-image") then
    target("game_of_life")
    do
        set_kind("binary")
        set_languages("cxxlatest", "clatest")

        add_packages("nzsl")
        add_deps("stormkit-core", "stormkit-main", "stormkit-log", "stormkit-wsi", "stormkit-gpu", "stormkit-image")

        add_rules("stormkit.utils.nzsl2spv")

        if is_mode("debug") then
            add_defines("STORMKIT_BUILD_DEBUG")
            add_defines("STORMKIT_ASSERT=1")
            set_suffixname("-d")
        else
            add_defines("STORMKIT_ASSERT=0")
        end

        add_files("src/*.cpp")
        add_files("src/*.mpp")
        add_files("shaders/*.nzsl")
        if is_plat("windows") then add_files("win32/*.manifest") end

        add_rules("windows.subsystem.windows")

        set_group("examples/stormkit-entities")
    end
end
