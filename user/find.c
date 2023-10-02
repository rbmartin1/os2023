#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"

void find(char path, charfile)
{
  char buf[512], *p;
  int fd;
  struct dirent de;
  struct stat st;

  if ((fd = open(path, O_RDONLY)) < 0)
  {
    fprintf(2, "find: cannot open %s\n", path);
    return;
  }

  if (fstat(fd, &st) < 0)
  {
    fprintf(2, "find: cannot stat %s\n", path);
    close(fd);
    return;
  }

  switch (st.type)
  {
  case T_DIR:
    if (strlen(path) + 1 + DIRSIZ + 1 > sizeof(buf))
    {
      fprintf(2, "find: path too long\n");
      break;
    }
    strcpy(buf, path);
    p = buf + strlen(buf);
    p++ = '/';

    while (read(fd, &de, sizeof(de)) == sizeof(de))
    {
      if (de.inum == 0)
        continue;

      if (strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0)
        continue;

      memmove(p, de.name, DIRSIZ);
      p[DIRSIZ] = 0;

      if (stat(buf, &st) < 0)
      {
        fprintf(2, "find: cannot stat %s\n", buf);
        continue;
      }

      if (st.type == T_FILE && strcmp(de.name, file) == 0)
      {
        printf("%s\n", buf);
      }
      else if (st.type == T_DIR)
      {
        find(buf, file);
      }
    }
    break;
  }

  close(fd);
}

int main(int argc, charargv[])
{
  if (argc != 3)
  {
    fprintf(2, "Usage: find <directory> <file>\n");
    exit(1);
  }
  find(argv[1], argv[2]);
  exit(0);
}