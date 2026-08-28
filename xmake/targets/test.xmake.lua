local src_test_dir = path.join(src_dir, "test")

target("test", function()
    set_kind("static")
    add_rules("flags")

    set_languages("cxxlatest", "clatest")

    set_basename("stormkit-test")

    add_files(path.join(module_dir, "test.cppm"), { public = true })
    add_files(path.join(src_test_dir, "*.cpp"))

    add_headerfiles(path.join(include_dir, "(stormkit/test/**.hpp)"))
    add_includedirs(include_dir, { public = true })

    add_deps("core")

    add_options("sanitizers")

    set_group("libraries")
end)
