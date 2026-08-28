namespace("lua", function()
    target("events", function()
        add_rules("stormkit::example", "compile.shaders")

        add_files("src/*.cpp")

        if get_config("devmode") then
            add_defines(format('RESOURCE_DIR="%s"', path.unix(path.join(os.projectdir(), "examples/lua/wsi/events"))))
        end
        add_embeddirs("$(builddir)/shaders")

        if get_config("devmode") then set_rundir("$(projectdir)") end

        set_group("examples/stormkit-lua")
    end)
end)
