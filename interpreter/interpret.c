#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEBUG

#define MAX_STACK_LENGTH 256

int stack[MAX_STACK_LENGTH];
int slen = 0;

int *variables = NULL;
char **variable_names = NULL;
int vlen = 0;

int *function_locations = NULL;
char **function_names = NULL;
int flen = 0;

int *return_to = NULL;
int rlen = 0;

int instruction = 0;

unsigned char *op_codes = NULL;
int **op_args = NULL;
int clen = 0;

void panic(char *error_message) {
  printf("%s\n", error_message);
  exit(1);
}

void nop(int *args) {
  return;
}

void op_push(int *args) {
  if (slen >= MAX_STACK_LENGTH) {
    printf("Stack overflow caused by line %d.\n", instruction);
    panic("Run time error.");
  }
  stack[slen++] = args[0];
}

void op_pop(int *args) {
  slen -= args[0];
  if (slen < 0) {
    printf("Stack underflow caused by line %d.\n", instruction);
    panic("Run time error.");
  }
}

void op_dup(int *args) {
  op_push((int[]) {stack[slen - 1]});
}

void op_swp(int *args) {
  int tmp = stack[slen - 1];
  stack[slen - 1] = stack[slen - 2];
  stack[slen - 2] = tmp;
}

void op_add(int *args) {
  int sum = stack[slen - 2] + stack[slen - 1];
  op_pop((int[]) {2});
  op_push(&sum);
}

void op_sub(int *args) {
  int diff = stack[slen - 2] - stack[slen - 1];
  op_pop((int[]) {2});
  op_push(&diff);
}

void op_mul(int *args) {
  int prod = stack[slen - 2]*stack[slen - 1];
  op_pop((int[]) {2});
  op_push(&prod);
}

void op_div(int *args) {
  if (stack[slen - 2] == 0) {
    printf("Division by 0 at line %d.\n", instruction);
    panic("Run time error.");
  }
  int quot = stack[slen - 2]/stack[slen - 1];
  op_pop((int[]) {2});
  op_push(&quot);
}

void op_cmp(int *args) {
  if (stack[slen - 1] != stack[slen - 2]) instruction++;
}

void op_grt(int *args) {
  if (stack[slen - 1] > stack[slen - 2]) instruction++;
}

void op_lss(int *args) {
  if (stack[slen - 1] < stack[slen - 2]) instruction++;
}

void op_jmp(int *args) {
  if (args[0] < 0 || args[0] > clen) {
    printf("Invalid jump to instruction %d, at line %d.\n", args[0], instruction);
    panic("Run time error.");
  }
  instruction = args[0] - 1;
}

void op_call(int *args) {
  return_to = realloc(return_to, sizeof(int)*(++rlen));
  if (!return_to) panic("Out of memory.");
  return_to[rlen - 1] = instruction;
  op_jmp(args);
}

void op_sve(int *args) {
  variables[args[0]] = stack[slen - 1];
  op_pop((int[]) {1});
}

void op_lde(int *args) {
  op_push(&variables[args[0]]);
}

void op_dsp(int *args) {
  switch (args[0]) {
  case 0:
    printf("Display: %d\n", stack[slen - 1]);
    break;
  case 1:
    printf("%c", stack[slen - 1]);
    break;
  }
}

void op_ret(int *args) {
  if (rlen > 0) {
    instruction = return_to[--rlen];
    return_to = realloc(return_to, sizeof(int)*rlen);
    if (rlen > 0 && !return_to) panic("Out of memory");
  } else {
    exit(0);
  }
}

typedef struct {
  unsigned char code;
  void (*function)(int *args);
  int req_args;
  char *name;
} operation_code;

void set_code(operation_code ***codes, int *len, void (*function)(int *args), char *name, int req_args) {
  *codes = realloc(*codes, sizeof(operation_code *)*++(*len));
  if (!codes) panic("Out of memory.");
  (*codes)[*len - 1] = malloc(sizeof(operation_code));
  if (!(*codes)[*len - 1]) panic("Out of memory.");
  (*codes)[*len - 1]->code = (unsigned char) (*len - 1);
  (*codes)[*len - 1]->function = function;
  (*codes)[*len - 1]->name = name;
  (*codes)[*len - 1]->req_args = req_args;
}

void split_string(char *str, char *spl, char ***list, int *len) {
  *list = malloc(sizeof(char *));
  char *dup = strdup(str);
  if (!list || !dup) panic("Out of memory.");
  *list[0] = dup;
  *len = 1;
  for (int i = 0; dup[i] != '\0'; i++) {
    char *token = NULL;
    for (int j = 0; ; j++) {
      if (dup[i + j] != spl[j] || dup[j + i] == '\0') break;
      if (spl[j + 1] == '\0') {
        dup[i] = '\0';
        if (dup[i + j + 1] != '\0') token = &dup[i + j + 1];
      }
    }
    if (token != NULL) {
      *list = realloc(*list, (*len + 1)*sizeof(char *));
      if (!(*list)) panic("Out of memory.");
      (*list)[(*len)++] = token;
    }
  }
}

int get_number(char *str) {
  int len = 0;
  while (str[len] != '\0') len++;
  int value = 0;
  for (int i = 0; i < len; i++) {
    value += (str[i] - '0')*pow(10, len - i - 1);
  }
  return value;
}

void parse_operation(char **operations, operation_code **codes) {
  char **arguments_str = NULL;
  int alen;
  split_string(*operations, " ", &arguments_str, &alen);
  
  op_args[clen] = malloc(sizeof(int)*(alen - 1));
  
  bool found_name = false;
  for (int i = 0; codes[i]->name != NULL; i++) {
    if (strcmp(arguments_str[0], codes[i]->name) == 0) {
      if (alen - 1 != codes[i]->req_args) {
	printf("Invalid number of arguments_str for %s.\n", codes[i]->name);
	panic("Error parsing.");
      }
      op_codes[clen] = codes[i]->code;
      found_name = true;
      break;
    }
  }
  if (!found_name) {
    printf("Unknown function \'%s\'.\n", arguments_str[0]);
    panic("Error parsing.");
  }
  for (int i = 1; i < alen; i++) {
    switch (arguments_str[i][0]) {
    case '$':
      found_name = false;
      for (int j = 0; j < vlen; j++) {
        if (strcmp(&arguments_str[i][1], variable_names[j]) == 0) {
          op_args[clen][i - 1] = j;
          found_name = true;
          break;
        }
      }
      if (!found_name) {
        variables = realloc(variables, sizeof(int)*(vlen + 1));
        variable_names = realloc(variable_names, sizeof(char *)*(vlen + 1));
        if (!variables || !variable_names) panic("Out of memory.");
        op_args[clen][i - 1] = vlen;
        variable_names[vlen] = strdup(&arguments_str[i][1]);
        vlen++;
      }
      break;
    case '@':
      found_name = false;
      for (int j = 0; j < flen; j++) {
	if (strcmp(&arguments_str[i][1], function_names[j]) == 0) {
	  op_args[clen][i - 1] = function_locations[j];
	  found_name = true;
	  break;
	}
      }
      if (!found_name) {
	printf("Unknown function %s.\n", &arguments_str[i][1]);
	panic("Error parsing.");
      }
      break;
    default:
      op_args[clen][i - 1] = get_number(arguments_str[i]);
      break;
    }
  }

#ifdef DEBUG
  printf("%d ", clen);
  for (int i = 0; i < alen; i++) {
    printf("%s (%d) ", arguments_str[i], i ? op_args[clen][i - 1] : op_codes[clen]);
  }
  printf("\n");
#endif
  
  clen++;

  free(arguments_str[0]);
  free(arguments_str);
}

void parse_function(char **functions, operation_code **codes) {
  char **operations_str = NULL;
  int olen;
  split_string(*functions, "\n", &operations_str, &olen);
  
  op_codes = realloc(op_codes, sizeof(unsigned char)*(clen + olen));
  op_args = realloc(op_args, sizeof(int *)*(clen + olen));
  function_locations = realloc(function_locations, sizeof(int)*(flen + 1));
  function_names = realloc(function_names, sizeof(char *)*(flen + 1));
  if (!op_codes || !op_args || !function_locations || !function_names) panic("Out of memory.");

  function_names[flen] = strdup(operations_str[0]);
  if (!function_names[flen]) panic("Out of memory.");
  function_locations[flen] = clen;
  for (int i = 0; i < flen; i++) {
    if (strcmp(function_names[flen], function_names[i]) == 0) {
      printf("Redeclaration of function %s on line %d. Originally declared on line %d.\n", function_names[flen], function_locations[flen], function_locations[i]);
      panic("Error parsing.");
    }
  }
  if (strcmp(function_names[flen], "start") == 0) {
    instruction = function_locations[flen];
  }
  flen++;
  char *nop = "nop";
  parse_operation(&nop, codes); //operations_str[0] is the function's name, so just make it a nop. This is done to not offset the lines of everything every time a function is declaired, since that would mess up jmp. (think of a better way though, cause this is dumb)
  for (int i = 1; i < olen; i++) {
    parse_operation(&operations_str[i], codes);
  }

  free(operations_str[0]);
  free(operations_str);
}

void parse(char *program_str, operation_code **codes) {
  char **functions_str = NULL;
  int flen;
  split_string(program_str, ":", &functions_str, &flen);
  for (int i = 0; i < flen; i++) {
    if (functions_str[i][0] == '\0') continue;
    parse_function(&functions_str[i], codes);
  }
#ifdef DEBUG
  printf("\n");
#endif
  free(functions_str[0]);
  free(functions_str);
}

int main(int argc, char *argv[]) {
  operation_code **codes = NULL;
  int oplen = 0;
  set_code(&codes, &oplen, &nop, "nop", 0);
  set_code(&codes, &oplen, &op_push, "push", 1);
  set_code(&codes, &oplen, &op_pop, "pop", 1);
  set_code(&codes, &oplen, &op_dup, "dup", 0);
  set_code(&codes, &oplen, &op_swp, "swp", 0);
  set_code(&codes, &oplen, &op_add, "add", 0);
  set_code(&codes, &oplen, &op_sub, "sub", 0);
  set_code(&codes, &oplen, &op_mul, "mul", 0);
  set_code(&codes, &oplen, &op_div, "div", 0);
  set_code(&codes, &oplen, &op_cmp, "cmp", 0);
  set_code(&codes, &oplen, &op_grt, "grt", 0);
  set_code(&codes, &oplen, &op_lss, "lss", 0);
  set_code(&codes, &oplen, &op_jmp, "jmp", 1);
  set_code(&codes, &oplen, &op_call, "call", 1);
  set_code(&codes, &oplen, &op_sve, "sve", 1);
  set_code(&codes, &oplen, &op_lde, "lde", 1);
  set_code(&codes, &oplen, &op_dsp, "dsp", 1);
  set_code(&codes, &oplen, &op_ret, "ret", 0);
  set_code(&codes, &oplen, NULL, NULL, 0);
#ifdef DEBUG
  for (int i = 0; i < oplen; i++) {
    printf("%s : %d %p\n", codes[i]->name, codes[i]->code, codes[i]->function);
  }
  printf("\n");
#endif
  char *code_str = NULL;
  int input_len = 0;
  FILE *file = fopen("prog", "r");
  if (!file) panic("Failed to open file.");
  int c;
  while ((c = fgetc(file)) != EOF) {
    code_str = realloc(code_str, sizeof(char)*(input_len + 2));
    if (!code_str) panic("Out of memory.");
    code_str[input_len] = (char) c;
    input_len++;
  }
  code_str[input_len] = '\0';
  fclose(file);
  parse(code_str, codes);
  
  while (1) {
#ifdef DEBUG
    printf("%d %s\n", instruction, codes[op_codes[instruction]]->name);
#endif
    codes[op_codes[instruction]]->function(op_args[instruction]);
#ifdef DEBUG
    for (int i = 0; i <= slen; i++) {
      printf("%d%c\n", stack[i], slen - i ? '\0' : '<');
    }
#endif
    instruction++;
    if (instruction >= clen) panic("Missing ret at end of program.");
  }
}
