/*
 * main.cpp  -  part of MC6809
 *
 * (c)2021 elmerucr
 */

#include "mc6809.hpp"
#include <cstdio>
#include <cstring>
#include <cstdlib>

/*
 * This function reads a line from stdin, and returns a pointer to a
 * string. The caller needs to free allocated memory.
 */
char *read_line(void);

char text_buffer[512];
#define TEXT_BUFFER_SIZE 64
uint8_t memory[65536];
extern uint8_t rom[];

uint8_t read(uint16_t address)
{
	if ((address & 0xe000) == 0xe000) {
		return rom[address & 0x1fff];
	} else {
		return memory[address];
	}
}

void write(uint16_t address, uint8_t byte)
{
	memory[address] = byte;
}

int main()
{
	memory[0x2000] = 0xb3;
	memory[0x2001] = 0x23;
	memory[0x2002] = 0xb3;
	memory[0x2003] = 0x24;

	memory[0xb323] = 0xf1;
	memory[0xb324] = 0xaa;

	mc6809 cpu(read, write);

	// reset system and put welcome message
	printf("emulate_mc6809 (c)2021 elmerucr\n");
	printf("resetting mc6809...\n\n");
	cpu.reset();
	cpu.status(text_buffer);
	printf("%s\n", text_buffer);
	uint16_t temp_pc = cpu.get_pc();
	for (int i=0; i<4; i++) {
		temp_pc += cpu.disassemble_instruction(text_buffer, temp_pc);
		printf("%s", text_buffer);
	}

	// prepare repl
	char prompt = '.';
	uint8_t temp_byte;
	char *input_string;
	char *token0, *token1, *token2, *token3;
	bool finished = false;

	do {
		putchar(prompt);
		input_string = read_line();
		token0 = strtok(input_string, " ");
		token1 = strtok(NULL, " ");
		token2 = strtok(NULL, " ");
		token3 = strtok(NULL, " ");

		if (token0 == NULL) {
			/*
			 * Do nothing, just catch the empty token, as
			 * strcmp with NULL pointer results in segfault
			 */
		} else if (strcmp(token0, "ac") == 0) {
			printf("$%02x\n", cpu.get_ac());
		} else if (strcmp(token0, "br") == 0) {
			printf("$%02x\n", cpu.get_br());
		} else if (strcmp(token0, "dr") == 0) {
			printf("$%04x\n", cpu.get_dr());
		} else if (strcmp(token0, "exit") == 0) {
			finished = true;
		} else if (strcmp(token0, "n") == 0) {
			cpu.run(0);
			cpu.status(text_buffer);
			printf("%s\n", text_buffer);
			uint16_t temp_pc = cpu.get_pc();
			for (int i=0; i<4; i++) {
				temp_pc += cpu.disassemble_instruction(text_buffer, temp_pc);
				printf("%s", text_buffer);
			}
		} else if (strcmp(token0, "reset") == 0) {
			printf("resetting mc6809...\n\n");
			cpu.reset();
			cpu.status(text_buffer);
			printf("%s\n", text_buffer);
			uint16_t temp_pc = cpu.get_pc();
			for (int i=0; i<4; i++) {
				temp_pc += cpu.disassemble_instruction(text_buffer, temp_pc);
				printf("%s", text_buffer);
			}
		} else if (strcmp(token0, "s") == 0) {
			// display last 8 values from both stacks
			printf("   sp        us\n");
			for (int i=0; i<8; i++) {
				printf("%04x: %02x  %04x: %02x\n",
					cpu.get_sp() + i,
					cpu.read_8((uint16_t)(cpu.get_sp() + i)),
					cpu.get_us() + i,
					cpu.read_8((uint16_t)(cpu.get_us() + i)));
			}
		} else {
			printf("error: unknown command '%s'\n", input_string);
		}
	} while (!finished);

	free(input_string);
	return 0;
}

char *read_line(void)
{
	int bufsize = TEXT_BUFFER_SIZE;
	int position = 0;
	char *buffer = (char *)malloc(sizeof(char) * bufsize);
	int c;
	if (!buffer) {
		fprintf(stderr, "lsh: allocation error\n");
		exit(1);	// failure
	}
	while (1) {
		// Read a character
		c = getchar();
		// If we hit EOF, replace it with a null character and return.
		if (c == EOF || c == '\n') {
			buffer[position] = '\0';
			return buffer;
		} else {
			buffer[position] = c;
		}
		position++;

    		// If we have exceeded the buffer, reallocate.
    		if (position >= bufsize) {
    			bufsize += TEXT_BUFFER_SIZE;
    			buffer = (char *)realloc(buffer, bufsize);
    			if (!buffer) {
        			fprintf(stderr, "lsh: allocation error\n");
        			exit(1);	// failure
      			}
    		}
  	}
}
