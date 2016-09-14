settings = NewSettings()


if family == "windows" then
   if arch == "amd64" or arch == "ia64" then
      lib_sdl_path = "SDL2-2.0.4/lib/x64"
      lib_sdl_image_path = "SDL2_image-2.0.1/lib/x64"
   else
      lib_sdl_path = "SDL2-2.0.4/lib/x86"
      lib_sdl_image_path = "SDL2_image-2.0.1/lib/x86"
   end   
   settings.cc.includes:Add("SDL2-2.0.4/include")
   settings.cc.includes:Add("SDL2-2.0.4/include/SDL2")
   settings.cc.includes:Add("SDL2_image-2.0.1/include")
   settings.cc.includes:Add("SDL2_image-2.0.1/include/SDL2")
   settings.debug = 0
   settings.cc.flags:Add("/MD")
   settings.link.flags:Add("/SUBSYSTEM:CONSOLE")
   settings.link.libs:Add("SDL2main")
   settings.link.libpath:Add(lib_sdl_path)
   settings.link.libpath:Add(lib_sdl_image_path)


else
   settings.cc.flags:Add("-Wall");
end



settings.link.libs:Add("SDL2");
settings.link.libs:Add("SDL2_image");


source = Collect("*.c");

objects = Compile(settings, source)
exe = Link(settings, "sdl2d", objects)

if family == "windows" then
   dll_copy = CopyToDirectory(".", Collect(lib_sdl_path .. "/*.dll", 
                                           lib_sdl_image_path .. "/*.dll"))
   PseudoTarget("all", exe, dll_copy)
   DefaultTarget("all")
else
   DefaultTarget(exe)
end


