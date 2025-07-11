-- Merge binary shaders to archivess
rule("find_nzsl", function()
    on_config(function(target)
        import("core.project.project")
        import("core.tool.toolchain")
        import("lib.detect.find_tool")

        -- on windows+asan/mingw we need run envs because of .dll dependencies which may be not part of the PATH
        local envs
        if is_plat("windows") then
            local msvc = target:toolchain("msvc")
            if msvc and msvc:check() then envs = msvc:runenvs() end
        elseif is_plat("mingw") then
            local mingw = target:toolchain("mingw")
            if mingw and mingw:check() then envs = mingw:runenvs() end
        end
        target:data_set("nzsl_envs", envs)

        -- find nzsl binaries
        local nzsl = project.required_package("nzsl~host") or project.required_package("nzsl")
        local nzsldir
        if nzsl then
            nzsldir = path.join(nzsl:installdir(), "bin")
            local osenvs = os.getenvs()
            envs = envs or {}
            for env, values in pairs(nzsl:get("envs")) do
                local flatval = path.joinenv(values)
                local oldenv = envs[env] or osenvs[env]
                if not oldenv or oldenv == "" then
                    envs[env] = flatval
                elseif not oldenv:startswith(flatval) then
                    envs[env] = flatval .. path.envsep() .. oldenv
                end
            end
        end

        local nzsla = find_tool("nzsla", { version = true, paths = nzsldir, envs = envs })
        local nzslc = find_tool("nzslc", { version = true, paths = nzsldir, envs = envs })

        target:data_set("nzsla", nzsla)
        target:data_set("nzslc", nzslc)
        target:data_set("nzsl_runenv", envs)
    end)
end)

-- Compile shaders to includables headers
rule("compile.shaders", function()
    set_extensions(".nzsl", ".nzslb")
    add_deps("find_nzsl")
    on_config(function(target)
        local archives = {}

        for _, sourcebatch in pairs(target:sourcebatches()) do
            local rulename = sourcebatch.rulename
            if rulename == "compile.shaders" then
                for _, sourcefile in ipairs(sourcebatch.sourcefiles) do
                    local fileconfig = target:fileconfig(sourcefile)
                    if fileconfig and fileconfig.archive then
                        local archivefiles = archives[fileconfig.archive]
                        if not archivefiles then
                            archivefiles = {}
                            archives[fileconfig.archive] = archivefiles
                        end
                        table.insert(
                            archivefiles,
                            path.join(path.directory(sourcefile), path.basename(sourcefile) .. ".nzslb")
                        )
                    end
                end
            end
        end

        if not table.empty(archives) then
            assert(target:rule("@nzsl/archive.shaders"), "you must add the @nzsl/archive.shaders rule to the target")
            for archive, archivefiles in table.orderpairs(archives) do
                local args =
                    { rule = "@nzsl/archive.shaders", always_added = true, compress = true, files = archivefiles }
                if archive:endswith(".nzsla.h") or archive:endswith(".nzsla.hpp") then
                    args.header = path.extension(archive)
                    archive = archive:sub(1, -#args.header - 1) -- foo.nzsla.h => foo.nzsla
                end

                target:add("files", archive, args)
            end
        end
    end)

    before_buildcmd_file(function(target, batchcmds, shaderfile, opt)
        import("lib.detect.find_tool")

        local outputdir = path.join(import("core.project.config").builddir(), "shaders")
        local nzslc = target:data("nzslc")
        local runenvs = target:data("nzsl_runenv")
        assert(nzslc, "nzslc not found! please install nzsl package with nzslc enabled")

        local outputfile = path.join(outputdir or path.directory(shaderfile), path.basename(shaderfile) .. ".spv")

        -- add commands
        batchcmds:show_progress(opt.progress, "${color.build.object}compiling.shader %s", shaderfile)
        local argv = { "--compile=spv", "--optimize", "--spv-version=130", "--gl-flipy" }
        if outputdir then
            batchcmds:mkdir(outputdir)
            table.insert(argv, "--output=" .. outputdir)
        end

        -- handle --log-format
        local kind = target:data("plugin.project.kind") or ""
        if kind:match("vs") then table.insert(argv, "--log-format=vs") end

        table.insert(argv, shaderfile)

        batchcmds:vrunv(nzslc.program, argv, { curdir = ".", envs = runenvs })

        -- add deps
        batchcmds:add_depfiles(shaderfile)
        batchcmds:add_depvalues(nzslc.version)
        batchcmds:set_depmtime(os.mtime(outputfile))
        batchcmds:set_depcache(target:dependfile(outputfile))
    end)
end)
