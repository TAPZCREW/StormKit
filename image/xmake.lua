target("stormkit-image")
    set_kind("$(kind)")
    set_languages("cxxlatest", "clatest")

    if is_mode("debug") then
        set_suffixname("-d")
    end

    add_headerfiles("include/(stormkit/**.hpp)")
    add_headerfiles("include/(stormkit/**.inl)")
    add_files("src/**.cpp")

    if has_config("enable_cxx20_modules") then
        add_files("include/**.mpp")
        add_files("src/**.mpp")
    else
        add_headerfiles("include/(stormkit/**.mpp)")
        add_headerfiles("src/**.mpp")
    end

    add_packages("gli", "libpng", "libjpeg")
    add_deps("stormkit-core", { public = true })
    add_includedirs("include", { public = true })
    add_includedirs("$(buildir)/include")

    add_options("enable_cxx20_modules")

    set_group("libraries")

    before_install(function(target)
        target:set("headerfiles")
    end)

    on_install(function(target)
        os.cp(path.join(target:scriptdir(), "include/*"), path.join(target:installdir(), "include"))
    end)

