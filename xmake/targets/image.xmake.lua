add_requires("libktx")
add_requires("libpng")
add_requires("libjpeg-turbo", is_plat("windows") and {
    system = false,
    configs = {
        runtimes = "MD",
        shared = true,
    },
} or {})

local src_image_dir = path.join(src_dir, "image")

target("image", function()
    set_kind("$(kind)")
    add_rules("flags")

    set_languages("cxxlatest", "clatest")

    set_basename("stormkit-image")

    add_defines("STORMKIT_IMAGE_BUILD", { public = false })

    add_files(path.join(module_dir, "image.cppm"), { public = true })
    add_files(path.join(src_image_dir, "*.cpp"), path.join(src_image_dir, "*.cppm"))

    add_headerfiles(path.join(include_dir, "(stormkit/image/**.hpp)"))
    add_includedirs(include_dir, { public = true })

    add_deps("core")

    add_packages("libktx", "libpng", "libjpeg-turbo")

    add_options("sanitizers")

    set_group("libraries")
end)
