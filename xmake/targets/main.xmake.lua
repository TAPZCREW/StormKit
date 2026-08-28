local src_main_dir = path.join(src_dir, "main")

target("main", function()
    set_kind("static")
    add_rules("flags")

    set_languages("cxxlatest", "clatest")

    set_basename("stormkit-main")

    add_defines("STORMKIT_MAIN_BUILD", { public = false })

    add_files(path.join(module_dir, "main.cppm"), { public = true })
    if is_plat("linux") then add_files(path.join(src_main_dir, "linux/**.cpp")) end
    if is_plat("windows") then add_files(path.join(src_main_dir, "win32/**.cpp")) end
    if is_plat("macosx") then add_files(path.join(src_main_dir, "macos/**.cpp")) end
    if is_plat("iphoneos") then add_files(path.join(src_main_dir, "ios/**.cpp")) end
    if is_plat("tvos") then add_files(path.join(src_main_dir, "tvos/**.cpp")) end
    if is_plat("android") then add_files(path.join(src_main_dir, "android/**.cpp")) end

    add_headerfiles("$(projectdir)/include/(stormkit/main/**.hpp)")
    add_includedirs("$(projectdir)/include", { public = true })

    add_cxflags("-Wno-main", { tools = { "clang", "gcc" } })

    add_deps("core")

    if is_plat("macosx") then add_frameworks("CoreFoundation") end

    add_options("sanitizers")

    set_group("libraries")
end)
