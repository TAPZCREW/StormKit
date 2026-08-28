local src_log_dir = path.join(src_dir, "log")

target("log", function()
    set_kind("$(kind)")
    add_rules("flags")

    set_languages("cxxlatest", "clatest")

    set_basename("stormkit-log")

    add_defines("STORMKIT_LOG_BUILD", { public = false })

    add_files(path.join(module_dir, "log.cppm"), { public = true })
    add_files(path.join(src_log_dir, "*.cpp"))

    add_headerfiles(path.join(include_dir, "(stormkit/log/**.hpp)"))
    add_includedirs(include_dir, { public = true })

    add_deps("core")

    add_options("sanitizers")

    set_group("libraries")
end)
