-- Turns resources into includables headers
rule("stormkit.utils.resource2cpp", function()
    before_build(function(target, opt)
        import("core.base.option")
        import("utils.progress")

        local function GenerateEmbedHeader(filepath, targetpath)
            local bufferSize = 1024 * 1024

            if progress.apply_target then opt.progress = progress.apply_target(target, opt.progress) end
            progress.show(opt.progress, "${color.build.object}embedding %s", filepath)

            local resource = assert(io.open(filepath, "rb"))
            local targetFile = assert(io.open(targetpath, "w+"))

            local resourceSize = resource:size()

            local remainingSize = resourceSize
            local headerSize = 0

            while remainingSize > 0 do
                local readSize = math.min(remainingSize, bufferSize)
                local data = resource:read(readSize)
                remainingSize = remainingSize - readSize

                local headerContentTable = {}
                table.insert(headerContentTable, string.format("std::byte { %d}", data:byte(1)))
                for i = 2, #data do
                    table.insert(headerContentTable, string.format(", std::byte { %d }", data:byte(i)))
                end
                local content = table.concat(headerContentTable)

                headerSize = headerSize + #content

                targetFile:write(content)
            end

            resource:close()
            targetFile:close()
        end

        local target_dir = target:extraconf("rules", "utils.embed_resources", "outputdir")
            or path.join(target:autogendir(), "rules", "utils", "embed_resources")
        for _, sourcebatch in pairs(target:sourcebatches()) do
            if sourcebatch.rulename == "embed_resources" then
                for _, sourcefile in ipairs(sourcebatch.sourcefiles) do
                    local targetpath = path.join(target_dir, path.filename(sourcefile) .. ".hpp")
                    if option.get("rebuild") or os.mtime(sourcefile) >= os.mtime(targetpath) then
                        GenerateEmbedHeader(sourcefile, targetpath)
                    end
                end
            end
        end

        target:add("includedirs", target_dir)
    end)
end)
