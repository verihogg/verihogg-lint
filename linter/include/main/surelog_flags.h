#pragma once

#include <array>
#include <cstdint>
#include <string_view>

// Whitelist of CLI flags accepted by Surelog 1.86. Used by cli::ParseArgs to
// distinguish a misspelled flag from one we should forward to Surelog.
//
// When upgrading Surelog, refresh the lists below:
//   verihogg-lint --surelog-help
// Reclassify any new flags by their argument arity (no-arg / one-arg /
// two-arg / prefix-with-value).

namespace cli {

inline constexpr std::array kSurelogNoArgFlags = {
    std::string_view{"-batch"},
    std::string_view{"-createcache"},
    std::string_view{"-diffcompunit"},
    std::string_view{"-elabuhdm"},
    std::string_view{"-fileunit"},
    std::string_view{"-filtercomments"},
    std::string_view{"-filterdirectives"},
    std::string_view{"-filterprotected"},
    std::string_view{"-formal"},
    std::string_view{"-gc"},
    std::string_view{"-init"},
    std::string_view{"-lineoffsetascomments"},
    std::string_view{"-link"},
    std::string_view{"-lowmem"},
    std::string_view{"-nobuiltin"},
    std::string_view{"-nocache"},
    std::string_view{"-nocomp"},
    std::string_view{"-noelab"},
    std::string_view{"-nogc"},
    std::string_view{"-nohash"},
    std::string_view{"-noinfo"},
    std::string_view{"-nonote"},
    std::string_view{"-noparse"},
    std::string_view{"-noprecompiledcache"},
    std::string_view{"-nostdout"},
    std::string_view{"-nouhdm"},
    std::string_view{"-nowarning"},
    std::string_view{"-nowritecache"},
    std::string_view{"-outputlineinfo"},
    std::string_view{"-parse"},
    std::string_view{"-parseonly"},
    std::string_view{"-pploc"},
    std::string_view{"-ppextra_loc"},
    std::string_view{"-profile"},
    std::string_view{"-replay"},
    std::string_view{"-sepcomp"},
    std::string_view{"-sv"},
    std::string_view{"-sverilog"},
    std::string_view{"-synth"},
    std::string_view{"-verbose"},
    std::string_view{"-writepp"},
};

inline constexpr std::array kSurelogOneArgFlags = {
    std::string_view{"-bb_inst"}, std::string_view{"-bb_mod"},
    std::string_view{"-builtin"}, std::string_view{"-cache"},
    std::string_view{"-cd"},      std::string_view{"-cfg"},
    std::string_view{"-cfgfile"}, std::string_view{"-d"},
    std::string_view{"-exe"},     std::string_view{"-f"},
    std::string_view{"-L"},       std::string_view{"-l"},
    std::string_view{"-map"},     std::string_view{"-mp"},
    std::string_view{"-mt"},      std::string_view{"-o"},
    std::string_view{"-odir"},    std::string_view{"-split"},
    std::string_view{"-top"},     std::string_view{"-v"},
    std::string_view{"-wd"},      std::string_view{"-writeppfile"},
    std::string_view{"-y"},
};

inline constexpr std::array kSurelogTwoArgFlags = {
    std::string_view{"-cmd_ign"},
    std::string_view{"-cmd_mrg"},
    std::string_view{"-cmd_ren"},
    std::string_view{"-remap"},
};

inline constexpr std::array kSurelogPrefixFlags = {
    std::string_view{"-D"},          std::string_view{"-I"},
    std::string_view{"-P"},          std::string_view{"-pvalue+"},
    std::string_view{"-timescale="},
};

inline constexpr std::array kSurelogLongOneArgFlags = {
    std::string_view{"--Mdir"},
    std::string_view{"--threads"},
    std::string_view{"--top-module"},
};

inline constexpr std::array kSurelogLongPrefixFlags = {
    std::string_view{"--disable-feature="},
    std::string_view{"--enable-feature="},
};

enum class SurelogFlagKind : std::uint8_t {
  Unknown,
  NoArg,
  OneArg,
  TwoArg,
};

inline auto ClassifySurelogFlag(std::string_view arg) -> SurelogFlagKind {
  if (!arg.empty() && arg.front() == '+') {
    return SurelogFlagKind::NoArg;
  }
  for (auto f : kSurelogNoArgFlags) {
    if (arg == f) {
      return SurelogFlagKind::NoArg;
    }
  }
  for (auto f : kSurelogOneArgFlags) {
    if (arg == f) {
      return SurelogFlagKind::OneArg;
    }
  }
  for (auto f : kSurelogTwoArgFlags) {
    if (arg == f) {
      return SurelogFlagKind::TwoArg;
    }
  }
  for (auto f : kSurelogLongOneArgFlags) {
    if (arg == f) {
      return SurelogFlagKind::OneArg;
    }
  }
  for (auto p : kSurelogPrefixFlags) {
    if (arg.starts_with(p)) {
      return SurelogFlagKind::NoArg;
    }
  }
  for (auto p : kSurelogLongPrefixFlags) {
    if (arg.starts_with(p)) {
      return SurelogFlagKind::NoArg;
    }
  }
  return SurelogFlagKind::Unknown;
}

}  // namespace cli