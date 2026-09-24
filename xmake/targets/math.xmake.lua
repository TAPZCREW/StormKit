local src_math_dir = path.join(src_dir, "math")
local module_math_dir = path.join(module_dir, "math")

target("math", function()
    set_kind("$(kind)")
    add_rules("flags")

    set_languages("cxxlatest", "clatest")

    set_basename("stormkit-math")

    add_defines("STORMKIT_MATH_BUILD", { public = false })

    add_files(path.join(module_dir, "math.cppm"), path.join(module_math_dir, "**.cppm"), { public = true })
    if os.exists(src_math_dir) then add_files(path.join(src_math_dir, "*.cpp")) end

    add_headerfiles(path.join(include_dir, "(stormkit/math/**.hpp)"))
    add_includedirs(include_dir, { public = true })

    add_deps("core")

    add_options("sanitizers")

    set_group("libraries")
end)
