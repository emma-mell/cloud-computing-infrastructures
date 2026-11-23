#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/uio.h>

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* reverse: reverse string s in place.
 *
 * Adapted from K&R2 to accept len as argument.
 */
void reverse(char s[], size_t len) {
  int i, j;
  char c;

  for (i = 0, j = len - 1; i < j; i++, j--) {
    c = s[i];
    s[i] = s[j];
    s[j] = c;
  }
}

/* itoa: convert n to characters in s.
 *
 * Returns strlen of s.
 *
 * Adapted from K&R2 to return strlen of s.
 */
size_t itoa(int n, char s[]) {
  int i, sign;

  if ((sign = n) < 0)
    n = -n;

  i = 0;
  do { /* generate digits in reverse order */
    s[i++] = n % 10 + '0';
  } while ((n /= 10) > 0);

  if (sign < 0)
    s[i++] = '-';

  s[i] = '\0';
  reverse(s, i);

  return i;
}

/* Write a line, i.e., s followed by a linefeed, using the plain syscall write.
 *
 * Expects a proper null-terminated string.
 *
 * Returns the number of bytes written, or -1 and sets errno as write does.
 */
int writeln(int fd, char *s) {
  int ret;
  size_t len = strlen(s);

  s[len] = '\n';
  ret = write(fd, s, len + 1);
  s[len] = '\0';

  return ret;
}

int main(int argc, char *argv[]) {
  // STUDENT CODE HERE

  return 0x42;
}
