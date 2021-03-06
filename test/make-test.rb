#!/usr/bin/env ruby

puts <<-END
/* generated by #{$0}#{ARGV.size > 0 ? ' ' + ARGV.join(' ') : ''} */

#include <stdarg.h>
#include <stdio.h>

#include <regex_compile.h>
#include <regex_parse.h>

#ifdef __GNUC__
void success(const char *source, const char *format, ...)
  __attribute__ ((format(printf, 2, 3)));
void fail(const char *source, const char *format, ...)
  __attribute__ ((format(printf, 2, 3)));
#endif

void success(const char *source, const char *format, ...) {
  va_list ap;
  va_start(ap, format);
  printf("%s [\\x1b[32mSUCCESS\\x1b[0m] ", source);
  vprintf(format, ap);
  printf("\\n");
  va_end(ap);
}

void fail(const char *source, const char *format, ...) {
  va_list ap;
  va_start(ap, format);
  printf("%s [\\x1b[31mFAIL   \\x1b[0m] ", source);
  vprintf(format, ap);
  printf("\\n");
  va_end(ap);
}

int test(const char *source, const char *pattern, const char *string,
  int nmatches, ...) {
  RegexNode *root;
  RegexProgram *program;
  const char *matches[20] = {0};
  int result = 0;
  va_list ap;

  // parse pattern
  if (!(root = regexParse(pattern))) {
    fail(source, "regexParse() failed");
    return -1;
  }

  // compile parsed pattern
  program = regexCompileNode(root);
  regexParseFree(root);
  if (!program) {
    fail(source, "regexCompile() failed");
    return -1;
  }

  // run program on string
  if ((result = regexProgramRun(program, string,
    matches, sizeof (matches) / sizeof (matches[0]))) < 0) {
    fail(source, "regexProgramRun() failed");
    regexCompileFree(program);
    return -1;
  }

  va_start(ap, nmatches);
  if (result > 0) {
    if (nmatches > 0) {
      success(source, "/%s/ =~ \\"%s\\"", pattern, string);
      result = 0;
      for (int i = 0; i + 1 < nmatches
        && i + 1 < sizeof (matches) / sizeof (matches[0]); i += 2) {
        int begin = va_arg(ap, int), end = va_arg(ap, int);
        if ((begin == -1 || begin == matches[i] - string)
          && (end == -1 || end == matches[i + 1] - string)) {
          // success(source, "(%d,%d)", begin, end);
        } else if (matches[i] && matches[i + 1]) {
          fail(source, "expected (%d,%d), got (%d,%d)", begin, end,
            (int)(matches[i] - string), (int)(matches[i + 1] - string));
          result = -1;
        } else {
          fail(source, "expected (%d,%d), got (NULL,NULL)", begin, end);
          result = -1;
        }
      }
    } else {
      fail(source, "/%s/ =~ \\"%s\\"", pattern, string);
      result = -1;
    }
  } else if (result == 0) {
    if (nmatches == 0)
      success(source, "/%s/ !~ \\"%s\\"", pattern, string);
    else {
      fail(source, "/%s/ !~ \\"%s\\"", pattern, string);
      result = -1;
    }
  }

  va_end(ap);
  regexCompileFree(program);
  return result;
}

int main(int argc, char *argv[]) {
  int nerrors = 0;
END

filename = nil
previous = nil
ntests   = 0

ARGF.each do |line|
  if ARGF.filename != filename
    filename = ARGF.filename
    ARGF.lineno = 1
  end

  line = line.sub(/^:[^:]*:/, '')
  next unless line =~ /^[{BEASKL]+/

  options, pattern, string, captures = line.chomp.split(/\t+/)
  string   = ''       if string  == 'NULL'
  pattern  = previous if pattern == 'SAME'
  previous = pattern
  pattern  = pattern.gsub('\\', "\\\\\\\\") unless options.include?('$')
  string   = string .gsub('\\', "\\\\\\\\") unless options.include?('$')
  captures = captures == 'NOMATCH' \
    ? captures
    : captures
        .scan(/\((.*?),(.*?)\)/)
        .flatten
        .map {|offset| offset == '?' ? -1 : offset.to_i }

  puts <<-END
  nerrors += test("#{ARGF.filename}:#{'%03d' % ARGF.lineno}", "#{pattern}", "#{string}",
    #{captures == 'NOMATCH' ? 0 : "#{captures.size}, #{captures.join(', ')}"});
END
  ntests += 1
end

puts <<-END
  printf("#{ntests} test(s), %d error(s).\\n", -nerrors);
  return 0;
}
END
