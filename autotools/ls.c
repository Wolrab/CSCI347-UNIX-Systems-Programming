/* This program lists the files contained within the current working directory.
 *
 */

#include <assert.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

/* Given a DIR object, dir, list the files within that directory.  
 *
 */
void list(DIR* dir) {
  assert(dir != NULL);
  struct dirent* ent = readdir(dir);
  while(ent != NULL) {
    puts(ent->d_name);
    ent = readdir(dir);
  }
}

/* List the contents of the current working directory.
 * 
 */
int main() {

  DIR* d = opendir(".");

  if(d == NULL) {
    perror ("diropen");
    exit(1);
  }

  list(d);
  closedir(d);
  return 0;
}
