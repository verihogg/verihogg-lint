#include "fix/replacement.h"

#include <yaml-cpp/yaml.h>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <ranges>
#include <sstream>
#include <stdexcept>
#include <string>

auto fixItToReplacement(const FixIt& fix, FixSourceManager& sm) -> Replacement {
  fix.range.validate();

  Replacement r;
  r.filename = fix.range.begin.filename;

  const unsigned begin_off =
      sm.getOffset(r.filename, fix.range.begin.line, fix.range.begin.col);
  r.offset = begin_off;

  if (fix.kind == FixKind::Insertion) {
    r.length = 0;
    r.text = fix.replacement;
  } else {
    const unsigned end_off =
        sm.getOffset(r.filename, fix.range.end.line, fix.range.end.col);
    if (end_off < begin_off) {
      throw std::out_of_range("fixItToReplacement: end offset (" +
                              std::to_string(end_off) + ") < begin offset (" +
                              std::to_string(begin_off) + ") in " + r.filename);
    }
    r.length = end_off - begin_off;
    r.text = (fix.kind == FixKind::Replacement) ? fix.replacement : "";
  }
  return r;
}

auto Replacements::overlaps(const Replacement& a, const Replacement& b)
    -> bool {
  if (a.length == 0 && b.length == 0 && a.offset == b.offset) {
    return false;
  }
  const unsigned a_end = a.offset + a.length;
  const unsigned b_end = b.offset + b.length;
  return (a_end > b.offset) && (b_end > a.offset);
}

auto Replacements::add(const Replacement& r, std::string* conflict_msg)
    -> bool {
  for (const auto& existing : repls_) {
    if (existing.offset == r.offset && existing.length == r.length &&
        existing.text == r.text) {
      return true;
    }
    if (overlaps(existing, r)) {
      if (conflict_msg) {
        *conflict_msg = "conflict: [" + existing.rule_id +
                        "] offset=" + std::to_string(existing.offset) +
                        " len=" + std::to_string(existing.length) + " vs [" +
                        r.rule_id + "] offset=" + std::to_string(r.offset) +
                        " len=" + std::to_string(r.length);
      }
      return false;
    }
  }
  const auto pos = std::ranges::lower_bound(repls_, r);
  repls_.insert(pos, r);
  return true;
}

auto Replacements::apply(const std::string& source) const -> std::string {
  std::string result = source;
  for (const auto& repl : repls_) {
    if (repl.offset > result.size()) {
      throw std::out_of_range("Replacement::apply: offset " +
                              std::to_string(repl.offset) + " > size " +
                              std::to_string(result.size()) +
                              " (rule: " + repl.rule_id + ")");
    }
    if (repl.offset + repl.length > result.size()) {
      throw std::out_of_range("Replacement::apply: offset+length " +
                              std::to_string(repl.offset + repl.length) +
                              " > size " + std::to_string(result.size()) +
                              " (rule: " + repl.rule_id + ")");
    }
    result.replace(repl.offset, repl.length, repl.text);
  }
  return result;
}

auto FileReplacements::add(const Replacement& r, std::string* conflict_msg)
    -> bool {
  return by_file_[r.filename].add(r, conflict_msg);
}

auto FileReplacements::applyToFile(const std::string& filepath,
                                   const std::string& backup_suffix) const
    -> bool {
  const auto it = by_file_.find(filepath);
  if (it == by_file_.end() || it->second.empty()) {
    return true;
  }

  std::ifstream in(filepath, std::ios::binary);
  if (!in.is_open()) {
    std::cerr << "autofix: cannot read: " << filepath << "\n";
    return false;
  }
  in.seekg(0, std::ios::end);
  std::string source(static_cast<size_t>(in.tellg()), '\0');
  in.seekg(0, std::ios::beg);
  in.read(source.data(), static_cast<std::streamsize>(source.size()));
  in.close();

  std::string fixed;
  try {
    fixed = it->second.apply(source);
  } catch (const std::exception& e) {
    std::cerr << "autofix: apply failed for " << filepath << ": " << e.what()
              << "\n";
    return false;
  }

  if (!backup_suffix.empty()) {
    const std::string backup_path = filepath + backup_suffix;
    std::ofstream bak(backup_path, std::ios::binary | std::ios::trunc);
    if (!bak.is_open()) {
      std::cerr << "autofix: cannot create backup: " << backup_path << "\n";
      return false;
    }
    bak << source;
    if (!bak.good()) {
      std::cerr << "autofix: backup write failed: " << backup_path << "\n";
      return false;
    }
  }

  const std::string tmp_path = filepath + ".fix_tmp";
  {
    std::ofstream out(tmp_path, std::ios::binary | std::ios::trunc);
    if (!out.is_open()) {
      std::cerr << "autofix: cannot create temp file: " << tmp_path << "\n";
      return false;
    }
    out << fixed;
    if (!out.good()) {
      std::cerr << "autofix: temp write failed: " << tmp_path << "\n";
      return false;
    }
  }

  std::error_code ec;
  std::filesystem::rename(tmp_path, filepath, ec);
  if (ec) {
    std::cerr << "autofix: rename failed for " << filepath << " ("
              << ec.message() << ")\n";
    std::filesystem::remove(tmp_path, ec);
    return false;
  }
  return true;
}

void FileReplacements::applyAll(std::vector<std::string>* fixed,
                                std::vector<std::string>* failed,
                                const std::string& backup_suffix) const {
  for (const auto& [filepath, repls] : by_file_) {
    if (repls.empty()) {
      continue;
    }
    if (applyToFile(filepath, backup_suffix)) {
      if (fixed) {
        fixed->push_back(filepath);
      }
    } else {
      if (failed) {
        failed->push_back(filepath);
      }
    }
  }
}

void FileReplacements::printDiff() const {
  for (const auto& [filepath, repls] : by_file_) {
    if (repls.empty()) {
      continue;
    }

    std::ifstream in(filepath, std::ios::binary);
    if (!in.is_open()) {
      std::cerr << "autofix[dry-run]: cannot read: " << filepath << "\n";
      continue;
    }
    in.seekg(0, std::ios::end);
    std::string source(static_cast<size_t>(in.tellg()), '\0');
    in.seekg(0, std::ios::beg);
    in.read(source.data(), static_cast<std::streamsize>(source.size()));

    std::string fixed;
    try {
      fixed = repls.apply(source);
    } catch (const std::exception& e) {
      std::cerr << "autofix[dry-run]: apply failed: " << e.what() << "\n";
      continue;
    }

    std::cout << "--- " << filepath << " (original)\n";
    std::cout << "+++ " << filepath << " (fixed)\n";

    std::istringstream src_ss(source);
    std::istringstream fix_ss(fixed);
    std::string src_line;
    std::string fix_line;
    unsigned lineno = 1;

    const auto stripCR = [](std::string& s) {
      if (!s.empty() && s.back() == '\r') {
        s.pop_back();
      }
    };

    bool src_ok = true;
    bool fix_ok = true;
    while (src_ok || fix_ok) {
      src_ok = static_cast<bool>(std::getline(src_ss, src_line));
      fix_ok = static_cast<bool>(std::getline(fix_ss, fix_line));
      if (!src_ok && !fix_ok) {
        break;
      }

      if (src_ok) {
        stripCR(src_line);
      }
      if (fix_ok) {
        stripCR(fix_line);
      }

      const std::string& src_display = src_ok ? src_line : "";
      const std::string& fix_display = fix_ok ? fix_line : "";

      if (src_display != fix_display) {
        std::cout << "@@ line " << lineno << " @@\n";
        if (src_ok) {
          std::cout << "- " << src_display << "\n";
        }
        if (fix_ok) {
          std::cout << "+ " << fix_display << "\n";
        }
      }
      ++lineno;
    }
  }
}

auto FileReplacements::exportToYaml(const std::string& output_path) const
    -> bool {
  const auto parent = std::filesystem::path(output_path).parent_path();
  if (!parent.empty()) {
    std::error_code ec;
    std::filesystem::create_directories(parent, ec);
    if (ec) {
      std::cerr << "autofix: cannot create directory: " << parent.string()
                << " (" << ec.message() << ")\n";
      return false;
    }
  }

  YAML::Emitter out;
  out << YAML::BeginMap;
  out << YAML::Key << "Replacements" << YAML::Value << YAML::BeginSeq;

  for (const auto& [filepath, repls] : by_file_) {
    for (const auto& r : std::views::reverse(repls)) {
      out << YAML::BeginMap;
      out << YAML::Key << "FilePath" << YAML::Value << filepath;
      out << YAML::Key << "Offset" << YAML::Value << r.offset;
      out << YAML::Key << "Length" << YAML::Value << r.length;
      out << YAML::Key << "ReplacementText" << YAML::Value << r.text;
      if (!r.rule_id.empty()) {
        out << YAML::Comment("rule: " + r.rule_id);
      }
      out << YAML::EndMap;
    }
  }

  out << YAML::EndSeq << YAML::EndMap;

  std::ofstream file(output_path, std::ios::binary);
  if (!file.is_open()) {
    std::cerr << "autofix: cannot open export file: " << output_path << "\n";
    return false;
  }
  file << out.c_str();
  return file.good();
}
