#include "monitor/monitor.h"
#include "monitor/expr.h"
#include "monitor/watchpoint.h"
#include "nemu.h"

#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

void cpu_exec(uint32_t);

/* We use the ``readline'' library to provide more flexibility to read from stdin. */
char* rl_gets() {
	static char *line_read = NULL;

	if (line_read) {
		free(line_read);
		line_read = NULL;
	}

	line_read = readline("(nemu) ");

	if (line_read && *line_read) {
		add_history(line_read);
	}

	return line_read;
}

static int cmd_c(char *args) {
	cpu_exec(-1);
	return 0;
}

static int cmd_q(char *args) {
	return -1;
}

static int cmd_si(char *args) {
	if(args == NULL) {
		cpu_exec(1);
	} else {
		volatile uint32_t n = atoi(args);
		cpu_exec(n);
	}
	return 0;
}

static int cmd_info(char *args) {
	if(args == NULL) {
		return 0;
	}
	if(strcmp(args, "r") == 0) {
		printf("eax\t0x%x\t%d\n", cpu.eax, cpu.eax);
		printf("ecx\t0x%x\t%d\n", cpu.ecx, cpu.ecx);
		printf("edx\t0x%x\t%d\n", cpu.edx, cpu.edx);
		printf("ebx\t0x%x\t%d\n", cpu.ebx, cpu.ebx);
		printf("esp\t0x%x\t%d\n", cpu.esp, cpu.esp);
		printf("ebp\t0x%x\t%d\n", cpu.ebp, cpu.ebp);
		printf("esi\t0x%x\t%d\n", cpu.esi, cpu.esi);
		printf("edi\t0x%x\t%d\n", cpu.edi, cpu.edi);
		printf("eip\t0x%x\n", cpu.eip);
	} else if(strcmp(args, "w") == 0) {
	}

	return 0;
}

static int cmd_x(char *args) {
	if(args == NULL) {
		return 0;
	} else {
		uint32_t len;
		swaddr_t addr;
		int i;
		sscanf(args, "%d %x", &len, &addr);
		for(i = 0; i < len; i ++) {
			if(i % 4 == 0) {
				printf("  %x:\t", addr);
			}

			uint32_t memory = swaddr_read(addr ++, 1);

			if(memory < 16) printf("0");
			printf("%x", memory);

			if(i % 4 == 3) printf("\n");
			else printf(" ");
		}
		if(i % 4 != 0) printf("\n");
	}
	
	return 0;
}

static int cmd_help(char *args);

static struct {
	char *name;
	char *description;
	int (*handler) (char *);
} cmd_table [] = {
	{ "help", "Display informations about all supported commands", cmd_help },
	{ "c", "Continue the execution of the program", cmd_c },
	{ "q", "Exit NEMU", cmd_q },
	{ "si", "Step one instruction exactly", cmd_si },
	{ "x", "Scan memory", cmd_x },
	{ "info", "Print information of registers or watchpoints", cmd_info },

	/* TODO: Add more commands */

};

#define NR_CMD (sizeof(cmd_table) / sizeof(cmd_table[0]))

static int cmd_help(char *args) {
	/* extract the first argument */
	char *arg = strtok(NULL, " ");
	int i;

	if(arg == NULL) {
		/* no argument given */
		for(i = 0; i < NR_CMD; i ++) {
			printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
		}
	}
	else {
		for(i = 0; i < NR_CMD; i ++) {
			if(strcmp(arg, cmd_table[i].name) == 0) {
				printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
				return 0;
			}
		}
		printf("Unknown command '%s'\n", arg);
	}
	return 0;
}

void ui_mainloop() {
	while(1) {
		char *str = rl_gets();
		char *str_end = str + strlen(str);

		/* extract the first token as the command */
		char *cmd = strtok(str, " ");
		if(cmd == NULL) { continue; }

		/* treat the remaining string as the arguments,
		 * which may need further parsing
		 */
		char *args = cmd + strlen(cmd) + 1;
		if(args >= str_end) {
			args = NULL;
		}

#ifdef HAS_DEVICE
		extern void sdl_clear_event_queue(void);
		sdl_clear_event_queue();
#endif

		int i;
		for(i = 0; i < NR_CMD; i ++) {
			if(strcmp(cmd, cmd_table[i].name) == 0) {
				if(cmd_table[i].handler(args) < 0) { return; }
				break;
			}
		}

		if(i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
	}
}
