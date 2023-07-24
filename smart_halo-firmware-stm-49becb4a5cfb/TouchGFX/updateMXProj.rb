require 'json'

Dir.chdir(".")
filename = Dir.glob("*.touchgfx")

file = File.read(filename[0])
filename[0].slice! ".touchgfx"
tgfxName = filename[0]

data_hash = JSON.parse(file)
iarName = data_hash["Application"]["ApplicationTemplate"]["PostGenerateCommand"][/#{"../EWARM/"}(.*?)#{".ewp"}/m, 1]

if iarName != tgfxName
  data_hash["Application"]["ApplicationTemplate"]["PostGenerateCommand"] = data_hash["Application"]["ApplicationTemplate"]["PostGenerateCommand"].gsub(iarName+".ewp", tgfxName+".ewp")
  data_hash["Application"]["ApplicationTemplate"]["PostGenerateCommand"] = data_hash["Application"]["ApplicationTemplate"]["PostGenerateCommand"].gsub(" && ruby updateMXProj.rb", "")
  
  File.open(tgfxName+".touchgfx","w") do |f|
    f.write(JSON.pretty_generate(data_hash))
  end
end 

Dir.chdir("..")

filename = Dir.glob("*.ioc")
cmxName = filename[0]

if cmxName != (tgfxName + ".ioc")
  File.rename(cmxName, tgfxName + ".ioc")
end

Dir.chdir("./EWARM")

filename = Dir.glob("*.ewp")
ewpName = filename[0]

if ewpName != (tgfxName + ".ewp")
  File.rename(ewpName, tgfxName + ".ewp")
  filename = Dir.glob("*.ewd")
  File.rename(filename[0],  tgfxName + ".ewd")

  xml = File.open( "Project.eww" )
  temp =  xml.readlines.join.gsub("STM32F746G-DISCO", tgfxName)
  f = File.open("Project.eww","w")
  f.write(temp)
end