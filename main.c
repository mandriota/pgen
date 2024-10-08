// Password Generator.
// Copyright (C) 2023  Mark Mandriota
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
#include <fcntl.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <sys/random.h>
#include <unistd.h>

#define STR_IDENT(x) #x
#define STR_VALUE(x) STR_IDENT(x)

#define WRITE_CONST(fd, str_lit) (write(fd, str_lit, sizeof str_lit))

#define CHARSET_LOWERS "qwertyuiopasdfghjklzxcvbnm"
#define CHARSET_UPPERS "QWERTYUIOPASDFGHJKLZXCVBNM"
#define CHARSET_DIGITS "0123456789"
#define CHARSET_SEQUEL "^*+-=_?.,:;\\|/\"'$%&@#()[]{}"
#define DEFAULT_CHARSET CHARSET_LOWERS CHARSET_UPPERS CHARSET_DIGITS

enum {
  BUF_LEN = 4096,
  CHARSET_LEN = 1 + (unsigned char)~0,
  ENTROPY_LEN = 256,
  DEFAULT_PWD_LEN = 8,
};

_Static_assert(BUF_LEN % ENTROPY_LEN == 0, "BUF_LEN should be a multiple of ENTROPY_LEN");
_Static_assert(BUF_LEN > CHARSET_LEN, "BUF_LEN should be bigger than CHARSET_SIZE");

size_t parse(const char s[restrict static 1]) {
  size_t r = 0;

  for (size_t i = 0; s[i]; ++i) {
    if (s[i] >= '0' && s[i] <= '9') {
      r = r * 10 + s[i] - '0';
    } else {
      break;
    }
  }

  return r;
}

size_t push(char dst[restrict static 1],
            const char src[restrict static 1],
            size_t dst_off,
            size_t dst_cap) {
  char *ptr = memccpy(dst + dst_off, src, '\0', dst_cap - dst_off);
  if (ptr == NULL)
    return dst_off;
  return ptr - dst - 1;
}

size_t trim_last_newline(const char src[restrict static 1], size_t src_len) {
  return src_len - (src[src_len - 1] == '\n');
}

void print_help() {
  // clang-format off
  WRITE_CONST(STDOUT_FILENO,
	  "PGen (Password Generator). (c) 2023 Mark Mandriota\n\n\n"
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
	  "\t   add special characters to charset\n\n"
  );
  // clang-format on
}

int main(int argc, char *argv[]) {
  char buf[BUF_LEN];
  size_t pwd_rem = DEFAULT_PWD_LEN;

  char charset[CHARSET_LEN];
  size_t charset_len = 0;

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
      charset_len = push(charset, optarg, charset_len, sizeof charset);
      if (charset + 1 == 0)
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

  size_t toread = BUF_LEN;

  do {
    if (pwd_rem < toread)
      toread = pwd_rem;

    for (size_t i = 0; i < toread; i += ENTROPY_LEN) {
      if (getentropy(buf + i, ENTROPY_LEN)) {
        WRITE_CONST(STDERR_FILENO, "failed to get entropy");
        return 1;
      };
    }

    for (size_t i = 0; i < toread; ++i)
      buf[i] = charset[buf[i] % charset_len];

    pwd_rem -= toread;
    write(STDOUT_FILENO, buf, toread);
  } while (pwd_rem);

  WRITE_CONST(STDOUT_FILENO, "\n");

  return 0;
}
