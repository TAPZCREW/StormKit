if is_plat("linux") then
    add_requires("libxcb")
    add_requires("xcb-util-keysyms")
    add_requires("xcb-util")
    add_requires("xcb-util-image")
    add_requires("xcb-util-wm")
    add_requires("xcb-util-errors")
    add_requires("wayland")
    add_requires("wayland-protocols")
    add_requires("libxkbcommon", {
        system = false,
        configs = {
            wayland = true,
            x11 = true,
        },
    })
end

local src_wsi_dir = path.join(src_dir, "wsi")
local module_wsi_dir = path.join(module_dir, "wsi")

target("wsi", function()
    set_kind("$(kind)")
    add_rules("flags")

    set_languages("cxxlatest", "clatest")

    set_basename("stormkit-wsi")

    add_defines("STORMKIT_WSI_BUILD", { public = false })

    add_files(path.join(module_dir, "wsi.cppm"), path.join(module_wsi_dir, "**.cppm"), { public = true })
    add_files(path.join(src_wsi_dir, "*.cpp"), path.join(src_wsi_dir, "common/*.cppm"))

    if is_plat("linux") then
        add_files(path.join(src_wsi_dir, "linux/**.cpp"), path.join(src_wsi_dir, "linux/**.cppm"))

        add_rules("wayland.protocols")

        on_load(function(target)
            assert(target:pkg("wayland-protocols"))
            local wayland_protocols_dir =
                path.join(target:pkg("wayland-protocols"):installdir() or "/usr", "share", "wayland-protocols")
            assert(wayland_protocols_dir, "wayland protocols directory not found")

            local protocols = {
                path.join("stable", "xdg-shell", "xdg-shell.xml"),
                path.join("stable", "tablet", "tablet-v2.xml"),
                path.join("stable", "viewporter", "viewporter.xml"),
                path.join("staging", "content-type", "content-type-v1.xml"),
                path.join("staging", "pointer-warp", "pointer-warp-v1.xml"),
                path.join("staging", "cursor-shape", "cursor-shape-v1.xml"),
                path.join("staging", "single-pixel-buffer", "single-pixel-buffer-v1.xml"),
                path.join("unstable", "xdg-decoration", "xdg-decoration-unstable-v1.xml"),
                path.join("unstable", "pointer-constraints", "pointer-constraints-unstable-v1.xml"),
                path.join("unstable", "relative-pointer", "relative-pointer-unstable-v1.xml"),
            }

            for _, protocol in ipairs(protocols) do
                target:add("files", path.join(wayland_protocols_dir, protocol))
            end
        end)

        add_packages(
            "libxcb",
            "xcb-util-keysyms",
            "xcb-util",
            "xcb-util-image",
            "xcb-util-wm",
            "xcb-util-errors",
            "wayland",
            "wayland-protocols",
            "libxkbcommon"
        )
    elseif is_plat("windows") then
        add_files(path.join(src_wsi_dir, "win32/**.cpp"), path.join(src_wsi_dir, "win32/**.cppm"))
        add_syslinks("User32", "Shell32", "Gdi32", "Shcore", "Gdiplus")
    elseif is_plat("macosx") then
        add_files(
            path.join(src_wsi_dir, "macos/**.cpp"),
            path.join(src_wsi_dir, "macos/**.m"),
            path.join(src_wsi_dir, "macos/**.cppm")
        )
        add_files(path.join(src_wsi_dir, "macos/**.swift"), { public = true })
        set_values("swift.modulename", "macOS")
        set_values("swift.interop", "cxx")
        add_scflags("-I" .. path.join(src_wsi_dir, "macos/swift"))
    elseif is_plat("iphoneos") then
        add_files(path.join(src_wsi_dir, "ios/**.cpp"), path.join(src_wsi_dir, "ios/**.m"))
        add_files(path.join(src_wsi_dir, "ios/**.swift"), { public = true })
        set_values("swift.modulename", "iOS")
        set_values("swift.interop", "cxx")
        add_scflags("-I" .. path.join(src_wsi_dir, "ios/swift"))
    elseif is_plat("tvos") then
        add_files(
            path.join(src_wsi_dir, "tvos/**.cpp"),
            path.join(src_wsi_dir, "tvos/**.m"),
            path.join(src_wsi_dir, "tvos/**.swift")
        )
        set_values("swift.modulename", "tvOS")
        set_values("swift.interop", "cxx")
        add_scflags("-I" .. path.join(src_wsi_dir, "tvos/swift"))
    end

    add_headerfiles(path.join(include_dir, "(stormkit/wsi/**.hpp)"))
    add_includedirs(include_dir, { public = true })

    add_deps("core", "log")

    add_packages(packages, { public = false })

    add_options("sanitizers")

    set_group("libraries")
end)
