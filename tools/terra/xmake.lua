target("terra", function()
    add_rules("stormkit::example")

    add_files("src/main.cpp")

    set_group("tools")
end)
