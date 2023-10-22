#include <getopt.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define STR_IDENT(x) #x
#define STR_VALUE(x) STR_IDENT(x)

#define CHARSET_LOWERS "qwertyuiopasdfghjklzxcvbnm"
#define CHARSET_UPPERS "QWERTYUIOPASDFGHJKLZXCVBNM"
#define CHARSET_DIGITS "0123456789"
#define CHARSET_SPECIALS "^*+-=_?.,:;\\|/\"'$%&@#()[]{}"
#define CHARSET_DEFAULT CHARSET_LOWERS CHARSET_UPPERS CHARSET_DIGITS

#define PLENGTH_DEFAULT 8

size_t parse(const char *s);
size_t append(char *restrict dst, const char *src, size_t dst_off,
              size_t dst_cap);

void print_help();

int main(int argc, char * argv[]) {
  char result[4096];
  size_t result_len = PLENGTH_DEFAULT;

  char charset[1<<sizeof(char)*8];
  size_t charset_len = 0;

  int opt;
  while ((opt = getopt(argc, argv, "hn:c:luds")) != -1) {
	switch (opt) {
	case 'h':
	  print_help();
	  return 0;
	case 'n':
	  result_len = parse(optarg);
	  break;
	case 'c':
	  if ((charset_len = append(charset, optarg, charset_len, sizeof charset))+1 == 0)
		charset_len = sizeof charset;
	  break;
	case 'l':
	  charset_len = append(charset, CHARSET_LOWERS, charset_len, sizeof charset);
	  break;
	case 'u':
	  charset_len = append(charset, CHARSET_UPPERS, charset_len, sizeof charset);
	  break;
	case 'd':
	  charset_len = append(charset, CHARSET_DIGITS, charset_len, sizeof charset);
	  break;
	case 's':
	  charset_len = append(charset, CHARSET_SPECIALS, charset_len, sizeof charset);
	  break;
	case '?':
	  return 1;
	}
  }

  if (charset_len == 0)
	charset_len = append(charset, CHARSET_DEFAULT, charset_len, sizeof charset);

  size_t i;
  sranddev();

  do {
	for (i = 0; i < sizeof result && i < result_len; ++i)
	  result[i] = charset[rand()%charset_len];

	result_len -= i;
	write(STDOUT_FILENO, result, i);
  } while (result_len);

  result[0] = '\n';
  write(STDOUT_FILENO, result, 1);
  
  return 0;
}

size_t parse(const char * s) {
  size_t r = 0;
  
  for (size_t i = 0; s[i]; ++i)
	if (s[i] >= '0' && s[i] <= '9') r = r*10+s[i]-'0';
	else break;
  
  return r;
}

size_t append(char * restrict dst, const char * src, size_t dst_off, size_t dst_cap) {
  return (char*) memccpy(dst+dst_off, src, '\0', dst_cap - dst_off)-dst-1;
}

void print_help() {
  puts("PGen (Password Generator). (c) 2023 Mark Mandriota");
  puts("");
  puts("Usage:");
  puts("");
  puts("\tpgen [options]");
  puts("");
  puts("Options:");
  puts("\t -h");
  puts("\t   print this message");
  puts("\t -n <number>");
  puts("\t   password length (defaults to " STR_VALUE(PLENGTH_DEFAULT) ")");
  puts("");
  puts("\tCharset (defaults to \"" CHARSET_DEFAULT "\"):");
  puts("");
  puts("\t -c <string>");
  puts("\t   add a custom string to charset");
  puts("\t -l");
  puts("\t   add lower letters to charset");
  puts("\t -u");
  puts("\t   add upper letters to charset");
  puts("\t -d");
  puts("\t   add digits to charset");
  puts("\t -s");
  puts("\t   add special characters to custom charset");
  puts("");
  fflush(stdout);
}
