require 'json'

Dir.chdir(".")
filename = Dir.glob("*.touchgfx")

file = File.read(filename[0])
filename[0].slice! ".touchgfx"
tgfxName = filename[0]

data_hash = JSON.parse(file)
if data_hash["Application"]["ApplicationTemplate"]["PostGenerateCommand"] !=  "touchgfx update_project --project-file=../EWARM/" + tgfxName + ".ewp --gui-group-name=Application/User/TouchGFX/gui --generated-group-name=Application/User/TouchGFX/generated && touchgfx update_project --project-file=simulator/msvs/Application.vcxproj && ruby updateMXProj.rb"
  data_hash["Application"]["ApplicationTemplate"]["PostGenerateCommand"] = "touchgfx update_project --project-file=../EWARM/" + tgfxName + ".ewp --gui-group-name=Application/User/TouchGFX/gui --generated-group-name=Application/User/TouchGFX/generated && touchgfx update_project --project-file=simulator/msvs/Application.vcxproj"
    File.open(tgfxName+".touchgfx","w") do |f|
    f.write(JSON.pretty_generate(data_hash))
  end
end

Dir.chdir("..")

filename = Dir.glob("*.ioc")
mxName = filename[0]

if mxName != (tgfxName + ".ioc")
  File.rename(mxName, tgfxName + ".ioc")
end

Dir.chdir("./EWARM")

filename = Dir.glob("*.ewp")
ewpName = filename[0]

if ewpName != (tgfxName + ".ewp")
  File.rename(ewpName, tgfxName + ".ewp")
  filename = Dir.glob("*.ewd")
  File.rename(filename[0],  tgfxName + ".ewd")

  xml = File.open( "Project.eww" )
  temp =  xml.readlines.join.gsub("STM32F7508_DISCO", tgfxName)
  f = File.open("Project.eww","w")
  f.write(temp)
end
