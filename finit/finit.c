#include <dirent.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

typedef struct {
    char *token;
    char *replacement;
} Replacement;

void fcopy(FILE *source, FILE *dest, Replacement *r, int rlen) {
  int c;
  while ((c = fgetc(source)) != EOF) {
    if (c == '`') {
      bool possible[rlen];
      memset(possible, true, rlen*sizeof(bool));
      int pos = 0;
      for (; (c = fgetc(source)) != '`'; pos++) {
	if (c == EOF) return;
	for (int i = 0; i < rlen; i++) {
	  if (possible[i] && r[i].token[pos] != c) {
	    possible[i] = false;
	  }
	}
      }
      for (int i = 0; i < rlen; i++) {
	if (possible[i] && r[i].token[pos] == '\0') {
	  fputs(r[i].replacement, dest);
	  break;
	}
      }
      continue;
    } else if (c == '\\' && (c = fgetc(source)) == EOF) {
      return;
    }
    fputc(c, dest);
  }
}

Replacement* parse_replacements(int argc, char *argv[]) {
  Replacement *r = malloc(sizeof(Replacement) * argc);
  if (!r) {
    return NULL;
  }
  for (int i = 0; i < argc; i++) {
    r[i].replacement = NULL;
    if (argv[i][0] == '\0') {
      r[i].token = NULL;
      continue;
    }
    r[i].token = argv[i];
    for (int j = 0; argv[i][j + 1] != '\0'; j++) {
      if (argv[i][j] == '\\') {
        j++;
      } else if (argv[i][j] == '=') {
	argv[i][j] = '\0';
	r[i].replacement = &argv[i][j + 1];
	break;
      }
    }
    if (!r[i].replacement) {
      return NULL;
    }
  }
  return r;
}

int main(int argc, char *argv[]) {
  if (argc < 3) {
    printf("Usage: finit <name> <template> [<args>]\n");
    exit(1);
  }

  Replacement *r = parse_replacements(argc - 3, &argv[3]);
  if (!r) {
    printf("Error parsing replacement arguments.\n");
    exit(1);
  }
  
  const char templates_path[] = "/home/nathaniel/Documents/c/finit/templates/";
  int base_size = sizeof(templates_path) + strlen(argv[2]);
  char *requested_path = malloc(base_size + 257);
  if (!requested_path) {
    printf("malloc() failed\n");
    exit(1);
  }
  strcpy(requested_path, templates_path);
  strcat(requested_path, argv[2]);
  
  DIR *template = opendir(requested_path);
  if (!template) {
    printf("No such template %s\n", argv[1]);
    exit(1);
  }

  if (mkdir(argv[1], 0777)) {
    perror("mkdir() failed");
    exit(1);
  }
  if (chdir(argv[1])) {
    perror("chdir() failed");
    exit(1);
  }
  
  requested_path[base_size - 1] = '/';
  struct dirent *dat;
  while ((dat = readdir(template)) != NULL) {
    if (dat->d_name[0] == '.') {
      continue;
    }
    strcpy(&requested_path[base_size], dat->d_name);
    FILE *f = fopen(requested_path, "r");
    FILE *copy = fopen(dat->d_name, "w");
    if (!f || !copy) {
      perror("fopen() failed");
      exit(1);
    }
    fcopy(f, copy, r, argc - 3);
    fclose(f);
    fclose(copy);
  }

  closedir(template);
  free(requested_path);
  free(r);
  return 0;
}
