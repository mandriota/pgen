#include <getopt.h>
#include <memory.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <stdio.h>

#define STR_IDENT(x) #x
#define STR_VALUE(x) STR_IDENT(x)

#define CHARSET_LOWERS "qwertyuiopasdfghjklzxcvbnm"
#define CHARSET_UPPERS "QWERTYUIOPASDFGHJKLZXCVBNM"
#define CHARSET_DIGITS "0123456789"
#define CHARSET_SEQUEL "^*+-=_?.,:;\\|/\"'$%&@#()[]{}"
#define DEFAULT_CHARSET CHARSET_LOWERS CHARSET_UPPERS CHARSET_DIGITS

enum {
  BUF_LEN = 4096,
  DEFAULT_PWD_LEN = 8,
};

size_t push(char *restrict dst, const char *src, size_t dst_off,
            size_t dst_cap);
size_t parse(const char *s);
size_t trim_last_newline(const char * src, size_t src_len);
void print_help();

int main(int argc, char * argv[]) {
  char buf[BUF_LEN];
  size_t pwd_rem = DEFAULT_PWD_LEN;

  char charset[1 + (unsigned char) ~0];
  size_t charset_len = 0;

  assert(BUF_LEN > sizeof charset);

  int opt;
  while ((opt = getopt(argc, argv, "hn:c:Cluds")) != -1) {
	switch (opt) {
	case 'h':
	  print_help();
	  return 0;
	case 'n':
	  pwd_rem = parse(optarg);
	  break;
	case 'c':
	  if ((charset_len = push(charset, optarg, charset_len, sizeof charset))+1 == 0)
		charset_len = sizeof charset;
	  break;
	case 'C':
	  buf[trim_last_newline(buf, read(STDIN_FILENO, buf, sizeof charset))] = '\0';
	  charset_len = push(charset, buf, charset_len, sizeof charset);
	  break;
	case 'l':
	  charset_len = push(charset, CHARSET_LOWERS, charset_len, sizeof charset);
	  break;
	case 'u':
	  charset_len = push(charset, CHARSET_UPPERS, charset_len, sizeof charset);
	  break;
	case 'd':
	  charset_len = push(charset, CHARSET_DIGITS, charset_len, sizeof charset);
	  break;
	case 's':
	  charset_len = push(charset, CHARSET_SEQUEL, charset_len, sizeof charset);
	  break;
	case '?':
	  return 1;
	}
  }

  if (charset_len == 0)
	charset_len = push(charset, DEFAULT_CHARSET, charset_len, sizeof charset);

  size_t i;
  sranddev();

  do {
	for (i = 0; i < sizeof buf && i < pwd_rem; ++i)
	  buf[i] = charset[rand()%charset_len];

	pwd_rem -= i;
	write(STDOUT_FILENO, buf, i);
  } while (pwd_rem);

  buf[0] = '\n';
  write(STDOUT_FILENO, buf, 1);
  
  return 0;
}

size_t parse(const char * s) {
  size_t r = 0;
  
  for (size_t i = 0; s[i]; ++i)
	if (s[i] >= '0' && s[i] <= '9') r = r*10+s[i]-'0';
	else break;
  
  return r;
}

size_t push(char * restrict dst, const char * src, size_t dst_off, size_t dst_cap) {
  char * ptr = memccpy(dst+dst_off, src, '\0', dst_cap - dst_off);
  if (ptr == NULL) return dst_off;
  return ptr-dst-1;
}

size_t trim_last_newline(const char * src, size_t src_len) {
  return src_len-(src[src_len-1] == '\n');
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
  puts("\tCharset (defaults to \"" DEFAULT_CHARSET "\"):");
  puts("");
  puts("\t -c <string>");
  puts("\t   add a custom string to charset");
  puts("\t -C");
  puts("\t   add a custom string to charset (read from stdin)");
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
