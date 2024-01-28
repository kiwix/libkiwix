#!/usr/bin/env python3
# Copyright (c) 2020 Matthieu Gautier <mgautier@kymeria.fr>
#
# This file is part of libkiwix.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.

from pathlib import Path
import json

script_path = Path(__file__)

resource_file = script_path.parent / "i18n_resources_list.txt"
translation_dir = script_path.parent / "skin/i18n"
language_list_relpath = "skin/languages.js"

def get_translation_info(filepath):
    lang_code = Path(filepath).stem
    with open(filepath, 'r', encoding="utf-8") as f:
        content = json.load(f)
        lang_name = content.get("name")
        return lang_code, lang_name

language_list = []
json_files = translation_dir.glob("*.json")
with open(resource_file, 'w', encoding="utf-8") as f:
    for i18n_file in sorted(translation_dir.glob("*.json")):
        if i18n_file.name == "qqq.json":
            continue
        print("Processing", i18n_file.name)
        if i18n_file.name != "test.json":
            lang_code, lang_name = get_translation_info(i18n_file)
            if lang_name:
                language_list.append((lang_code, lang_name))
            else:
                print(f"Warning: missing 'name' in {i18n_file.name}")
        f.write(str(i18n_file.relative_to(script_path.parent)) + '\n')

def make_language_entry(lang_info_tuple):
    iso_code, self_name = lang_info_tuple
    return dict(iso_code=iso_code, self_name=self_name)

language_list = [make_language_entry(x) for x in sorted(language_list)]
language_list_jsobj_str = json.dumps(language_list,
                                     indent=2,
                                     ensure_ascii=False)
print("Saving", language_list_relpath)
fullpath = script_path.parent / language_list_relpath
with open(fullpath, 'w', encoding="utf-8") as f:
    f.write("const uiLanguages = " + language_list_jsobj_str)
