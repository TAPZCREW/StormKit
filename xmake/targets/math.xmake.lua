local src_math_dir = path.join(src_dir, "math")

target("math", function()
    set_kind("$(kind)")
    add_rules("flags")

    set_languages("cxxlatest", "clatest")

    set_basename("stormkit-math")

    add_defines("STORMKIT_LOG_BUILD", { public = false })

    add_files(path.join(module_dir, "math.cppm"), { public = true })
    add_files(path.join(src_math_dir, "*.cpp"))

    add_headerfiles(path.join(include_dir, "(stormkit/math/**.hpp)"))
    add_includedirs(include_dir, { public = true })

    add_deps("core")

    add_options("sanitizers")

    set_group("libraries")
end)
