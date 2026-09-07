#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "compiler/compiler.h"
#include "vm/vm.h"

#define RUN_OK 0
#define RUN_COMPILE_ERR 1
#define RUN_RUNTIME_ERR 2

bool endsWith(const char *str, const char *suffix) {
	if (!str || !suffix) {
		return false;
	}

	int len = strlen(str);
	int suffixLen = strlen(suffix);

	if (suffixLen > len) {
		return false;
	}

	return strncmp(str + len - suffixLen, suffix, suffixLen) == 0;
}

char *readSourceFile(const char *filename) {
	if (!endsWith(filename, ".lox")) {
		fprintf(stderr, "Program file must end in .lox\n");
		exit(1);
	}

	FILE *srcFile = fopen(filename, "rb");
	if (srcFile == NULL) {
		fprintf(stderr, "Couldn't open file %s\n", filename);
		exit(2);
	}

	fseek(srcFile, 0, SEEK_END);
	size_t size = ftell(srcFile);
	fseek(srcFile, 0, SEEK_SET);

	char *buf = calloc(size + 1, sizeof(char));
	if (buf == NULL) {
		fprintf(stderr, "Not enough memory to load source file\n");
		exit(3);
	}

	size_t bytesRead = fread(buf, sizeof(char), size, srcFile);
	if (bytesRead < size) {
		fprintf(stderr, "Couldn't read file %s\n", filename);
		exit(4);
	}
	buf[bytesRead] = 0;

	fclose(srcFile);
	return buf;
}

int runProgram(const char *src, bool dumpChunk, bool showDebugExecution) {
	Common common;

	Chunk chunk;
	initChunk(&chunk);
	initVirtualMachine(&common.vm, &chunk);

	if (!compile(&common, src, &chunk, dumpChunk)) {
		freeChunk(&chunk);
		return RUN_COMPILE_ERR;
	}

	int res = execute(&common.vm, showDebugExecution);

	freeVirtualMachine(&common.vm);
	freeChunk(&chunk);
	return res;
}

int main(int argc, const char *argv[]) {
	if (argc < 2) {
		fprintf(stderr, "Usage: %s <path>\n", argv[0]);
		return 2;
	}

	bool dumpChunk = false;
	bool showDebugExecution = false;

	for (int i = 1; i < argc - 1; i++) {
		if (argv[i][0] == '-') {
			for (int j = 1; argv[i][j] != '\0'; j++) {
				switch (argv[i][j]) {
					case 'd': dumpChunk = true; break;
					case 's': showDebugExecution = true; break;
					default: fprintf(stderr, "invalid option '%c'\n", argv[i][j]); return 5;
				}
			}
		}
	}

	char *buf = readSourceFile(argv[argc - 1]);
	int res = runProgram(buf, dumpChunk, showDebugExecution);

	if (res == RUN_COMPILE_ERR || res == RUN_RUNTIME_ERR) {
		free(buf);
		return 6;
	}

	free(buf);
	return 0;
}
