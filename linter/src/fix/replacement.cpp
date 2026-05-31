#include "fix/replacement.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <ranges>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

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
  const auto file_size = in.tellg();
  if (file_size < 0) {
    std::cerr << "autofix: cannot determine size of: " << filepath << "\n";
    return false;
  }
  std::string source(static_cast<size_t>(file_size), '\0');
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
    bak.write(source.data(), static_cast<std::streamsize>(source.size()));
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

namespace {

auto splitLines(const std::string& text) -> std::vector<std::string> {
  std::vector<std::string> lines;
  std::istringstream ss(text);
  std::string line;
  while (std::getline(ss, line)) {
    if (!line.empty() && line.back() == '\r') {
      line.pop_back();
    }
    lines.push_back(std::move(line));
  }
  return lines;
}

auto makeRelativePath(const std::string& filepath) -> std::string {
  std::error_code ec;
  const auto rel =
      std::filesystem::relative(filepath, std::filesystem::current_path(), ec);
  if (!ec && !rel.empty()) {
    return rel.generic_string();
  }
  return filepath;
}

auto computeLCS(const std::vector<std::string>& a,
                const std::vector<std::string>& b)
    -> std::vector<std::pair<int, int>> {
  const int n = static_cast<int>(a.size());
  const int m = static_cast<int>(b.size());

  std::vector<std::vector<int>> dp(n + 1, std::vector<int>(m + 1, 0));
  for (int i = 1; i <= n; ++i) {
    for (int j = 1; j <= m; ++j) {
      dp.at(i).at(j) = (a.at(i - 1) == b.at(j - 1))
                           ? dp.at(i - 1).at(j - 1) + 1
                           : std::max(dp.at(i - 1).at(j), dp.at(i).at(j - 1));
    }
  }

  std::vector<std::pair<int, int>> matches;
  for (int i = n, j = m; i > 0 && j > 0;) {
    if (a.at(i - 1) == b.at(j - 1)) {
      matches.emplace_back(i - 1, j - 1);
      --i;
      --j;
    } else if (dp.at(i - 1).at(j) >= dp.at(i).at(j - 1)) {
      --i;
    } else {
      --j;
    }
  }
  std::ranges::reverse(matches);
  return matches;
}

struct DiffEdit {
  enum class Kind : std::uint8_t { Context, Remove, Add } kind;
  std::string text;
  int orig_lineno;
  int new_lineno;
};

auto buildEdits(const std::vector<std::string>& orig,
                const std::vector<std::string>& fixed)
    -> std::vector<DiffEdit> {
  const auto matches = computeLCS(orig, fixed);

  std::vector<DiffEdit> edits;
  edits.reserve(orig.size() + fixed.size());
  int oi = 0, ni = 0;

  for (const auto& [om, nm] : matches) {
    while (oi < om) {
      edits.push_back({.kind = DiffEdit::Kind::Remove,
                       .text = orig.at(oi),
                       .orig_lineno = oi + 1,
                       .new_lineno = -1});
      ++oi;
    }
    while (ni < nm) {
      edits.push_back({.kind = DiffEdit::Kind::Add,
                       .text = fixed.at(ni),
                       .orig_lineno = -1,
                       .new_lineno = ni + 1});
      ++ni;
    }
    edits.push_back({.kind = DiffEdit::Kind::Context,
                     .text = orig.at(oi),
                     .orig_lineno = oi + 1,
                     .new_lineno = ni + 1});
    ++oi;
    ++ni;
  }
  while (std::cmp_less(oi, orig.size())) {
    edits.push_back({.kind = DiffEdit::Kind::Remove,
                     .text = orig.at(oi),
                     .orig_lineno = oi + 1,
                     .new_lineno = -1});
    ++oi;
  }
  while (std::cmp_less(ni, fixed.size())) {
    edits.push_back({.kind = DiffEdit::Kind::Add,
                     .text = fixed.at(ni),
                     .orig_lineno = -1,
                     .new_lineno = ni + 1});
    ++ni;
  }
  return edits;
}

struct Hunk {
  int orig_start = 0;
  int orig_count = 0;
  int new_start = 0;
  int new_count = 0;
  std::vector<DiffEdit> edits;
};

auto buildHunks(const std::vector<DiffEdit>& edits, int context)
    -> std::vector<Hunk> {
  const int n = static_cast<int>(edits.size());

  std::vector<int> changes;
  for (int i = 0; i < n; ++i) {
    if (edits.at(i).kind != DiffEdit::Kind::Context) {
      changes.push_back(i);
    }
  }
  if (changes.empty()) {
    return {};
  }

  std::vector<std::pair<int, int>> windows;
  int ws = std::max(0, changes.at(0) - context);
  int we = std::min(n - 1, changes.at(0) + context);

  for (int k = 1; std::cmp_less(k, changes.size()); ++k) {
    const int ns = std::max(0, changes.at(k) - context);
    const int ne = std::min(n - 1, changes.at(k) + context);
    if (ns <= we + 1) {
      we = ne;
    } else {
      windows.emplace_back(ws, we);
      ws = ns;
      we = ne;
    }
  }
  windows.emplace_back(ws, we);

  std::vector<Hunk> hunks;
  for (const auto& [rs, re] : windows) {
    Hunk h;
    int orig_start_found = -1;
    int new_start_found = -1;

    for (int i = rs; i <= re; ++i) {
      const auto& e = edits.at(i);
      h.edits.push_back(e);
      if (e.kind != DiffEdit::Kind::Add) {
        if (orig_start_found == -1) {
          orig_start_found = e.orig_lineno;
        }
        ++h.orig_count;
      }
      if (e.kind != DiffEdit::Kind::Remove) {
        if (new_start_found == -1) {
          new_start_found = e.new_lineno;
        }
        ++h.new_count;
      }
    }

    if (orig_start_found == -1) {
      h.orig_start = 0;
      for (int i = rs - 1; i >= 0; --i) {
        if (edits.at(i).kind != DiffEdit::Kind::Add) {
          h.orig_start = edits.at(i).orig_lineno;
          break;
        }
      }
    } else {
      h.orig_start = orig_start_found;
    }

    if (new_start_found == -1) {
      h.new_start = 0;
      for (int i = rs - 1; i >= 0; --i) {
        if (edits.at(i).kind != DiffEdit::Kind::Remove) {
          h.new_start = edits.at(i).new_lineno;
          break;
        }
      }
    } else {
      h.new_start = new_start_found;
    }

    hunks.push_back(std::move(h));
  }
  return hunks;
}

void emitHunk(std::ostream& out, const Hunk& h) {
  out << "@@ -" << h.orig_start;
  if (h.orig_count != 1) {
    out << ',' << h.orig_count;
  }
  out << " +" << h.new_start;
  if (h.new_count != 1) {
    out << ',' << h.new_count;
  }
  out << " @@\n";

  for (const auto& e : h.edits) {
    switch (e.kind) {
      case DiffEdit::Kind::Context:
        out << ' ' << e.text << '\n';
        break;
      case DiffEdit::Kind::Remove:
        out << '-' << e.text << '\n';
        break;
      case DiffEdit::Kind::Add:
        out << '+' << e.text << '\n';
        break;
    }
  }
}

}  // namespace

void FileReplacements::printDiff(std::ostream& out) const {
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
    const auto file_size = in.tellg();
    if (file_size < 0) {
      std::cerr << "autofix[dry-run]: cannot determine size of: " << filepath
                << "\n";
      continue;
    }
    std::string source(static_cast<size_t>(file_size), '\0');
    in.seekg(0, std::ios::beg);
    in.read(source.data(), static_cast<std::streamsize>(source.size()));

    std::string fixed;
    try {
      fixed = repls.apply(source);
    } catch (const std::exception& e) {
      std::cerr << "autofix[dry-run]: apply failed: " << e.what() << "\n";
      continue;
    }

    const auto orig_lines = splitLines(source);
    const auto fixed_lines = splitLines(fixed);
    const auto edits = buildEdits(orig_lines, fixed_lines);
    const auto hunks = buildHunks(edits, 3);

    if (hunks.empty()) {
      continue;
    }

    const std::string rel = makeRelativePath(filepath);
    out << "--- a/" << rel << "\n";
    out << "+++ b/" << rel << "\n";
    for (const auto& hunk : hunks) {
      emitHunk(out, hunk);
    }
  }
}
