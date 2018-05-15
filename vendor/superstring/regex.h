#ifndef REGEX_H_
#define REGEX_H_

#include "optional.h"
#include <string>

struct pcre2_real_code_16;
struct pcre2_real_match_data_16;
struct BuildRegexResult;

class Regex {
  pcre2_real_code_16 *code;
  Regex(pcre2_real_code_16 *);

 public:
  Regex();
  Regex(const char16_t *, uint32_t, std::u16string *error_message, bool ignore_case = false);
  Regex(const std::u16string &, std::u16string *error_message, bool ignore_case = false);
  Regex(Regex &&);
  ~Regex();

  class MatchData {
    pcre2_real_match_data_16 *data;
    friend class Regex;

   public:
    MatchData(const Regex &);
    ~MatchData();
  };

  struct MatchResult {
    enum {
      None,
      Error,
      Partial,
      Full,
    } type;

    size_t start_offset;
    size_t end_offset;
  };

  enum MatchOptions {
    None = 0,
    IsBeginningOfLine = 1,
    IsEndOfLine = 2,
    IsEndSearch = 4,
  };

  MatchResult match(const char16_t *data, size_t length, MatchData &, unsigned options = 0) const;
};

struct BuildRegexResult {
  optional<Regex> regex;
  std::u16string error_message;
};

#endif  // REGX_H_
