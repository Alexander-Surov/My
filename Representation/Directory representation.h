#ifndef DIR_REPR
#define DIR_REPR

#include <filesystem>
#include <string>
#include <vector>
#include <ranges>
#include <algorithm>


struct DefaultDesigner {
  static constexpr std::string blank  = " ";
  static constexpr std::string vert   = "│";
  static constexpr std::string sub    = "├";
  static constexpr std::string corner = "└";
  static constexpr std::string horiz  = "─";
  static constexpr std::string half   = "╴";
};


struct Line {
  std::string name;
  bool is_last;
  std::vector<bool> is_vert;  // reversed
};


std::vector<Line> construct_repr(const std::filesystem::path& directory) {
  namespace fs = std::filesystem;

  std::vector<Line> lines;

  std::vector<fs::path> subdirs;
  std::vector<fs::path> files;

  for (auto&& entry : fs::directory_iterator(directory)) {
    if (fs::is_directory(entry)) {
      subdirs.push_back(entry.path());
    } else {
      files.push_back(entry.path());
    }
  }

  std::ranges::sort(subdirs, {}, [](auto&& d) { return d.filename().string(); });
  std::ranges::sort(files, {}, [](auto&& f) { return f.filename().string(); });

  for (auto&& [i, subdir] : subdirs | std::views::enumerate) {
    lines.emplace_back(subdir.filename().string(), false);
    std::vector<Line> subs = construct_repr(subdir);

    for (auto&& line : subs) {
      line.is_vert.push_back(!files.empty() || i != subdirs.size() - 1);
    }

    if (files.empty() && i == subdirs.size() - 1) {
      lines.back().is_last = true;
    }

    if (!subs.empty()) {
      subs.back().is_last = true;
    }
    lines.append_range(subs);
  }

  for (auto&& file : files) {
    lines.emplace_back(file.filename().string(), false);
  }

  if (!files.empty()) {
    lines.back().is_last = true;
  }

  return lines;
}


std::string directory_repr(const std::filesystem::path& root) {
  namespace fs = std::filesystem;

  if (!fs::exists(root) || !fs::is_directory(root)) {
    return "";
  }

  std::vector<Line> lines = construct_repr(root);

  std::string repr = root.filename().string() + '\n';

  for (auto&& line : lines) {
    for (auto&& is_vert : line.is_vert | std::views::reverse) {
      repr += (is_vert ? DefaultDesigner::vert : DefaultDesigner::blank);
      repr += DefaultDesigner::blank + DefaultDesigner::blank;
    }

    repr += (line.is_last ? DefaultDesigner::corner : DefaultDesigner::sub) +
            DefaultDesigner::horiz + DefaultDesigner::half;
    repr += line.name + '\n';
  }

  return repr;
}


#endif