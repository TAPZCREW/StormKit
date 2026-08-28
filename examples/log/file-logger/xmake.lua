target("file-logger", function()
    add_rules("stormkit::example")

    add_files("src/*.cpp")

    set_group("examples/stormkit-log")
end)
