settings = NewSettings()

if family == "windows" then
   settings.cc.includes:Add("SDL2-2.0.1/include");
   settings.debug = 0
   settings.cc.flags:Add("/MD");
   settings.link.flags:Add("/SUBSYSTEM:CONSOLE");   
   settings.link.libs:Add("SDL2main");
   settings.link.libpath:Add("SDL2-2.0.1/lib/x86");
else
   settings.cc.flags:Add("-Wall");
end



settings.link.libs:Add("SDL2");
settings.link.libs:Add("SDL2_image");


source = Collect("*.c");

objects = Compile(settings, source)
exe = Link(settings, "sdl2d", objects)

