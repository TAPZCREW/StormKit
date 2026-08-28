namespace("examples", function()
    for _, name in ipairs({ "log", "entities", "gpu", "image", "wsi", "lua" }) do
        if get_config("examples_" .. name) then
            includes(path.join(os.projectdir(), "examples", name, "**/xmake.lua"))
        end
    end
end)
