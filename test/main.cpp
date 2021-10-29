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
void memory_dump(uint16_t address, int rows);
bool hex_string_to_int(const char *temp_string, uint16_t *return_value);

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
	bool nmi_pin = true;
	bool firq_pin = true;
	bool irq_pin = true;

	memory[0x2000] = 0xb3;
	memory[0x2001] = 0x23;
	memory[0x2002] = 0xb3;
	memory[0x2003] = 0x24;

	memory[0xb323] = 0xf1;
	memory[0xb324] = 0xaa;

	mc6809 cpu(read, write);
	cpu.assign_nmi_line(&nmi_pin);
	cpu.assign_firq_line(&firq_pin);
	cpu.assign_irq_line(&irq_pin);

	// reset system and put welcome message
	printf("emulate_mc6809 (c)2021 elmerucr\n");
	printf("resetting mc6809...\n");
	cpu.reset();
	cpu.status(text_buffer);
	printf("%s\n\n", text_buffer);
	uint16_t temp_pc = cpu.get_pc();
	for (int i=0; i<4; i++) {
		temp_pc += cpu.disassemble_instruction(text_buffer, temp_pc);
		printf("%s\n", text_buffer);
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
		} else if (strcmp(token0, "b") == 0) {
			if (token1 == NULL) {
				uint16_t count = 0;
				for (int i=0; i< 65536; i++) {
					if (cpu.breakpoint[i]) {
						printf("%04x ", i);
						count++;
						if ((count % 4) == 0)
							putchar('\n');
					}
				}
				if (count == 0) {
					puts("no breakpoints");
				} else {
					printf("\n");
				}
			} else {
				uint16_t temp_16bit;
				if (hex_string_to_int(token1, &temp_16bit)) {
					temp_16bit &= 0xffff;
					cpu.toggle_breakpoint(temp_16bit);
					printf("breakpoint %s at $%04x\n",
						cpu.breakpoint[temp_16bit] ? "set" : "cleared",
						temp_16bit);
				} else {
					puts("error: invalid address\n");
				}
			}
		} else if (strcmp(token0, "br") == 0) {
			printf("$%02x\n", cpu.get_br());
		} else if (strcmp(token0, "dr") == 0) {
			printf("$%04x\n", cpu.get_dr());
		} else if (strcmp(token0, "exit") == 0) {
			finished = true;
		} else if (strcmp(token0, "firq") == 0) {
			firq_pin = !firq_pin;
			printf("changed status of firq to %c\n", firq_pin ? '1' : '0');
		} else if (strcmp(token0, "irq") == 0) {
			irq_pin = !irq_pin;
			printf("changed status of irq to %c\n", irq_pin ? '1' : '0');
		} else if (strcmp(token0, "m") == 0) {
			uint16_t temp_pc = cpu.get_pc();

			if (token1 == NULL) {
				memory_dump(temp_pc, 4);
			} else {
				if (!hex_string_to_int(token1, &temp_pc)) {
					putchar('\n');
					puts("error: invalid address\n");
				} else {
					memory_dump(temp_pc, 4);
				}
			}
		} else if (strcmp(token0, "n") == 0) {
			uint16_t to_run;

			if (token1 == NULL) {
				to_run = 0;
			} else {
				if (!hex_string_to_int(token1, &to_run)) {
					putchar('\n');
					puts("error: invalid number\n");
					//to_run = 0;
				}
			}
			bool breakpoint_reached;
			int32_t cycles_done = cpu.execute(&breakpoint_reached);
			if (breakpoint_reached)
				printf("reached breakpoint at: %04x\n", cpu.get_pc());
			printf("last run took %i cycles\n\n", cycles_done);
			cpu.status(text_buffer);
			printf("%s\n\n", text_buffer);
			uint16_t temp_pc = cpu.get_pc();
			for (int i=0; i<4; i++) {
				temp_pc += cpu.disassemble_instruction(text_buffer, temp_pc);
				printf("%s\n", text_buffer);
			}
		} else if (strcmp(token0, "nmi") == 0) {
			nmi_pin = !nmi_pin;
			printf("changed status of nmi to %c\n", nmi_pin ? '1' : '0');
		} else if (strcmp(token0, "r") == 0) {
			cpu.status(text_buffer);
			printf("%s\n\n", text_buffer);
			uint16_t temp_pc = cpu.get_pc();
			for (int i=0; i<4; i++) {
				temp_pc += cpu.disassemble_instruction(text_buffer, temp_pc);
				printf("%s\n", text_buffer);
			}
		} else if (strcmp(token0, "reset") == 0) {
			printf("resetting mc6809...\n\n");
			cpu.reset();
			cpu.status(text_buffer);
			printf("%s\n\n", text_buffer);
			uint16_t temp_pc = cpu.get_pc();
			for (int i=0; i<4; i++) {
				temp_pc += cpu.disassemble_instruction(text_buffer, temp_pc);
				printf("%s\n", text_buffer);
			}
		} else if (strcmp(token0, "s") == 0) {
			cpu.stacks(text_buffer, 8);
			printf("%s\n", text_buffer);
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

void memory_dump(uint16_t address, int rows)
{
	address = address & 0xffff;

	for (int i=0; i<rows; i++ ) {
		printf("\r:%04x", address);
		for (int i=0; i<8; i++) {
			printf(" %02x", read(address));
			address++;
			address &= 0xffff;
		}
		printf("\n");
	}
}

bool hex_string_to_int(const char *temp_string, uint16_t *return_value)
{
	uint16_t val = 0;
	while (*temp_string) {
		/* Get current character then increment */
		uint8_t byte = *temp_string++;
		/* Transform hex character to the 4bit equivalent number */
		if (byte >= '0' && byte <= '9') {
			byte = byte - '0';
		} else if (byte >= 'a' && byte <='f') {
			byte = byte - 'a' + 10;
		} else if (byte >= 'A' && byte <='F') {
			byte = byte - 'A' + 10;
		} else {
			/* Problem, return false and do not write the return value */
			return false;
		}
		/* Shift 4 to make space for new digit, and add the 4 bits of the new digit */
		val = (val << 4) | (byte & 0xf);
	}
	*return_value = val;
	return true;
}
