#include "chai_library.h"

#include <tinydir.h>
#include <regex>
#include <fstream>
namespace detail {
  std::vector<std::string> grabFiles(const std::string& directory, const std::vector<std::string>& exclusions, const bool recursive = true) {
    std::vector<std::string> files;

    tinydir_dir dir;
    tinydir_open(&dir, directory.c_str());

    while(dir.has_next) {
      tinydir_file file;
      tinydir_readfile(&dir, &file);

      std::regex h_match("[^[:space:]]+[.]h(?:pp)?");
      if(std::none_of(exclusions.cbegin(), exclusions.cend(), [&](const std::string& exc){return exc == std::string(file.name);})) {
        if(recursive && file.is_dir && std::string(file.name) != "." && std::string(file.name) != "..") {
          auto dir_files = grabFiles(directory + file.name + "/", exclusions, recursive);
          files.insert(files.end(), dir_files.begin(), dir_files.end());
        }
        else if(!file.is_dir && std::regex_match(std::string(file.name), h_match) ) {
          files.push_back(std::string(directory + file.name));
        }
      }

      tinydir_next(&dir);
    }
    return files;
  }

  void checkOrCreateFolder(const std::string& path, const std::string& folder) {
    tinydir_dir dir;
    tinydir_open(&dir, path.c_str());

    while(dir.has_next) {
      tinydir_file file;
      tinydir_readfile(&dir, &file);
      if(file.name == folder.c_str())
        return;
      tinydir_next(&dir);
    }

    mkdir(std::string(folder).c_str(), S_IRWXU | S_IRWXG | S_IRWXO);
  }
}

ChaiLibrary::ChaiLibrary(const std::string& name, const std::string& location, const std::string& output,
  const std::vector<std::string>& includes, const std::vector<std::string>& exculsions,
  const std::string ns, const bool parse_verbosely, const bool process_verbosely) :
  name(name), location(location), output(output), includes(includes), exclusions(exculsions),
  _namespace(ns), verbose_processing(process_verbosely) {

  config.fast_preprocessing(false);

  config.set_flags(cppast::cpp_standard::cpp_14, flags);
  for(auto i : includes) {
    config.add_include_dir(i);
  }

  processor = ChaiObjectProcessor(config, parse_verbosely);

  exclusions.push_back("generated");
}


void ChaiLibrary::process() {
  auto files = detail::grabFiles(location, exclusions);
  processor.processObjects(files, verbose_processing);

  auto registrations = processor.generateRegistrations(_namespace, location, verbose_processing);

  detail::checkOrCreateFolder(location, output);
  std::ofstream output_file(output + "/" + _namespace + "_registrations.h");
  output_file << registrations.str();
  output_file.flush();
  output_file.close();
}
