#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

static unsigned int numFiles = 0;
static unsigned long numBytes = 0;

static void byte(unsigned char v){

	if(numBytes) printf(", ");
	
	printf((numBytes & 0x0F) ? "0x%02X" : "\n\t0x%02X", v);
	numBytes++;
}

int main(int argc, char** argv){

	const char* self = argv[0];
	int c;
	const char* classCvt = 0;
	long len;
	
	if(argc == 1){
		
		fprintf(stderr, "USAGE: %s [-c <path_to_classCvt>] <file 1> [<file 2> [ <file 3> [...]]] > result.c\n", self);
		return -1;
	}

	argv++;
	argc--;
	
	if(argv[0][0] == '-' && argv[0][1] == 'c' && !argv[0][2]){
		
		classCvt = argv[1];
		argv += 2;
		argc -= 2;
	}
	
	printf("#include <avr/pgmspace.h>\n\nconst unsigned char uj_code[] PROGMEM = {\n\t");
	
	while(argc--){
		char* filename = *argv;
	
		fprintf(stderr, "processing '%s'\n", filename);
	
		if(classCvt){
		
			char* t;
			
			filename = tempnam(NULL, NULL);
			if(!filename){
			
				fprintf(stderr, "%s: failed to create a tempfile: %d\n", self, errno);
				return -10;	
			}
			
			t = malloc(strlen(filename) + strlen(classCvt) + strlen(*argv) + 32);
			if(!t){
			
				fprintf(stderr, "%s: failed to alloc a small string. This is unlikely\n", self);
				return -11;	
			}
			
			sprintf(t, "%s < %s > %s", classCvt, *argv, filename);
			
			if(system(t)){
			
				fprintf(stderr, "%s: system() fail: %d\n", self, errno);
				return -12;	
			}
			
			
			free(t);
		}
	
		FILE* f = fopen(filename, "r");
		if(!f){
		
			fprintf(stderr, "%s: failed to open '%s': %d\n", self, *argv, errno);
			return -2;
		}
		
		if(fseek(f, 0, SEEK_END)){
			
			fprintf(stderr, "%s: failed to seek(1) in '%s': %d\n", self, *argv, errno);
			return -3;
		}
		
		len = ftell(f);
		if(len < 0){
			
			fprintf(stderr, "%s: failed to tell in '%s': %d\n", self, *argv, errno);
			return -4;
		}
		
		if(fseek(f, 0, SEEK_SET)){
			
			fprintf(stderr, "%s: failed to seek(2) in '%s': %d\n", self, *argv, errno);
			return -5;
		}
		
		if(len > 0x00FFFFFFUL){
		
			fprintf(stderr, "%s:  file '%s' is %lu bytes, while maximum allowable size is %lu.\n", self, *argv, len, 0x00FFFFFFUL);
			return -6;	
		}
		
		byte(len >> 16);
		byte(len >> 8);
		byte(len);
		
		while((c = fgetc(f)) != EOF){
		
			byte(c);
		}
		
		numFiles++;
		fclose(f);
		if(filename != *argv){
			unlink(filename);
			free(filename);
		}
		argv++;
	}
	
	byte(0);
	byte(0);
	byte(0);
	
	printf("\n};\n");
	
	fprintf(stderr, "%s: processed %u files, producing %lu (0x%lX) bytes of output\n", self, numFiles, numBytes, numBytes);
	
	return 0;
}