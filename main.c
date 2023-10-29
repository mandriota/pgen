#include <getopt.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>

#define STR_IDENT(x) #x
#define STR_VALUE(x) STR_IDENT(x)
#define STR_STR_LEN(s) s, sizeof s

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
  write(STDOUT_FILENO,
	    STR_STR_LEN("PGen (Password Generator). (c) 2023 Mark Mandriota\n\n\n"
					"Usage:\n\n"
					"\tpgen [options]\n\n"
					"Options:\n\n"
					"\t -h\n"
					"\t   print this message\n"
					"\t -n <number>\n"
					"\t   password length (defaults to " STR_VALUE(PLENGTH_DEFAULT) ")\n\n"
					"\tCharset (defaults to \"" DEFAULT_CHARSET "\"):\n\n"
					"\t -c <string>\n"
					"\t   add a custom string to charset\n"
					"\t -C\n"
					"\t   add a custom string to charset (read from stdin)\n"
					"\t -l\n"
					"\t   add lower letters to charset\n"
					"\t -u\n"
					"\t   add upper letters to charset\n"
					"\t -d\n"
					"\t   add digits to charset\n"
					"\t -s\n"
					"\t   add special characters to custom charset\n\n"));
}
