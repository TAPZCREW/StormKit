target("sprites")
do
    set_kind("binary")
    set_languages("cxxlatest", "clatest")

    -- add_rules("@nzsl/compile.shaders")
    add_rules("compile.shaders")
    add_deps("stormkit-core", "stormkit-main", "stormkit-log", "stormkit-wsi", "stormkit-engine")

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

    set_group("examples/stormkit-engine")
end
