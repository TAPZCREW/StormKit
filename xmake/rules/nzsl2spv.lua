-- Compile shaders to includables headers
rule("compile.shaders")
do
    set_extensions(".nzsl", ".nzslb")

    -- for c++ module dependency discovery
    on_load(function(target)
        if target:rule("c++.build.modules.builder") then
            local rule = target:rule("c++.build.modules.builder"):clone()
            rule:add("deps", "compile.shaders", { order = true })
            target:rule_add(rule)
        end
    end)

    on_config(function(target)
        import("core.base.option")

        -- add outputdir to include path
        local outputdir = target:extraconf("rules", "compile.shaders", "outputdir")
            or path.join(target:autogendir(), "rules", "compile.shaders")
        if not os.isdir(outputdir) then os.mkdir(outputdir) end
        target:add("includedirs", outputdir)

        if target:rule("c++.build.modules") then
            local dryrun = option.get("dry-run")
            local sourcebatches = target:sourcebatches()
            if not dryrun and sourcebatches["compile.shaders"] and sourcebatches["compile.shaders"].sourcefiles then
                for _, shaderfile in ipairs(sourcebatches["compile.shaders"].sourcefiles) do
                    local outputfile = path.join(outputdir, path.basename(shaderfile) .. ".spv.h")

                    -- for c++ module dependency discovery
                    if not os.exists(outputfile) then os.touch(outputfile) end
                end
            end
        end
    end)

    before_buildcmd_file(function(target, batchcmds, shaderfile, opt)
        import("core.tool.toolchain")
        import("lib.detect.find_tool")
        import("core.project.project")

        local outputdir = target:extraconf("rules", "compile.shaders", "outputdir")
            or path.join(target:autogendir(), "rules", "compile.shaders")
        local fileconfig = target:fileconfig(shaderfile)
        if fileconfig and fileconfig.prefixdir then outputdir = path.join(outputdir, fileconfig.prefixdir) end

        -- on mingw we need run envs because of .dll dependencies which may be not part of the PATH
        local envs
        if is_plat("mingw") then
            local mingw = toolchain.load("mingw")
            if mingw and mingw:check() then envs = mingw:runenvs() end
        end

        -- find nzslc
        local nzsl = project.required_package("nzsl~host") or project.required_package("nzsl")
        local nzsldir
        if nzsl then nzsldir = path.join(nzsl:installdir(), "bin") end

        local nzslc = find_tool("nzslc", { paths = nzsldir, envs = envs })
        assert(nzslc, "nzslc not found! please install nzsl package")

        -- add commands
        batchcmds:show_progress(opt.progress, "${color.build.object}compiling.shader %s", shaderfile)
        local argv = { "--compile=spv-header", "--optimize", "--output=" .. outputdir }
        batchcmds:mkdir(outputdir)

        -- handle --log-format
        local kind = target:data("plugin.project.kind") or ""
        if kind:match("vs") then table.insert(argv, "--log-format=vs") end

        table.insert(argv, shaderfile)

        local outputfile = path.join(outputdir, path.basename(shaderfile) .. ".nzslb.h")
        batchcmds:vrunv(nzslc.program, argv, { curdir = ".", envs = envs })

        -- add deps
        batchcmds:add_depfiles(shaderfile)
        batchcmds:set_depmtime(os.mtime(outputfile))
        batchcmds:set_depcache(target:dependfile(outputfile))
    end)
end
