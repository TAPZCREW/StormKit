package("libktx", function()
    set_kind("library")
    set_homepage("https://github.com/KhronosGroup/KTX-Software")
    set_description("KTX (Khronos Texture) Library and Tools")
    set_license("Apache-2.0")

    set_urls(
        "https://github.com/KhronosGroup/KTX-Software/archive/refs/tags/$(version).tar.gz",
        "https://github.com/KhronosGroup/KTX-Software.git"
    )
    add_versions("v4.3.2", "74a114f465442832152e955a2094274b446c7b2427c77b1964c85c173a52ea1f")
    add_versions("v4.4.0", "3585d76edcdcbe3a671479686f8c81c1c10339f419e4b02a9a6f19cc6e4e0612")

    add_deps("cmake")

    add_configs("vulkan", { description = "Enable Vulkan texture upload.", default = true, type = "boolean" })
    add_configs("opengl", { description = "Enable OpenGL texture upload.", default = true, type = "boolean" })

    on_install("macosx", "android", "linux", "windows", "mingw", "cross", function(package)
        local configs = {}
        table.insert(configs, "-DCMAKE_BUILD_TYPE=" .. (package:is_debug() and "Debug" or "Release"))
        table.insert(configs, "-DBUILD_SHARED_LIBS=".. (package:config("shared") and "ON" or "OFF"))
        table.insert(configs, "-DKTX_FEATURE_STATIC_LIBRARY=" .. (package:config("shared") and "OFF" or "ON"))
        table.insert(configs, "-DKTX_FEATURE_VK_UPLOAD=" .. (package:config("vulkan") and "ON" or "OFF"))
        table.insert(configs, "-DKTX_FEATURE_GL_UPLOAD=" .. (package:config("opengl") and "ON" or "OFF"))
        table.insert(configs, "-DKTX_FEATURE_TESTS=OFF")
        table.insert(configs, "-DKTX_FEATURE_TOOLS=OFF")
        import("package.tools.cmake").install(package, configs)
    end)

    on_test(
        function(package)
            assert(package:check_csnippets({
                test = [[
            #include <ktx.h>

            void test() {
                ktxTexture* kTexture;
            }
        ]],
            }))
        end
    )
end)
