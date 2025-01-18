#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <raymath.h>
#include "render_model.h"

char** split(const char* str, const char* delimiter, int* count) {
    char* strCopy = strdup(str);
    if (strCopy == NULL) {
        *count = 0;
        return NULL;
    }

    int capacity = 64;
    char** tokens = malloc(capacity * sizeof(char*));
    if (tokens == NULL) {
        free(strCopy);
        *count = 0;
        return NULL;
    }

    int index = 0;
    char* token = strtok(strCopy, delimiter);
    while (token != NULL) {
        if (index >= capacity) {
            capacity *= 2;
            char** newTokens = realloc(tokens, capacity * sizeof(char*));
            if (newTokens == NULL) {
                free(tokens);
                free(strCopy);
                *count = 0;
                return NULL;
            }
            tokens = newTokens;
        }
        tokens[index++] = strdup(token);
        token = strtok(NULL, delimiter);
    }

    *count = index;
    free(strCopy);
    return tokens;
}

void freeTokens(char** tokens, int count) {
    for (int i = 0; i < count; i++) {
        free(tokens[i]);
    }
    free(tokens);
}

RenderModel parse_obj(const char *filename) {
  int vn = 0, vtn = 0, vnn = 0, fn = 0;
  
  FILE *file;
  char line[2048];

  file = fopen(filename, "r"); 
  while (fgets(line, sizeof(line), file)) {
    char *endptr;
    int count;
    char **pms = split(line, " ", &count);
    
    if(!strcmp(pms[0], "v")){
      vn++;
    } else if(!strcmp(pms[0], "vn")){
      vnn++;
    } else if(!strcmp(pms[0], "vt")){
      vtn++;
    } if(!strcmp(pms[0], "f")){
      fn++;
    }

    freeTokens(pms, count);
  }
  fclose(file);

  
  RenderModel obj = {
    .vs = malloc(sizeof(Vector3)*vn),
    .vts = malloc(sizeof(Vector3)*vtn),
    .vns = malloc(sizeof(Vector3)*vnn),
    .fs = malloc(sizeof(Face)*fn),
    .vn = 0, .vtn = 0, .vnn = 0, .fn = 0,
  };
  
  file = fopen(filename, "r");
  while (fgets(line, sizeof(line), file)) {
    char *endptr;
    int count;
    char **pms = split(line, " ", &count);
    
    if(!strcmp(pms[0], "v")){
      Vector3 v = {strtof(pms[1], &endptr), strtof(pms[2], &endptr), strtof(pms[3], &endptr)};
      obj.vs[obj.vn++] = v;
    } else if(!strcmp(pms[0], "vn")){
      Vector3 n = {strtof(pms[1], &endptr), strtof(pms[2], &endptr), strtof(pms[3], &endptr)};
      obj.vns[obj.vnn++] = n;
    } else if(!strcmp(pms[0], "vt")){
      Vector2 uv = {strtof(pms[1], &endptr), strtof(pms[2], &endptr)};
      obj.vts[obj.vtn++] = uv;
    } else if(!strcmp(pms[0], "f")){
      Face f = {
	.vi = malloc(sizeof(int)*(count-1)),
	.vti = malloc(sizeof(int)*(count-1)),
	.vni = malloc(sizeof(int)*(count-1)),
	.cnt = 0,
      }; 
      for(int i = 1; i < count; i++){
	int cnt;
	char **fpms = split(pms[i], "//", &cnt);
	if(cnt == 2){
	 f.vi[f.cnt] = strtol(fpms[0], &endptr, 10);
	 f.vni[f.cnt++] = strtol(fpms[1], &endptr, 10);
	} else {
	  freeTokens(fpms, cnt);
	  fpms = split(pms[i], "/", &cnt);
	  f.vi[f.cnt] = strtol(fpms[0], &endptr, 10);
	  f.vti[f.cnt] = strtol(fpms[1], &endptr, 10);
	  f.vni[f.cnt++] = strtol(fpms[2], &endptr, 10);
	}
	freeTokens(fpms, cnt);
      }
      obj.fs[obj.fn++] = f;
    }

    freeTokens(pms, count);
  }
  
  fclose(file);
  return obj;
}
