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

class TextKeysAndLanguages < Template
  def countries
    text_entries.languages.map { |language| language.upcase }.join(",\n    ")
  end
  def texts
    text_entries.entries.map(&:cpp_text_id)
  end
  def input_path
    File.join(root_dir,'Templates','TextKeysAndLanguages.hpp.temp')
  end
  def output_path
    'include/texts/TextKeysAndLanguages.hpp'
  end
end
