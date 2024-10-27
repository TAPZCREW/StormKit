package("frozen", function()
	set_kind("library", {moduleonly = true})
	set_homepage("https://github.com/Arthapz/frozen")
	set_description("A header-only, constexpr alternative to gperf for C++14 users")
	set_license("Apache-2.0")

	-- set_urls("https://github.com/Arthapz/frozen.git")
  set_sourcedir("../frozen")

	add_configs("modules", { default = false, type = "boolean" })

	on_install(function(package)
		local configs = {
			modules = package:config("modules"),
		}
		import("package.tools.xmake").install(package, configs)
	end)
end)
