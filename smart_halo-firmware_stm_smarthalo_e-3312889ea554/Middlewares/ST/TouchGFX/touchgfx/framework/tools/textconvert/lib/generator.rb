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

require 'lib/text_entries_excel_reader'
require 'lib/typographies_excel_reader'
require 'lib/sanitizer'
require 'lib/outputter'

class Generator
  def run(file_name, output_path, text_output_path, font_asset_path, data_format)
    text_entries = TextEntriesExcelReader.new(file_name).run
    typographies = TypographiesExcelReader.new(file_name).run
    Sanitizer.new(text_entries, typographies).run
    Outputter.new(text_entries, typographies, text_output_path, output_path, font_asset_path, data_format).run
  end
end
