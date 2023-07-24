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

require 'lib/file_io'
require 'lib/emitters/template'
require 'lib/emitters/text_keys_and_languages_hpp'
require 'lib/emitters/texts_cpp'
require 'lib/emitters/languages_cpp'
require 'lib/emitters/unicodes_txt'
require 'lib/emitters/fonts_cpp'
require 'lib/emitters/application_font_provider_hpp'
require 'lib/emitters/application_font_provider_cpp'
require 'lib/emitters/typed_text_database_hpp'
require 'lib/emitters/typed_text_database_cpp'

class Outputter
  def initialize(text_entries, typographies, localization_output_directory, fonts_output_directory, font_asset_path, data_format)
    @text_entries = text_entries
    @typographies = typographies
    @localization_output_directory = localization_output_directory
    @fonts_output_directory = fonts_output_directory
    @font_asset_path = font_asset_path
    @data_format = data_format
  end

  def run

    [ UnicodesTxt,
      ApplicationFontProviderCpp,
      ApplicationFontProviderHpp ].each { |template| template.new(@text_entries, @typographies, @fonts_output_directory).run }

    [ TextKeysAndLanguages,
      TextsCpp,
      LanguagesCpp,
      TypedTextDatabaseHpp,
      TypedTextDatabaseCpp ].each { |template| template.new(@text_entries, @typographies, @localization_output_directory).run }

    FontsCpp.new(@text_entries, @typographies, @fonts_output_directory, @font_asset_path, @data_format).run

  end
end

