##############################################################################
# This file is part of the TouchGFX 4.10.0 distribution.
#
# <h2><center>&copy; Copyright (c) 2018 STMicroelectronics.
# All rights reserved.</center></h2>
#
# This software component is licensed by ST under Ultimate Liberty license
# SLA0044, the "License"; You may not use this file except in compliance with
# the License. You may obtain a copy of the License at:
#                             www.st.com/SLA0044
#
##############################################################################

$:.unshift File.dirname(__FILE__)
require 'rubygems'
require 'lib/generator'
require 'lib/emitters/fonts_cpp'

WINDOWS_LINE_ENDINGS = "\r\n"
UNIX_LINE_ENDINGS = "\n"
#on windows/mingw file.write will it self translate \n to \r\n, on linux not
LINE_ENDINGS = RUBY_PLATFORM.match(/linux/) ? WINDOWS_LINE_ENDINGS : UNIX_LINE_ENDINGS

FileEntry = Struct.new(:cpp_file, :content, :texts_name, :texts, :texts_hash, :texts_string, :texts_string_hash, :extern)
StringEntry = Struct.new(:cpp_file, :t_symbol, :int_values)
Database = Struct.new(:pragma, :name, :value)
DatabaseArray = Struct.new(:pragma, :databases, :finish)

def root_dir
  # Get the dirname of this (main.rb) file:
  @root_dir ||= File.dirname(__FILE__)
end

class Main
  def self.banner
    <<-BANNER
Create cpp text files from excel translations

Usage: #{File.basename($0)} file.xlsx path/to/fontconvert.out path/to/fonts_output_dir path/to/localization_output_dir path/to/font/asset remap_identical_texts data_format
Where 'remap_identical_texts' is yes/no
      'data_format' is A4/A8 or blank
BANNER
  end

  def self.save_int_flash(localization_output_path)
    local_path = localization_output_path.gsub('\\','/')
    # Handle Languge*.cpp
    languages_cpp = Dir["#{local_path}/src/Language*.cpp"]
    files = languages_cpp.collect do |cpp|
      content = IO.binread(cpp)
      
      texts_string_match = content.match(/^\/\/ Language.*?\r\n(.*?\r\n)\r\n/m)
      return unless texts_string_match
      texts_string = texts_string_match[1].split(";\r\n")
      texts_string_hash = {}
      texts_string.each_with_index { |t,ix| texts_string_hash[t.match(/UnicodeChar (T_[^\[]*)/)[1]] = ix }
      
      texts_array_match = content.match(/^{(.*)^}/m)
      return unless texts_array_match
      texts_array = texts_array_match[1].gsub(/\s+/,'').split(',')
      texts_array_hash = {}
      texts_array.each_with_index { |t,ix| texts_array_hash[t] = ix }
      
      texts_name_match = content.match(/^KEEP extern const touchgfx::Unicode::UnicodeChar\* const (texts.*)\[/)
      return unless texts_name_match
      texts_name = texts_name_match[1]
      
      FileEntry.new(cpp,content,texts_name,texts_array,texts_array_hash,texts_string,texts_string_hash,[])
    end
    files_cpp_index = {}
    files.each_with_index do |file,index|
      files_cpp_index[file.cpp_file] = index
    end
    strings = files.collect do |file|
      file.content.scan(/UnicodeChar (T_.*)\[.*{ ([0-9a-fA-Fx, ]*) }/).collect { |t_symbol, hex_values| StringEntry.new(file.cpp_file, t_symbol, hex_values.split(',').map{|hex|Integer(hex)}) }
    end.flatten.sort_by { |x| sprintf("%010d %s %s",x.int_values.length,x.cpp_file,x.t_symbol) }.reverse
    remapped_strings = 0
    remapped_bytes = 0
    str_index = {}
    strings.each_with_index do |short,short_ix|
      long_ix = str_index[short.int_values]
      if long_ix
        remapped_strings += 1
        remapped_bytes += short.int_values.length * 2

        long = strings[long_ix]
        srch = short.t_symbol
        rplc = long.t_symbol
        offset = long.int_values.length - short.int_values.length
        ofst = offset > 0 ? "+#{offset}" : ""
        file = files[files_cpp_index[short.cpp_file]]
        # Remove the 'KEEP' declaration to prevent the symbol from staying in memory even if unreferenced
        file.texts_string[file.texts_string_hash[srch]] = ''
        file.texts[file.texts_hash[srch]] = "#{rplc}#{ofst}"
        if short.cpp_file != long.cpp_file
          file.extern += [ rplc ]
          files[files_cpp_index[long.cpp_file]].extern += [ rplc ]
        end
      else
        for start in 0 ... short.int_values.length-1
          sub_int_values = short.int_values[start..-1]
          # if the substring is present, all shorter substrings are also present
          break if str_index[sub_int_values]
          str_index[sub_int_values] = short_ix
        end
      end
    end
    puts "Remapped #{remapped_strings} strings (#{remapped_bytes} bytes saved)" if remapped_strings > 0

    # Handle Texts.cpp
    texts_cpp = "#{local_path}/src/Texts.cpp"
    texts = IO.binread(texts_cpp)
    files.each_with_index do |srch,i|
      j = files.rindex { |f| srch.texts == f.texts }
      if j > i
        rplc = files[j]
        texts.gsub!(/^extern const touchgfx::Unicode::UnicodeChar\* const #{rplc.texts_name}\[\];\r\n/,'')
        texts.gsub!(/^( *)#{rplc.texts_name}(,|\r\n|\n)/,"\\1#{srch.texts_name}\\2")
        rplc.texts = []
        puts "Remapped #{srch.texts_name}"
      end
    end
    # Handle TypedTextDatabase.cpp
    typed_text_database_cpp = "#{local_path}/src/TypedTextDatabase.cpp"
    typed_text_database = IO.binread(typed_text_database_cpp)
    databases = typed_text_database.scan(/(TEXT_LOCATION_FLASH_PRAGMA\r\nconst touchgfx::TypedText::TypedTextData )(typedText_database_.*?)(\[\].*? =.*?{.*?};\r\n\r\n)/m).collect do |pragma,name,value|
      Database.new(pragma, name, value)
    end
    pragma,dbs,finish = typed_text_database.match(/(TEXT_LOCATION_FLASH_PRAGMA\r\nconst touchgfx::TypedText::TypedTextData\* const typedTextDatabaseArray\[\] TEXT_LOCATION_FLASH_ATTRIBUTE =\r\n{\r\n)(.*?)(\r\n};)/m).captures
    database_array = DatabaseArray.new(pragma, dbs.gsub(/[ \r\n]/,'').split(','), finish)
    header,footer = typed_text_database.match(/(.*?)TEXT_LOCATION_FLASH_PRAGMA.*};(.*)/m).captures
    default_ix = databases.find_index { |db| db.name.match(/_DEFAULT/) }
    # Move _DEFAULT to start of array
    databases.unshift(databases.delete_at(default_ix))
    # Remove duplicates
    databases.each do |rplc|
      databases.each_with_index do |srch, ix|
        if srch.value == rplc.value && srch.name != rplc.name
          puts "Remapped #{srch.name}"
          database_array.databases.map! do |db|
            db == srch.name ? rplc.name : db
          end
          databases.delete_at(ix)
        end
      end
    end

    # Write modified files back

    # Texts.cpp
    File.open(texts_cpp,'wb') { |f| f.write(texts) }

    # TypedTextDatabase.cpp
    File.open(typed_text_database_cpp,'wb') do |f|
      f.write(header)
      databases.each do |db|
        f.write(db.pragma)
        f.write(db.name)
        f.write(db.value)
      end
      f.write("\r\n")
      f.write(database_array.pragma)
      f.write(database_array.databases.map{|x|"    #{x}"}*",\r\n")
      f.write(database_array.finish)
      f.write(footer)
    end

    # Language*.cpp
    files.each do |file|
      file.extern.sort!
      file.extern.uniq!
      if file.texts.length > 0
        parts = file.content.match(/(.*#endif).*(\/\/ Language.*?\r\n).*?\r\n\r\n(.*^{\r\n)( *).*(}.*)/m)
        newstrings = file.texts_string.reject{|x|x.empty?}.inject(""){|txt,x|"#{txt}#{x};\r\n"}
        newmiddle = file.texts * ",\r\n#{parts[4]}"
        externs = file.extern.collect{|e|"extern const touchgfx::Unicode::UnicodeChar #{e}[];"}*"\r\n"
        File.open(file.cpp_file,'wb') { |f| f.write("#{parts[1]}\r\n\r\n#{externs}\r\n\r\n#{parts[2]}#{newstrings}\r\n#{parts[3]}#{parts[4]}#{newmiddle}\r\n#{parts[5]}") }
      else
        parts = file.content.match(/(.*#endif).*(\/\/ Language.*)^TEXT_LOCATION_FLASH_PRAGMA\r\nKEEP extern const touchgfx::Unicode::UnicodeChar\* const.*/m)
        externs = file.extern.collect{|e|"extern const touchgfx::Unicode::UnicodeChar #{e}[];"}*"\r\n"
        File.open(file.cpp_file,'wb') { |f| f.write("#{parts[1]}\r\n\r\n#{externs}\r\n\r\n#{parts[2]}\r\n") }
      end
    end

  end

  if __FILE__ == $0
    if ARGV.count < 7 || ARGV.count > 8
      abort self.banner
    end
    file_name = ARGV.shift
    FontsCpp.font_convert = ARGV.shift
    fonts_output_path = ARGV.shift
    localization_output_path = ARGV.shift
    font_asset_path = ARGV.shift
    $calling_path = ARGV.shift
    remap_identical_texts = ARGV.shift
    data_format = ARGV.shift
    if remap_identical_texts=='A4' || remap_identical_texts=='A8'
      data_format,remap_identical_texts = remap_identical_texts,data_format
    end
    begin
      # Remove old language files in case a language changes ID
      local_path = localization_output_path.gsub('\\','/')
      Dir["#{local_path}/src/Language*.cpp"].each do |language_file|
        FileUtils.rm(language_file)
      end
      Generator.new.run(file_name, fonts_output_path, localization_output_path, font_asset_path, data_format)
      if remap_identical_texts && remap_identical_texts.downcase == 'yes'
        self.save_int_flash(localization_output_path)
      end
    rescue Exception => e
      # Makefiles depend on this file, so remove in case of error.
      FileUtils.rm_f "#{localization_output_path}/include/texts/TextKeysAndLanguages.hpp"
      STDERR.puts e
      abort "an error occurred in converting texts:\r\n#{e}"
    end
  end
end
