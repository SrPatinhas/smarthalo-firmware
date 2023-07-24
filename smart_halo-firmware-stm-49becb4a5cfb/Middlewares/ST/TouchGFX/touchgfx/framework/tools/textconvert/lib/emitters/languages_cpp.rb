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


class LanguagesCpp
  def initialize(text_entries, typographies, output_directory)
    @text_entries = text_entries
    @output_directory = output_directory
  end
  def run
    @text_entries.languages.each do |language|
      LanguageXxCpp.new(@text_entries, @output_directory, language).run
    end
  end
end

class LanguageXxCpp < Template
  Presenter = Struct.new(:text_id, :translation, :unicodes)

  def initialize(text_entries, output_directory, language)
    @language = language
    super(text_entries, [], output_directory)
  end

  def language
    @language
  end

  def entries
    entries = text_entries

    entries = handle_no_entries(entries, "DO_NOT_USE")
    present(entries)
  end

  def entries_texts_const_initialization
    entries.map { |entry| "    #{entry.text_id}_#{language}" }.join(",\n")
  end

#  def entries_s
#    entries = text_entries.entries_with_1_substitution
#    entries = handle_no_entries(entries, "DO_NOT_USE_S")
#    present(entries)
#  end

# def entries_s_texts_const_initialization
#   entries_s.map { |entry| "#{entry.text_id}_#{language}" }.join(",\n")
# end

# def entries_ss
#   entries = text_entries.entries_with_2_substitutions
#   entries = handle_no_entries(entries, "DO_NOT_USE_SS")
#   present(entries)
# end

# def entries_ss_texts_const_initialization
#   entries_ss.map { |entry| "#{entry.text_id}_#{language}" }.join(",\n")
# end

  def input_path
    File.join(root_dir,'Templates','LanguageXX.cpp.temp')
  end

  def output_path
    "src/Language#{language}.cpp"
  end

  private

  def handle_no_entries(entries, text)
    if entries.empty?
       empty_entry = TextEntry.new(text, "typography")
       empty_entry.add_translation(language, "")
       [empty_entry]
    else
      entries
    end
  end

  def present(entries)
    entries.map do |entry|
      Presenter.new(entry.cpp_text_id, entry.translation_in(language).to_cpp, ( entry.translation_in(language).unicodes.map { |u| '0x' + u.to_s(16) } << '0x0' ) .join(', ') )
    end
  end

end
