/* generated by ./make-test.rb testdata/basic.dat testdata/nullsubexpr.dat testdata/repetition.dat */

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
  printf("%s [\x1b[32mSUCCESS\x1b[0m] ", source);
  vprintf(format, ap);
  printf("\n");
  va_end(ap);
}

void fail(const char *source, const char *format, ...) {
  va_list ap;
  va_start(ap, format);
  printf("%s [\x1b[31mFAIL   \x1b[0m] ", source);
  vprintf(format, ap);
  printf("\n");
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
      success(source, "/%s/ =~ \"%s\"", pattern, string);
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
      fail(source, "/%s/ =~ \"%s\"", pattern, string);
      result = -1;
    }
  } else if (result == 0) {
    if (nmatches == 0)
      success(source, "/%s/ !~ \"%s\"", pattern, string);
    else {
      fail(source, "/%s/ !~ \"%s\"", pattern, string);
      result = -1;
    }
  }

  va_end(ap);
  regexCompileFree(program);
  return result;
}

int main(int argc, char *argv[]) {
  int nerrors = 0;
  nerrors += test("testdata/basic.dat:003", "abracadabra$", "abracadabracadabra",
    2, 7, 18);
  nerrors += test("testdata/basic.dat:004", "a...b", "abababbb",
    2, 2, 7);
  nerrors += test("testdata/basic.dat:005", "XXXXXX", "..XXXXXX",
    2, 2, 8);
  nerrors += test("testdata/basic.dat:006", "\\)", "()",
    2, 1, 2);
  nerrors += test("testdata/basic.dat:007", "a]", "a]a",
    2, 0, 2);
  nerrors += test("testdata/basic.dat:008", "}", "}",
    2, 0, 1);
  nerrors += test("testdata/basic.dat:009", "\\}", "}",
    2, 0, 1);
  nerrors += test("testdata/basic.dat:010", "\\]", "]",
    2, 0, 1);
  nerrors += test("testdata/basic.dat:011", "]", "]",
    2, 0, 1);
  nerrors += test("testdata/basic.dat:012", "]", "]",
    2, 0, 1);
  nerrors += test("testdata/basic.dat:013", "{", "{",
    2, 0, 1);
  nerrors += test("testdata/basic.dat:014", "}", "}",
    2, 0, 1);
  nerrors += test("testdata/basic.dat:015", "^a", "ax",
    2, 0, 1);
  nerrors += test("testdata/basic.dat:016", "\\^a", "a^a",
    2, 1, 3);
  nerrors += test("testdata/basic.dat:017", "a\\^", "a^",
    2, 0, 2);
  nerrors += test("testdata/basic.dat:018", "a$", "aa",
    2, 1, 2);
  nerrors += test("testdata/basic.dat:019", "a\\$", "a$",
    2, 0, 2);
  nerrors += test("testdata/basic.dat:020", "^$", "",
    2, 0, 0);
  nerrors += test("testdata/basic.dat:021", "$^", "",
    2, 0, 0);
  nerrors += test("testdata/basic.dat:022", "a($)", "aa",
    4, 1, 2, 2, 2);
  nerrors += test("testdata/basic.dat:023", "a*(^a)", "aa",
    4, 0, 1, 0, 1);
  nerrors += test("testdata/basic.dat:024", "(..)*(...)*", "a",
    2, 0, 0);
  nerrors += test("testdata/basic.dat:025", "(..)*(...)*", "abcd",
    4, 0, 4, 2, 4);
  nerrors += test("testdata/basic.dat:026", "(ab|a)(bc|c)", "abc",
    6, 0, 3, 0, 2, 2, 3);
  nerrors += test("testdata/basic.dat:027", "(ab)c|abc", "abc",
    4, 0, 3, 0, 2);
  nerrors += test("testdata/basic.dat:028", "a{0}b", "ab",
    2, 1, 2);
  nerrors += test("testdata/basic.dat:029", "(a*)(b?)(b+)b{3}", "aaabbbbbbb",
    8, 0, 10, 0, 3, 3, 4, 4, 7);
  nerrors += test("testdata/basic.dat:030", "(a*)(b{0,1})(b{1,})b{3}", "aaabbbbbbb",
    8, 0, 10, 0, 3, 3, 4, 4, 7);
  nerrors += test("testdata/basic.dat:032", "((a|a)|a)", "a",
    6, 0, 1, 0, 1, 0, 1);
  nerrors += test("testdata/basic.dat:033", "(a*)(a|aa)", "aaaa",
    6, 0, 4, 0, 3, 3, 4);
  nerrors += test("testdata/basic.dat:034", "a*(a.|aa)", "aaaa",
    4, 0, 4, 2, 4);
  nerrors += test("testdata/basic.dat:035", "a(b)|c(d)|a(e)f", "aef",
    8, 0, 3, -1, -1, -1, -1, 1, 2);
  nerrors += test("testdata/basic.dat:036", "(a|b)?.*", "b",
    4, 0, 1, 0, 1);
  nerrors += test("testdata/basic.dat:037", "(a|b)c|a(b|c)", "ac",
    4, 0, 2, 0, 1);
  nerrors += test("testdata/basic.dat:038", "(a|b)c|a(b|c)", "ab",
    6, 0, 2, -1, -1, 1, 2);
  nerrors += test("testdata/basic.dat:039", "(a|b)*c|(a|ab)*c", "abc",
    4, 0, 3, 1, 2);
  nerrors += test("testdata/basic.dat:040", "(a|b)*c|(a|ab)*c", "xc",
    2, 1, 2);
  nerrors += test("testdata/basic.dat:041", "(.a|.b).*|.*(.a|.b)", "xa",
    4, 0, 2, 0, 2);
  nerrors += test("testdata/basic.dat:042", "a?(ab|ba)ab", "abab",
    4, 0, 4, 0, 2);
  nerrors += test("testdata/basic.dat:043", "a?(ac{0}b|ba)ab", "abab",
    4, 0, 4, 0, 2);
  nerrors += test("testdata/basic.dat:044", "ab|abab", "abbabab",
    2, 0, 2);
  nerrors += test("testdata/basic.dat:045", "aba|bab|bba", "baaabbbaba",
    2, 5, 8);
  nerrors += test("testdata/basic.dat:046", "aba|bab", "baaabbbaba",
    2, 6, 9);
  nerrors += test("testdata/basic.dat:047", "(aa|aaa)*|(a|aaaaa)", "aa",
    4, 0, 2, 0, 2);
  nerrors += test("testdata/basic.dat:048", "(a.|.a.)*|(a|.a...)", "aa",
    4, 0, 2, 0, 2);
  nerrors += test("testdata/basic.dat:049", "ab|a", "xabc",
    2, 1, 3);
  nerrors += test("testdata/basic.dat:050", "ab|a", "xxabc",
    2, 2, 4);
  nerrors += test("testdata/basic.dat:052", "[^-]", "--a",
    2, 2, 3);
  nerrors += test("testdata/basic.dat:053", "[a-]*", "--a",
    2, 0, 3);
  nerrors += test("testdata/basic.dat:054", "[a-m-]*", "--amoma--",
    2, 0, 4);
  nerrors += test("testdata/basic.dat:055", ":::1:::0:|:::1:1:0:", ":::0:::1:::1:::0:",
    2, 8, 17);
  nerrors += test("testdata/basic.dat:056", ":::1:::0:|:::1:1:1:", ":::0:::1:::1:::0:",
    2, 8, 17);
  nerrors += test("testdata/basic.dat:065", "\n", "\n",
    2, 0, 1);
  nerrors += test("testdata/basic.dat:067", "[^a]", "\n",
    2, 0, 1);
  nerrors += test("testdata/basic.dat:068", "\na", "\na",
    2, 0, 2);
  nerrors += test("testdata/basic.dat:069", "(a)(b)(c)", "abc",
    8, 0, 3, 0, 1, 1, 2, 2, 3);
  nerrors += test("testdata/basic.dat:070", "xxx", "xxx",
    2, 0, 3);
  nerrors += test("testdata/basic.dat:071", "(^|[ (,;])((([Ff]eb[^ ]* *|0*2/|\\* */?)0*[6-7]))([^0-9]|$)", "feb 6,",
    2, 0, 6);
  nerrors += test("testdata/basic.dat:072", "(^|[ (,;])((([Ff]eb[^ ]* *|0*2/|\\* */?)0*[6-7]))([^0-9]|$)", "2/7",
    2, 0, 3);
  nerrors += test("testdata/basic.dat:073", "(^|[ (,;])((([Ff]eb[^ ]* *|0*2/|\\* */?)0*[6-7]))([^0-9]|$)", "feb 1,Feb 6",
    2, 5, 11);
  nerrors += test("testdata/basic.dat:074", "((((((((((((((((((((((((((((((x))))))))))))))))))))))))))))))", "x",
    6, 0, 1, 0, 1, 0, 1);
  nerrors += test("testdata/basic.dat:075", "((((((((((((((((((((((((((((((x))))))))))))))))))))))))))))))*", "xx",
    6, 0, 2, 1, 2, 1, 2);
  nerrors += test("testdata/basic.dat:076", "a?(ab|ba)*", "ababababababababababababababababababababababababababababababababababababababababa",
    4, 0, 81, 79, 81);
  nerrors += test("testdata/basic.dat:077", "abaa|abbaa|abbbaa|abbbbaa", "ababbabbbabbbabbbbabbbbaa",
    2, 18, 25);
  nerrors += test("testdata/basic.dat:078", "abaa|abbaa|abbbaa|abbbbaa", "ababbabbbabbbabbbbabaa",
    2, 18, 22);
  nerrors += test("testdata/basic.dat:079", "aaac|aabc|abac|abbc|baac|babc|bbac|bbbc", "baaabbbabac",
    2, 7, 11);
  nerrors += test("testdata/basic.dat:080", ".*", "\x01\xff",
    2, 0, 2);
  nerrors += test("testdata/basic.dat:081", "aaaa|bbbb|cccc|ddddd|eeeeee|fffffff|gggg|hhhh|iiiii|jjjjj|kkkkk|llll", "XaaaXbbbXcccXdddXeeeXfffXgggXhhhXiiiXjjjXkkkXlllXcbaXaaaa",
    2, 53, 57);
  nerrors += test("testdata/basic.dat:082", "aaaa\\nbbbb\\ncccc\\nddddd\\neeeeee\\nfffffff\\ngggg\\nhhhh\\niiiii\\njjjjj\\nkkkkk\\nllll", "XaaaXbbbXcccXdddXeeeXfffXgggXhhhXiiiXjjjXkkkXlllXcbaXaaaa",
    0);
  nerrors += test("testdata/basic.dat:083", "a*a*a*a*a*b", "aaaaaaaaab",
    2, 0, 10);
  nerrors += test("testdata/basic.dat:084", "^", "",
    2, 0, 0);
  nerrors += test("testdata/basic.dat:085", "$", "",
    2, 0, 0);
  nerrors += test("testdata/basic.dat:086", "^$", "",
    2, 0, 0);
  nerrors += test("testdata/basic.dat:087", "^a$", "a",
    2, 0, 1);
  nerrors += test("testdata/basic.dat:088", "abc", "abc",
    2, 0, 3);
  nerrors += test("testdata/basic.dat:089", "abc", "xabcy",
    2, 1, 4);
  nerrors += test("testdata/basic.dat:090", "abc", "ababc",
    2, 2, 5);
  nerrors += test("testdata/basic.dat:091", "ab*c", "abc",
    2, 0, 3);
  nerrors += test("testdata/basic.dat:092", "ab*bc", "abc",
    2, 0, 3);
  nerrors += test("testdata/basic.dat:093", "ab*bc", "abbc",
    2, 0, 4);
  nerrors += test("testdata/basic.dat:094", "ab*bc", "abbbbc",
    2, 0, 6);
  nerrors += test("testdata/basic.dat:095", "ab+bc", "abbc",
    2, 0, 4);
  nerrors += test("testdata/basic.dat:096", "ab+bc", "abbbbc",
    2, 0, 6);
  nerrors += test("testdata/basic.dat:097", "ab?bc", "abbc",
    2, 0, 4);
  nerrors += test("testdata/basic.dat:098", "ab?bc", "abc",
    2, 0, 3);
  nerrors += test("testdata/basic.dat:099", "ab?c", "abc",
    2, 0, 3);
  nerrors += test("testdata/basic.dat:100", "^abc$", "abc",
    2, 0, 3);
  nerrors += test("testdata/basic.dat:101", "^abc", "abcc",
    2, 0, 3);
  nerrors += test("testdata/basic.dat:102", "abc$", "aabc",
    2, 1, 4);
  nerrors += test("testdata/basic.dat:103", "^", "abc",
    2, 0, 0);
  nerrors += test("testdata/basic.dat:104", "$", "abc",
    2, 3, 3);
  nerrors += test("testdata/basic.dat:105", "a.c", "abc",
    2, 0, 3);
  nerrors += test("testdata/basic.dat:106", "a.c", "axc",
    2, 0, 3);
  nerrors += test("testdata/basic.dat:107", "a.*c", "axyzc",
    2, 0, 5);
  nerrors += test("testdata/basic.dat:108", "a[bc]d", "abd",
    2, 0, 3);
  nerrors += test("testdata/basic.dat:109", "a[b-d]e", "ace",
    2, 0, 3);
  nerrors += test("testdata/basic.dat:110", "a[b-d]", "aac",
    2, 1, 3);
  nerrors += test("testdata/basic.dat:111", "a[-b]", "a-",
    2, 0, 2);
  nerrors += test("testdata/basic.dat:112", "a[b-]", "a-",
    2, 0, 2);
  nerrors += test("testdata/basic.dat:113", "a]", "a]",
    2, 0, 2);
  nerrors += test("testdata/basic.dat:114", "a[]]b", "a]b",
    2, 0, 3);
  nerrors += test("testdata/basic.dat:115", "a[^bc]d", "aed",
    2, 0, 3);
  nerrors += test("testdata/basic.dat:116", "a[^-b]c", "adc",
    2, 0, 3);
  nerrors += test("testdata/basic.dat:117", "a[^]b]c", "adc",
    2, 0, 3);
  nerrors += test("testdata/basic.dat:118", "ab|cd", "abc",
    2, 0, 2);
  nerrors += test("testdata/basic.dat:119", "ab|cd", "abcd",
    2, 0, 2);
  nerrors += test("testdata/basic.dat:120", "a\\(b", "a(b",
    2, 0, 3);
  nerrors += test("testdata/basic.dat:121", "a\\(*b", "ab",
    2, 0, 2);
  nerrors += test("testdata/basic.dat:122", "a\\(*b", "a((b",
    2, 0, 4);
  nerrors += test("testdata/basic.dat:123", "((a))", "abc",
    6, 0, 1, 0, 1, 0, 1);
  nerrors += test("testdata/basic.dat:124", "(a)b(c)", "abc",
    6, 0, 3, 0, 1, 2, 3);
  nerrors += test("testdata/basic.dat:125", "a+b+c", "aabbabc",
    2, 4, 7);
  nerrors += test("testdata/basic.dat:126", "a*", "aaa",
    2, 0, 3);
  nerrors += test("testdata/basic.dat:128", "(a*)*", "-",
    4, 0, 0, -1, -1);
  nerrors += test("testdata/basic.dat:129", "(a*)+", "-",
    4, 0, 0, 0, 0);
  nerrors += test("testdata/basic.dat:131", "(a*|b)*", "-",
    4, 0, 0, -1, -1);
  nerrors += test("testdata/basic.dat:132", "(a+|b)*", "ab",
    4, 0, 2, 1, 2);
  nerrors += test("testdata/basic.dat:133", "(a+|b)+", "ab",
    4, 0, 2, 1, 2);
  nerrors += test("testdata/basic.dat:134", "(a+|b)?", "ab",
    4, 0, 1, 0, 1);
  nerrors += test("testdata/basic.dat:135", "[^ab]*", "cde",
    2, 0, 3);
  nerrors += test("testdata/basic.dat:137", "(^)*", "-",
    4, 0, 0, -1, -1);
  nerrors += test("testdata/basic.dat:138", "a*", "",
    2, 0, 0);
  nerrors += test("testdata/basic.dat:139", "([abc])*d", "abbbcd",
    4, 0, 6, 4, 5);
  nerrors += test("testdata/basic.dat:140", "([abc])*bcd", "abcd",
    4, 0, 4, 0, 1);
  nerrors += test("testdata/basic.dat:141", "a|b|c|d|e", "e",
    2, 0, 1);
  nerrors += test("testdata/basic.dat:142", "(a|b|c|d|e)f", "ef",
    4, 0, 2, 0, 1);
  nerrors += test("testdata/basic.dat:144", "((a*|b))*", "-",
    6, 0, 0, -1, -1, -1, -1);
  nerrors += test("testdata/basic.dat:145", "abcd*efg", "abcdefg",
    2, 0, 7);
  nerrors += test("testdata/basic.dat:146", "ab*", "xabyabbbz",
    2, 1, 3);
  nerrors += test("testdata/basic.dat:147", "ab*", "xayabbbz",
    2, 1, 2);
  nerrors += test("testdata/basic.dat:148", "(ab|cd)e", "abcde",
    4, 2, 5, 2, 4);
  nerrors += test("testdata/basic.dat:149", "[abhgefdc]ij", "hij",
    2, 0, 3);
  nerrors += test("testdata/basic.dat:150", "(a|b)c*d", "abcd",
    4, 1, 4, 1, 2);
  nerrors += test("testdata/basic.dat:151", "(ab|ab*)bc", "abc",
    4, 0, 3, 0, 1);
  nerrors += test("testdata/basic.dat:152", "a([bc]*)c*", "abc",
    4, 0, 3, 1, 3);
  nerrors += test("testdata/basic.dat:153", "a([bc]*)(c*d)", "abcd",
    6, 0, 4, 1, 3, 3, 4);
  nerrors += test("testdata/basic.dat:154", "a([bc]+)(c*d)", "abcd",
    6, 0, 4, 1, 3, 3, 4);
  nerrors += test("testdata/basic.dat:155", "a([bc]*)(c+d)", "abcd",
    6, 0, 4, 1, 2, 2, 4);
  nerrors += test("testdata/basic.dat:156", "a[bcd]*dcdcde", "adcdcde",
    2, 0, 7);
  nerrors += test("testdata/basic.dat:157", "(ab|a)b*c", "abc",
    4, 0, 3, 0, 2);
  nerrors += test("testdata/basic.dat:158", "((a)(b)c)(d)", "abcd",
    10, 0, 4, 0, 3, 0, 1, 1, 2, 3, 4);
  nerrors += test("testdata/basic.dat:159", "[A-Za-z_][A-Za-z0-9_]*", "alpha",
    2, 0, 5);
  nerrors += test("testdata/basic.dat:160", "^a(bc+|b[eh])g|.h$", "abh",
    2, 1, 3);
  nerrors += test("testdata/basic.dat:161", "(bc+d$|ef*g.|h?i(j|k))", "effgz",
    4, 0, 5, 0, 5);
  nerrors += test("testdata/basic.dat:162", "(bc+d$|ef*g.|h?i(j|k))", "ij",
    6, 0, 2, 0, 2, 1, 2);
  nerrors += test("testdata/basic.dat:163", "(bc+d$|ef*g.|h?i(j|k))", "reffgz",
    4, 1, 6, 1, 6);
  nerrors += test("testdata/basic.dat:164", "(((((((((a)))))))))", "a",
    20, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1);
  nerrors += test("testdata/basic.dat:165", "multiple words", "multiple words yeah",
    2, 0, 14);
  nerrors += test("testdata/basic.dat:166", "(.*)c(.*)", "abcde",
    6, 0, 5, 0, 2, 3, 5);
  nerrors += test("testdata/basic.dat:167", "abcd", "abcd",
    2, 0, 4);
  nerrors += test("testdata/basic.dat:168", "a(bc)d", "abcd",
    4, 0, 4, 1, 3);
  nerrors += test("testdata/basic.dat:169", "a[-]?c", "ac",
    2, 0, 3);
  nerrors += test("testdata/basic.dat:170", "M[ou]'?am+[ae]r .*([AEae]l[- ])?[GKQ]h?[aeu]+([dtz][dhz]?)+af[iy]", "Muammar Qaddafi",
    6, 0, 15, -1, -1, 10, 12);
  nerrors += test("testdata/basic.dat:171", "M[ou]'?am+[ae]r .*([AEae]l[- ])?[GKQ]h?[aeu]+([dtz][dhz]?)+af[iy]", "Mo'ammar Gadhafi",
    6, 0, 16, -1, -1, 11, 13);
  nerrors += test("testdata/basic.dat:172", "M[ou]'?am+[ae]r .*([AEae]l[- ])?[GKQ]h?[aeu]+([dtz][dhz]?)+af[iy]", "Muammar Kaddafi",
    6, 0, 15, -1, -1, 10, 12);
  nerrors += test("testdata/basic.dat:173", "M[ou]'?am+[ae]r .*([AEae]l[- ])?[GKQ]h?[aeu]+([dtz][dhz]?)+af[iy]", "Muammar Qadhafi",
    6, 0, 15, -1, -1, 10, 12);
  nerrors += test("testdata/basic.dat:174", "M[ou]'?am+[ae]r .*([AEae]l[- ])?[GKQ]h?[aeu]+([dtz][dhz]?)+af[iy]", "Muammar Gadafi",
    6, 0, 14, -1, -1, 10, 11);
  nerrors += test("testdata/basic.dat:175", "M[ou]'?am+[ae]r .*([AEae]l[- ])?[GKQ]h?[aeu]+([dtz][dhz]?)+af[iy]", "Mu'ammar Qadafi",
    6, 0, 15, -1, -1, 11, 12);
  nerrors += test("testdata/basic.dat:176", "M[ou]'?am+[ae]r .*([AEae]l[- ])?[GKQ]h?[aeu]+([dtz][dhz]?)+af[iy]", "Moamar Gaddafi",
    6, 0, 14, -1, -1, 9, 11);
  nerrors += test("testdata/basic.dat:177", "M[ou]'?am+[ae]r .*([AEae]l[- ])?[GKQ]h?[aeu]+([dtz][dhz]?)+af[iy]", "Mu'ammar Qadhdhafi",
    6, 0, 18, -1, -1, 13, 15);
  nerrors += test("testdata/basic.dat:178", "M[ou]'?am+[ae]r .*([AEae]l[- ])?[GKQ]h?[aeu]+([dtz][dhz]?)+af[iy]", "Muammar Khaddafi",
    6, 0, 16, -1, -1, 11, 13);
  nerrors += test("testdata/basic.dat:179", "M[ou]'?am+[ae]r .*([AEae]l[- ])?[GKQ]h?[aeu]+([dtz][dhz]?)+af[iy]", "Muammar Ghaddafy",
    6, 0, 16, -1, -1, 11, 13);
  nerrors += test("testdata/basic.dat:180", "M[ou]'?am+[ae]r .*([AEae]l[- ])?[GKQ]h?[aeu]+([dtz][dhz]?)+af[iy]", "Muammar Ghadafi",
    6, 0, 15, -1, -1, 11, 12);
  nerrors += test("testdata/basic.dat:181", "M[ou]'?am+[ae]r .*([AEae]l[- ])?[GKQ]h?[aeu]+([dtz][dhz]?)+af[iy]", "Muammar Ghaddafi",
    6, 0, 16, -1, -1, 11, 13);
  nerrors += test("testdata/basic.dat:182", "M[ou]'?am+[ae]r .*([AEae]l[- ])?[GKQ]h?[aeu]+([dtz][dhz]?)+af[iy]", "Muamar Kaddafi",
    6, 0, 14, -1, -1, 9, 11);
  nerrors += test("testdata/basic.dat:183", "M[ou]'?am+[ae]r .*([AEae]l[- ])?[GKQ]h?[aeu]+([dtz][dhz]?)+af[iy]", "Muammar Quathafi",
    6, 0, 16, -1, -1, 11, 13);
  nerrors += test("testdata/basic.dat:184", "M[ou]'?am+[ae]r .*([AEae]l[- ])?[GKQ]h?[aeu]+([dtz][dhz]?)+af[iy]", "Muammar Gheddafi",
    6, 0, 16, -1, -1, 11, 13);
  nerrors += test("testdata/basic.dat:185", "M[ou]'?am+[ae]r .*([AEae]l[- ])?[GKQ]h?[aeu]+([dtz][dhz]?)+af[iy]", "Moammar Khadafy",
    6, 0, 15, -1, -1, 11, 12);
  nerrors += test("testdata/basic.dat:186", "M[ou]'?am+[ae]r .*([AEae]l[- ])?[GKQ]h?[aeu]+([dtz][dhz]?)+af[iy]", "Moammar Qudhafi",
    6, 0, 15, -1, -1, 10, 12);
  nerrors += test("testdata/basic.dat:187", "a+(b|c)*d+", "aabcdd",
    4, 0, 6, 3, 4);
  nerrors += test("testdata/basic.dat:188", "^.+$", "vivi",
    2, 0, 4);
  nerrors += test("testdata/basic.dat:189", "^(.+)$", "vivi",
    4, 0, 4, 0, 4);
  nerrors += test("testdata/basic.dat:190", "^([^!.]+).att.com!(.+)$", "gryphon.att.com!eby",
    6, 0, 19, 0, 7, 16, 19);
  nerrors += test("testdata/basic.dat:191", "^([^!]+!)?([^!]+)$", "bas",
    6, 0, 3, -1, -1, 0, 3);
  nerrors += test("testdata/basic.dat:192", "^([^!]+!)?([^!]+)$", "bar!bas",
    6, 0, 7, 0, 4, 4, 7);
  nerrors += test("testdata/basic.dat:193", "^([^!]+!)?([^!]+)$", "foo!bas",
    6, 0, 7, 0, 4, 4, 7);
  nerrors += test("testdata/basic.dat:194", "^.+!([^!]+!)([^!]+)$", "foo!bar!bas",
    6, 0, 11, 4, 8, 8, 11);
  nerrors += test("testdata/basic.dat:195", "((foo)|(bar))!bas", "bar!bas",
    8, 0, 7, 0, 3, -1, -1, 0, 3);
  nerrors += test("testdata/basic.dat:196", "((foo)|(bar))!bas", "foo!bar!bas",
    8, 4, 11, 4, 7, -1, -1, 4, 7);
  nerrors += test("testdata/basic.dat:197", "((foo)|(bar))!bas", "foo!bas",
    6, 0, 7, 0, 3, 0, 3);
  nerrors += test("testdata/basic.dat:198", "((foo)|bar)!bas", "bar!bas",
    4, 0, 7, 0, 3);
  nerrors += test("testdata/basic.dat:199", "((foo)|bar)!bas", "foo!bar!bas",
    4, 4, 11, 4, 7);
  nerrors += test("testdata/basic.dat:200", "((foo)|bar)!bas", "foo!bas",
    6, 0, 7, 0, 3, 0, 3);
  nerrors += test("testdata/basic.dat:201", "(foo|(bar))!bas", "bar!bas",
    6, 0, 7, 0, 3, 0, 3);
  nerrors += test("testdata/basic.dat:202", "(foo|(bar))!bas", "foo!bar!bas",
    6, 4, 11, 4, 7, 4, 7);
  nerrors += test("testdata/basic.dat:203", "(foo|(bar))!bas", "foo!bas",
    4, 0, 7, 0, 3);
  nerrors += test("testdata/basic.dat:204", "(foo|bar)!bas", "bar!bas",
    4, 0, 7, 0, 3);
  nerrors += test("testdata/basic.dat:205", "(foo|bar)!bas", "foo!bar!bas",
    4, 4, 11, 4, 7);
  nerrors += test("testdata/basic.dat:206", "(foo|bar)!bas", "foo!bas",
    4, 0, 7, 0, 3);
  nerrors += test("testdata/basic.dat:207", "^(([^!]+!)?([^!]+)|.+!([^!]+!)([^!]+))$", "foo!bar!bas",
    12, 0, 11, 0, 11, -1, -1, -1, -1, 4, 8, 8, 11);
  nerrors += test("testdata/basic.dat:208", "^([^!]+!)?([^!]+)$|^.+!([^!]+!)([^!]+)$", "bas",
    6, 0, 3, -1, -1, 0, 3);
  nerrors += test("testdata/basic.dat:209", "^([^!]+!)?([^!]+)$|^.+!([^!]+!)([^!]+)$", "bar!bas",
    6, 0, 7, 0, 4, 4, 7);
  nerrors += test("testdata/basic.dat:210", "^([^!]+!)?([^!]+)$|^.+!([^!]+!)([^!]+)$", "foo!bar!bas",
    10, 0, 11, -1, -1, -1, -1, 4, 8, 8, 11);
  nerrors += test("testdata/basic.dat:211", "^([^!]+!)?([^!]+)$|^.+!([^!]+!)([^!]+)$", "foo!bas",
    6, 0, 7, 0, 4, 4, 7);
  nerrors += test("testdata/basic.dat:212", "^(([^!]+!)?([^!]+)|.+!([^!]+!)([^!]+))$", "bas",
    8, 0, 3, 0, 3, -1, -1, 0, 3);
  nerrors += test("testdata/basic.dat:213", "^(([^!]+!)?([^!]+)|.+!([^!]+!)([^!]+))$", "bar!bas",
    8, 0, 7, 0, 7, 0, 4, 4, 7);
  nerrors += test("testdata/basic.dat:214", "^(([^!]+!)?([^!]+)|.+!([^!]+!)([^!]+))$", "foo!bar!bas",
    12, 0, 11, 0, 11, -1, -1, -1, -1, 4, 8, 8, 11);
  nerrors += test("testdata/basic.dat:215", "^(([^!]+!)?([^!]+)|.+!([^!]+!)([^!]+))$", "foo!bas",
    8, 0, 7, 0, 7, 0, 4, 4, 7);
  nerrors += test("testdata/basic.dat:216", ".*(/XXX).*", "/XXX",
    4, 0, 4, 0, 4);
  nerrors += test("testdata/basic.dat:217", ".*(\\\\XXX).*", "\\XXX",
    4, 0, 4, 0, 4);
  nerrors += test("testdata/basic.dat:218", "\\\\XXX", "\\XXX",
    2, 0, 4);
  nerrors += test("testdata/basic.dat:219", ".*(/000).*", "/000",
    4, 0, 4, 0, 4);
  nerrors += test("testdata/basic.dat:220", ".*(\\\\000).*", "\\000",
    4, 0, 4, 0, 4);
  nerrors += test("testdata/basic.dat:221", "\\\\000", "\\000",
    2, 0, 4);
  nerrors += test("testdata/nullsubexpr.dat:003", "(a*)*", "a",
    4, 0, 1, 0, 1);
  nerrors += test("testdata/nullsubexpr.dat:005", "(a*)*", "x",
    4, 0, 0, -1, -1);
  nerrors += test("testdata/nullsubexpr.dat:006", "(a*)*", "aaaaaa",
    4, 0, 6, 0, 6);
  nerrors += test("testdata/nullsubexpr.dat:007", "(a*)*", "aaaaaax",
    4, 0, 6, 0, 6);
  nerrors += test("testdata/nullsubexpr.dat:008", "(a*)+", "a",
    4, 0, 1, 0, 1);
  nerrors += test("testdata/nullsubexpr.dat:009", "(a*)+", "x",
    4, 0, 0, 0, 0);
  nerrors += test("testdata/nullsubexpr.dat:010", "(a*)+", "aaaaaa",
    4, 0, 6, 0, 6);
  nerrors += test("testdata/nullsubexpr.dat:011", "(a*)+", "aaaaaax",
    4, 0, 6, 0, 6);
  nerrors += test("testdata/nullsubexpr.dat:012", "(a+)*", "a",
    4, 0, 1, 0, 1);
  nerrors += test("testdata/nullsubexpr.dat:013", "(a+)*", "x",
    2, 0, 0);
  nerrors += test("testdata/nullsubexpr.dat:014", "(a+)*", "aaaaaa",
    4, 0, 6, 0, 6);
  nerrors += test("testdata/nullsubexpr.dat:015", "(a+)*", "aaaaaax",
    4, 0, 6, 0, 6);
  nerrors += test("testdata/nullsubexpr.dat:016", "(a+)+", "a",
    4, 0, 1, 0, 1);
  nerrors += test("testdata/nullsubexpr.dat:017", "(a+)+", "x",
    0);
  nerrors += test("testdata/nullsubexpr.dat:018", "(a+)+", "aaaaaa",
    4, 0, 6, 0, 6);
  nerrors += test("testdata/nullsubexpr.dat:019", "(a+)+", "aaaaaax",
    4, 0, 6, 0, 6);
  nerrors += test("testdata/nullsubexpr.dat:021", "([a]*)*", "a",
    4, 0, 1, 0, 1);
  nerrors += test("testdata/nullsubexpr.dat:023", "([a]*)*", "x",
    4, 0, 0, -1, -1);
  nerrors += test("testdata/nullsubexpr.dat:024", "([a]*)*", "aaaaaa",
    4, 0, 6, 0, 6);
  nerrors += test("testdata/nullsubexpr.dat:025", "([a]*)*", "aaaaaax",
    4, 0, 6, 0, 6);
  nerrors += test("testdata/nullsubexpr.dat:026", "([a]*)+", "a",
    4, 0, 1, 0, 1);
  nerrors += test("testdata/nullsubexpr.dat:027", "([a]*)+", "x",
    4, 0, 0, 0, 0);
  nerrors += test("testdata/nullsubexpr.dat:028", "([a]*)+", "aaaaaa",
    4, 0, 6, 0, 6);
  nerrors += test("testdata/nullsubexpr.dat:029", "([a]*)+", "aaaaaax",
    4, 0, 6, 0, 6);
  nerrors += test("testdata/nullsubexpr.dat:030", "([^b]*)*", "a",
    4, 0, 1, 0, 1);
  nerrors += test("testdata/nullsubexpr.dat:032", "([^b]*)*", "b",
    4, 0, 0, -1, -1);
  nerrors += test("testdata/nullsubexpr.dat:033", "([^b]*)*", "aaaaaa",
    4, 0, 6, 0, 6);
  nerrors += test("testdata/nullsubexpr.dat:034", "([^b]*)*", "aaaaaab",
    4, 0, 6, 0, 6);
  nerrors += test("testdata/nullsubexpr.dat:035", "([ab]*)*", "a",
    4, 0, 1, 0, 1);
  nerrors += test("testdata/nullsubexpr.dat:036", "([ab]*)*", "aaaaaa",
    4, 0, 6, 0, 6);
  nerrors += test("testdata/nullsubexpr.dat:037", "([ab]*)*", "ababab",
    4, 0, 6, 0, 6);
  nerrors += test("testdata/nullsubexpr.dat:038", "([ab]*)*", "bababa",
    4, 0, 6, 0, 6);
  nerrors += test("testdata/nullsubexpr.dat:039", "([ab]*)*", "b",
    4, 0, 1, 0, 1);
  nerrors += test("testdata/nullsubexpr.dat:040", "([ab]*)*", "bbbbbb",
    4, 0, 6, 0, 6);
  nerrors += test("testdata/nullsubexpr.dat:041", "([ab]*)*", "aaaabcde",
    4, 0, 5, 0, 5);
  nerrors += test("testdata/nullsubexpr.dat:042", "([^a]*)*", "b",
    4, 0, 1, 0, 1);
  nerrors += test("testdata/nullsubexpr.dat:043", "([^a]*)*", "bbbbbb",
    4, 0, 6, 0, 6);
  nerrors += test("testdata/nullsubexpr.dat:045", "([^a]*)*", "aaaaaa",
    4, 0, 0, -1, -1);
  nerrors += test("testdata/nullsubexpr.dat:046", "([^ab]*)*", "ccccxx",
    4, 0, 6, 0, 6);
  nerrors += test("testdata/nullsubexpr.dat:048", "([^ab]*)*", "ababab",
    4, 0, 0, -1, -1);
  nerrors += test("testdata/nullsubexpr.dat:050", "((z)+|a)*", "zabcde",
    4, 0, 2, 1, 2);
  nerrors += test("testdata/nullsubexpr.dat:069", "(a*)*(x)", "x",
    6, 0, 1, -1, -1, 0, 1);
  nerrors += test("testdata/nullsubexpr.dat:070", "(a*)*(x)", "ax",
    6, 0, 2, 0, 1, 1, 2);
  nerrors += test("testdata/nullsubexpr.dat:071", "(a*)*(x)", "axa",
    6, 0, 2, 0, 1, 1, 2);
  nerrors += test("testdata/nullsubexpr.dat:073", "(a*)+(x)", "x",
    6, 0, 1, 0, 0, 0, 1);
  nerrors += test("testdata/nullsubexpr.dat:074", "(a*)+(x)", "ax",
    6, 0, 2, 0, 1, 1, 2);
  nerrors += test("testdata/nullsubexpr.dat:075", "(a*)+(x)", "axa",
    6, 0, 2, 0, 1, 1, 2);
  nerrors += test("testdata/nullsubexpr.dat:077", "(a*){2}(x)", "x",
    6, 0, 1, 0, 0, 0, 1);
  nerrors += test("testdata/nullsubexpr.dat:078", "(a*){2}(x)", "ax",
    6, 0, 2, 1, 1, 1, 2);
  nerrors += test("testdata/nullsubexpr.dat:079", "(a*){2}(x)", "axa",
    6, 0, 2, 1, 1, 1, 2);
  nerrors += test("testdata/repetition.dat:010", "((..)|(.))", "",
    0);
  nerrors += test("testdata/repetition.dat:011", "((..)|(.))((..)|(.))", "",
    0);
  nerrors += test("testdata/repetition.dat:012", "((..)|(.))((..)|(.))((..)|(.))", "",
    0);
  nerrors += test("testdata/repetition.dat:014", "((..)|(.)){1}", "",
    0);
  nerrors += test("testdata/repetition.dat:015", "((..)|(.)){2}", "",
    0);
  nerrors += test("testdata/repetition.dat:016", "((..)|(.)){3}", "",
    0);
  nerrors += test("testdata/repetition.dat:018", "((..)|(.))*", "",
    2, 0, 0);
  nerrors += test("testdata/repetition.dat:020", "((..)|(.))", "a",
    8, 0, 1, 0, 1, -1, -1, 0, 1);
  nerrors += test("testdata/repetition.dat:021", "((..)|(.))((..)|(.))", "a",
    0);
  nerrors += test("testdata/repetition.dat:022", "((..)|(.))((..)|(.))((..)|(.))", "a",
    0);
  nerrors += test("testdata/repetition.dat:024", "((..)|(.)){1}", "a",
    8, 0, 1, 0, 1, -1, -1, 0, 1);
  nerrors += test("testdata/repetition.dat:025", "((..)|(.)){2}", "a",
    0);
  nerrors += test("testdata/repetition.dat:026", "((..)|(.)){3}", "a",
    0);
  nerrors += test("testdata/repetition.dat:028", "((..)|(.))*", "a",
    8, 0, 1, 0, 1, -1, -1, 0, 1);
  nerrors += test("testdata/repetition.dat:030", "((..)|(.))", "aa",
    8, 0, 2, 0, 2, 0, 2, -1, -1);
  nerrors += test("testdata/repetition.dat:031", "((..)|(.))((..)|(.))", "aa",
    14, 0, 2, 0, 1, -1, -1, 0, 1, 1, 2, -1, -1, 1, 2);
  nerrors += test("testdata/repetition.dat:032", "((..)|(.))((..)|(.))((..)|(.))", "aa",
    0);
  nerrors += test("testdata/repetition.dat:034", "((..)|(.)){1}", "aa",
    8, 0, 2, 0, 2, 0, 2, -1, -1);
  nerrors += test("testdata/repetition.dat:035", "((..)|(.)){2}", "aa",
    8, 0, 2, 1, 2, -1, -1, 1, 2);
  nerrors += test("testdata/repetition.dat:036", "((..)|(.)){3}", "aa",
    0);
  nerrors += test("testdata/repetition.dat:038", "((..)|(.))*", "aa",
    8, 0, 2, 0, 2, 0, 2, -1, -1);
  nerrors += test("testdata/repetition.dat:040", "((..)|(.))", "aaa",
    8, 0, 2, 0, 2, 0, 2, -1, -1);
  nerrors += test("testdata/repetition.dat:041", "((..)|(.))((..)|(.))", "aaa",
    14, 0, 3, 0, 2, 0, 2, -1, -1, 2, 3, -1, -1, 2, 3);
  nerrors += test("testdata/repetition.dat:042", "((..)|(.))((..)|(.))((..)|(.))", "aaa",
    20, 0, 3, 0, 1, -1, -1, 0, 1, 1, 2, -1, -1, 1, 2, 2, 3, -1, -1, 2, 3);
  nerrors += test("testdata/repetition.dat:044", "((..)|(.)){1}", "aaa",
    8, 0, 2, 0, 2, 0, 2, -1, -1);
  nerrors += test("testdata/repetition.dat:046", "((..)|(.)){2}", "aaa",
    8, 0, 3, 2, 3, 0, 2, 2, 3);
  nerrors += test("testdata/repetition.dat:047", "((..)|(.)){3}", "aaa",
    8, 0, 3, 2, 3, -1, -1, 2, 3);
  nerrors += test("testdata/repetition.dat:050", "((..)|(.))*", "aaa",
    8, 0, 3, 2, 3, 0, 2, 2, 3);
  nerrors += test("testdata/repetition.dat:052", "((..)|(.))", "aaaa",
    8, 0, 2, 0, 2, 0, 2, -1, -1);
  nerrors += test("testdata/repetition.dat:053", "((..)|(.))((..)|(.))", "aaaa",
    14, 0, 4, 0, 2, 0, 2, -1, -1, 2, 4, 2, 4, -1, -1);
  nerrors += test("testdata/repetition.dat:054", "((..)|(.))((..)|(.))((..)|(.))", "aaaa",
    20, 0, 4, 0, 2, 0, 2, -1, -1, 2, 3, -1, -1, 2, 3, 3, 4, -1, -1, 3, 4);
  nerrors += test("testdata/repetition.dat:056", "((..)|(.)){1}", "aaaa",
    8, 0, 2, 0, 2, 0, 2, -1, -1);
  nerrors += test("testdata/repetition.dat:057", "((..)|(.)){2}", "aaaa",
    8, 0, 4, 2, 4, 2, 4, -1, -1);
  nerrors += test("testdata/repetition.dat:059", "((..)|(.)){3}", "aaaa",
    8, 0, 4, 3, 4, 0, 2, 3, 4);
  nerrors += test("testdata/repetition.dat:061", "((..)|(.))*", "aaaa",
    8, 0, 4, 2, 4, 2, 4, -1, -1);
  nerrors += test("testdata/repetition.dat:063", "((..)|(.))", "aaaaa",
    8, 0, 2, 0, 2, 0, 2, -1, -1);
  nerrors += test("testdata/repetition.dat:064", "((..)|(.))((..)|(.))", "aaaaa",
    14, 0, 4, 0, 2, 0, 2, -1, -1, 2, 4, 2, 4, -1, -1);
  nerrors += test("testdata/repetition.dat:065", "((..)|(.))((..)|(.))((..)|(.))", "aaaaa",
    20, 0, 5, 0, 2, 0, 2, -1, -1, 2, 4, 2, 4, -1, -1, 4, 5, -1, -1, 4, 5);
  nerrors += test("testdata/repetition.dat:067", "((..)|(.)){1}", "aaaaa",
    8, 0, 2, 0, 2, 0, 2, -1, -1);
  nerrors += test("testdata/repetition.dat:068", "((..)|(.)){2}", "aaaaa",
    8, 0, 4, 2, 4, 2, 4, -1, -1);
  nerrors += test("testdata/repetition.dat:070", "((..)|(.)){3}", "aaaaa",
    8, 0, 5, 4, 5, 2, 4, 4, 5);
  nerrors += test("testdata/repetition.dat:073", "((..)|(.))*", "aaaaa",
    8, 0, 5, 4, 5, 2, 4, 4, 5);
  nerrors += test("testdata/repetition.dat:075", "((..)|(.))", "aaaaaa",
    8, 0, 2, 0, 2, 0, 2, -1, -1);
  nerrors += test("testdata/repetition.dat:076", "((..)|(.))((..)|(.))", "aaaaaa",
    14, 0, 4, 0, 2, 0, 2, -1, -1, 2, 4, 2, 4, -1, -1);
  nerrors += test("testdata/repetition.dat:077", "((..)|(.))((..)|(.))((..)|(.))", "aaaaaa",
    20, 0, 6, 0, 2, 0, 2, -1, -1, 2, 4, 2, 4, -1, -1, 4, 6, 4, 6, -1, -1);
  nerrors += test("testdata/repetition.dat:079", "((..)|(.)){1}", "aaaaaa",
    8, 0, 2, 0, 2, 0, 2, -1, -1);
  nerrors += test("testdata/repetition.dat:080", "((..)|(.)){2}", "aaaaaa",
    8, 0, 4, 2, 4, 2, 4, -1, -1);
  nerrors += test("testdata/repetition.dat:081", "((..)|(.)){3}", "aaaaaa",
    8, 0, 6, 4, 6, 4, 6, -1, -1);
  nerrors += test("testdata/repetition.dat:083", "((..)|(.))*", "aaaaaa",
    8, 0, 6, 4, 6, 4, 6, -1, -1);
  nerrors += test("testdata/repetition.dat:090", "X(.?){0,}Y", "X1234567Y",
    4, 0, 9, 7, 8);
  nerrors += test("testdata/repetition.dat:091", "X(.?){1,}Y", "X1234567Y",
    4, 0, 9, 7, 8);
  nerrors += test("testdata/repetition.dat:092", "X(.?){2,}Y", "X1234567Y",
    4, 0, 9, 7, 8);
  nerrors += test("testdata/repetition.dat:093", "X(.?){3,}Y", "X1234567Y",
    4, 0, 9, 7, 8);
  nerrors += test("testdata/repetition.dat:094", "X(.?){4,}Y", "X1234567Y",
    4, 0, 9, 7, 8);
  nerrors += test("testdata/repetition.dat:095", "X(.?){5,}Y", "X1234567Y",
    4, 0, 9, 7, 8);
  nerrors += test("testdata/repetition.dat:096", "X(.?){6,}Y", "X1234567Y",
    4, 0, 9, 7, 8);
  nerrors += test("testdata/repetition.dat:097", "X(.?){7,}Y", "X1234567Y",
    4, 0, 9, 7, 8);
  nerrors += test("testdata/repetition.dat:098", "X(.?){8,}Y", "X1234567Y",
    4, 0, 9, 8, 8);
  nerrors += test("testdata/repetition.dat:100", "X(.?){0,8}Y", "X1234567Y",
    4, 0, 9, 8, 8);
  nerrors += test("testdata/repetition.dat:102", "X(.?){1,8}Y", "X1234567Y",
    4, 0, 9, 8, 8);
  nerrors += test("testdata/repetition.dat:104", "X(.?){2,8}Y", "X1234567Y",
    4, 0, 9, 8, 8);
  nerrors += test("testdata/repetition.dat:106", "X(.?){3,8}Y", "X1234567Y",
    4, 0, 9, 8, 8);
  nerrors += test("testdata/repetition.dat:108", "X(.?){4,8}Y", "X1234567Y",
    4, 0, 9, 8, 8);
  nerrors += test("testdata/repetition.dat:110", "X(.?){5,8}Y", "X1234567Y",
    4, 0, 9, 8, 8);
  nerrors += test("testdata/repetition.dat:112", "X(.?){6,8}Y", "X1234567Y",
    4, 0, 9, 8, 8);
  nerrors += test("testdata/repetition.dat:114", "X(.?){7,8}Y", "X1234567Y",
    4, 0, 9, 8, 8);
  nerrors += test("testdata/repetition.dat:115", "X(.?){8,8}Y", "X1234567Y",
    4, 0, 9, 8, 8);
  nerrors += test("testdata/repetition.dat:127", "(a|ab|c|bcd){0,}(d*)", "ababcd",
    6, 0, 1, 0, 1, 1, 1);
  nerrors += test("testdata/repetition.dat:129", "(a|ab|c|bcd){1,}(d*)", "ababcd",
    6, 0, 1, 0, 1, 1, 1);
  nerrors += test("testdata/repetition.dat:130", "(a|ab|c|bcd){2,}(d*)", "ababcd",
    6, 0, 6, 3, 6, 6, 6);
  nerrors += test("testdata/repetition.dat:131", "(a|ab|c|bcd){3,}(d*)", "ababcd",
    6, 0, 6, 3, 6, 6, 6);
  nerrors += test("testdata/repetition.dat:132", "(a|ab|c|bcd){4,}(d*)", "ababcd",
    0);
  nerrors += test("testdata/repetition.dat:134", "(a|ab|c|bcd){0,10}(d*)", "ababcd",
    6, 0, 1, 0, 1, 1, 1);
  nerrors += test("testdata/repetition.dat:136", "(a|ab|c|bcd){1,10}(d*)", "ababcd",
    6, 0, 1, 0, 1, 1, 1);
  nerrors += test("testdata/repetition.dat:137", "(a|ab|c|bcd){2,10}(d*)", "ababcd",
    6, 0, 6, 3, 6, 6, 6);
  nerrors += test("testdata/repetition.dat:138", "(a|ab|c|bcd){3,10}(d*)", "ababcd",
    6, 0, 6, 3, 6, 6, 6);
  nerrors += test("testdata/repetition.dat:139", "(a|ab|c|bcd){4,10}(d*)", "ababcd",
    0);
  nerrors += test("testdata/repetition.dat:141", "(a|ab|c|bcd)*(d*)", "ababcd",
    6, 0, 1, 0, 1, 1, 1);
  nerrors += test("testdata/repetition.dat:143", "(a|ab|c|bcd)+(d*)", "ababcd",
    6, 0, 1, 0, 1, 1, 1);
  nerrors += test("testdata/repetition.dat:149", "(ab|a|c|bcd){0,}(d*)", "ababcd",
    6, 0, 6, 4, 5, 5, 6);
  nerrors += test("testdata/repetition.dat:151", "(ab|a|c|bcd){1,}(d*)", "ababcd",
    6, 0, 6, 4, 5, 5, 6);
  nerrors += test("testdata/repetition.dat:153", "(ab|a|c|bcd){2,}(d*)", "ababcd",
    6, 0, 6, 4, 5, 5, 6);
  nerrors += test("testdata/repetition.dat:155", "(ab|a|c|bcd){3,}(d*)", "ababcd",
    6, 0, 6, 4, 5, 5, 6);
  nerrors += test("testdata/repetition.dat:156", "(ab|a|c|bcd){4,}(d*)", "ababcd",
    0);
  nerrors += test("testdata/repetition.dat:158", "(ab|a|c|bcd){0,10}(d*)", "ababcd",
    6, 0, 6, 4, 5, 5, 6);
  nerrors += test("testdata/repetition.dat:160", "(ab|a|c|bcd){1,10}(d*)", "ababcd",
    6, 0, 6, 4, 5, 5, 6);
  nerrors += test("testdata/repetition.dat:162", "(ab|a|c|bcd){2,10}(d*)", "ababcd",
    6, 0, 6, 4, 5, 5, 6);
  nerrors += test("testdata/repetition.dat:164", "(ab|a|c|bcd){3,10}(d*)", "ababcd",
    6, 0, 6, 4, 5, 5, 6);
  nerrors += test("testdata/repetition.dat:165", "(ab|a|c|bcd){4,10}(d*)", "ababcd",
    0);
  nerrors += test("testdata/repetition.dat:167", "(ab|a|c|bcd)*(d*)", "ababcd",
    6, 0, 6, 4, 5, 5, 6);
  nerrors += test("testdata/repetition.dat:169", "(ab|a|c|bcd)+(d*)", "ababcd",
    6, 0, 6, 4, 5, 5, 6);
  printf("345 test(s), %d error(s).\n", -nerrors);
  return 0;
}
