add_requires("imgui", {
    configs = { vulkan = true, debug = true, cxxflags = { "-DIMGUI_IMPL_VULKAN_NO_PROTOTYPES" } },
    system = false,
    debug = true,
})

target("imgui", function()
    add_rules("stormkit::example")

    add_files("src/*.cpp", "src/*.cppm", "../common/app.cppm")

    if get_config("devmode") then
        add_defines(format('RESOURCE_DIR="%s"', path.unix(path.join(os.projectdir(), "gpu/imgui"))))
        add_defines(format('SHADER_DIR="%s"', path.unix("$(builddir)/shader")))
    end
    add_embeddirs("$(builddir)/shaders")

    if get_config("devmode") then set_rundir("$(projectdir)") end

    add_packages("imgui", "volk", "vulkan-headers")

    set_group("examples/stormkit-gpu")
end)
