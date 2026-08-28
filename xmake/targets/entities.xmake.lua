local src_entities_dir = path.join(src_dir, "entities")

target("entities", function()
    set_kind("$(kind)")
    add_rules("flags")

    set_languages("cxxlatest", "clatest")

    set_basename("stormkit-entities")

    add_defines("STORMKIT_ENTITIES_BUILD", { public = false })

    if get_config("lua") then add_packages("luau", "sol2_luau", { public = true }) end

    add_files(path.join(module_dir, "entities.cppm"), { public = true })
    add_files(path.join(src_entities_dir, "*.cpp"))

    add_headerfiles(path.join(include_dir, "(stormkit/entities/**.hpp)"))
    add_includedirs(include_dir, { public = true })

    add_options("sanitizers")

    add_deps("core")

    set_group("libraries")
end)
