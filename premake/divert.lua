divert = {
	settings = nil
}

function divert.setup(settings)
	if not settings.source then error("Missing source.") end

	divert.settings = settings

	if not divert.settings.defines then divert.settings.defines = {} end
end

function divert.import()
	if not divert.settings then error("You need to call divert.setup first") end

	--filter "architecture:x86"
	--	libdirs { path.join(divert.settings.source, "x86") }
	--	
	--filter "architecture:x64"
	--	libdirs { path.join(divert.settings.source, "amd64") }
	--filter {}
	--
	--links { "WinDivert" }
	divert.includes()
end

function divert.includes()
	if not divert.settings then error("You need to call divert.setup first") end

	includedirs { path.join(divert.settings.source, "include") }
	defines(divert.settings.defines)
end

function divert.project()
	if not divert.settings then error("You need to call divert.setup first") end
end