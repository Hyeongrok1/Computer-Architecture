#include <stdio.h>
#include <stdlib.h>
#define START_INST_MEMORY 0x10000000

typedef struct _Node {
	int addr;
	int data;
	struct _Node *next;
} Node;

typedef struct {
	Node *head;
} LinkedList;

void list_init(LinkedList *plist) {
	plist->head = (Node *) malloc(sizeof(Node));
	plist->head->addr = 0;
	plist->head->data = 0;
	plist->head->next = NULL;
}

void list_insert(LinkedList *plist, int addr, int data) {
	Node *cur = plist->head;
	Node *newNode = (Node *)malloc(sizeof(Node));
	newNode->addr = addr;
	newNode->data = data;
	newNode->next = NULL;

	while (cur->next != NULL) cur = cur->next;

	cur->next = newNode;
}

void list_store(LinkedList *plist, int addr, int data) {
	Node *cur = plist->head;

	while (cur->next != NULL) {
		cur = cur->next;
		if (cur->addr == addr) {
			cur->data = data;
			return;
		}
	}

	list_insert(plist, addr, data);
}

void list_free(LinkedList *plist) {
	Node *cur = plist->head;
	Node *tmp = NULL;
	while (cur->next != NULL) {
		tmp = cur;
		cur = cur->next;
		free(tmp);
	}
	free(cur);
	free(plist);
}

int list_load(LinkedList *plist, int addr) {
	Node *cur = plist->head;
	while (cur->next != NULL) {
		cur = cur->next;
		if ((unsigned) cur->addr == (unsigned) addr) {
			return cur->data;
		}
	}
	
	return 0;	
}

int registers[32] = {0,};
int pc = 0; 

void r_format(unsigned int instruction, int opcode);
void i_format(unsigned int instruction, int opcode, LinkedList *plist);
void i_shift_format(unsigned int instruction, int funct3);
void sb_format(unsigned int instruction);
void u_format(unsigned int instruction, int opcode);
void j_format(unsigned int instruction);
void store_format(unsigned int instruction, LinkedList *plist);
void print_final_registers();

int main(int argc, char *argv[]) {
	
	LinkedList *plist = NULL;

	unsigned int buffer;
	int instruction_number = 0;
	int opcode;
	int funct3;
	int executed_instruction_num = 0;

	if (argc == 3) {
		FILE *fs_inst = fopen(argv[1], "rb");
		instruction_number = atoi(argv[2]);

		pc = 0;
		while (fread(&buffer, 4, 1, fs_inst) == 1) {
			opcode = (buffer << 25) >> 25;
			funct3 = (buffer << 17) >> 29;

			if (opcode == 51) {
				r_format(buffer, opcode);
			}
			else if (opcode == 3 || opcode == 103) {
				i_format(buffer, opcode, plist);
			}
			else if (opcode == 19) {
				if (funct3 == 1 || funct3 == 5) i_shift_format(buffer, funct3);
				else i_format(buffer, opcode, plist);
			}
			else if (opcode == 99) {
				sb_format(buffer);
			}
			else if (opcode == 55) {
				u_format(buffer, opcode);
			}
			else if (opcode == 23) {
				u_format(buffer, opcode);
			}
			else if (opcode == 111) {
				j_format(buffer);
			}
			else if (opcode == 35) {
				store_format(buffer, plist);		
			}
			fseek(fs_inst,  pc, SEEK_SET);
			executed_instruction_num++;
			if (executed_instruction_num == instruction_number) break;
		}
		fclose(fs_inst);
	}
	else if (argc == 4) {
		FILE *fs_inst = fopen(argv[1], "rb");
		FILE *fs_data = fopen(argv[2], "rb");
		instruction_number = atoi(argv[3]);
		
		pc = 0;
		
		plist = (LinkedList *) malloc(sizeof(LinkedList));
		list_init(plist);
		int data_memory = START_INST_MEMORY;
		while (fread(&buffer, 4, 1, fs_data) == 1) {
			if (buffer == 0xff) break;
			list_insert(plist, data_memory, buffer);		
			data_memory += 4;
		}	
		printf("0\n");
		while (fread(&buffer, 4, 1, fs_inst) == 1) {
			opcode = (buffer << 25) >> 25;
			funct3 = (buffer << 17) >> 29;

			if (opcode == 51) {
				r_format(buffer, opcode);
			}
			else if (opcode == 3 || opcode == 103) {
				i_format(buffer, opcode, plist);
			}
			else if (opcode == 19) {
				if (funct3 == 1 || funct3 == 5) i_shift_format(buffer, funct3);
				else i_format(buffer, opcode, plist);
			}
			else if (opcode == 99) {
				sb_format(buffer);
			}
			else if (opcode == 55) {
				u_format(buffer, opcode);
			}
			else if (opcode == 23) {
				u_format(buffer, opcode);
			}
			else if (opcode == 111) {
				j_format(buffer);
			}
			else if (opcode == 35) {
				store_format(buffer, plist);		
			}
			printf("%d\n", (pc)/4);
			fseek(fs_inst,  pc, SEEK_SET);
			executed_instruction_num++;
			if (executed_instruction_num == instruction_number) break;
		}
		fclose(fs_inst);
		fclose(fs_data);
					Node *cur = plist->head;
			while (cur->next != NULL) {
				cur = cur->next;
				printf("%.8x: %.8x\n", cur->addr, cur->data);
			}
			printf("\n");
		list_free(plist);
	}
	print_final_registers();
	return 0;
}

void r_format(unsigned int instruction, int opcode) {
	int rd;
	int funct3;
	int rs1;
	int rs2;
	int funct7;
	
	rd = (instruction << 20) >> 27;
	funct3 = (instruction << 17) >> 29;
	rs1 = (instruction << 12) >> 27;
	rs2 = (instruction << 7) >> 27;
	funct7 = instruction >> 25;
	
	if (funct7 == 0) {
		if (funct3 == 0) { // add
			registers[rd] = registers[rs1] + registers[rs2];
		}
		else if (funct3 == 1) {
			registers[rd] = registers[rs1] << (registers[rs2] & 0x1f); // sll
		}
		else if (funct3 == 2) { // slt
			if (registers[rs1] < registers[rs2]) registers[rd] = 1;
			else registers[rd] = 0;
		}
		else if (funct3 == 3) { // sltu
			if ((unsigned) registers[rs1] < (unsigned) registers[rs2]) registers[rd] = 1;
			else registers[rd] = 0;
		}
		else if (funct3 == 4) registers[rd] = registers[rs1] ^ registers[rs2];	// xor
		else if (funct3 == 5) registers[rd] = ((unsigned) registers[rs1]) >> (registers[rs2] & 0x1f); // srl
		else if (funct3 == 6) registers[rd] = registers[rs1] | registers[rs2];	// or
		else if (funct3 == 7) registers[rd] = registers[rs1] & registers[rs2];	//and
	}
	else if (funct7 == 32) {
		if (funct3 == 0) registers[rd] = registers[rs1] - registers[rs2]; 	// sub
		else if (funct3 == 5) registers[rd] = registers[rs1] >> registers[rs2];	// sra
	}

	pc += 4;
	// if rd is x0, reset the value to 0
	if (rd == 0) registers[rd] = 0;
}

void i_format(unsigned int instruction, int opcode, LinkedList *plist) {

	int rd;
	int funct3;
	int rs1;
	int immediate;
	
	char *instruction_name = NULL;
	
	rd = (instruction << 20) >> 27;
	funct3 = (instruction << 17) >> 29;
	rs1 = (instruction << 12) >> 27;
	immediate = (signed) instruction >> 20;
	if (opcode == 3) {
		int data = list_load(plist, registers[rs1] + immediate);
		if (funct3 == 0) { // lb
			data = (data << 24) >> 24;
		}
		else if (funct3 == 1) { // lh
			data = (data << 16) >> 16;
		}
		else if (funct3 == 4) { // lbu
			data = (unsigned) (data << 24) >> 24;
		}
		else if (funct3 == 5) { // lhu
			data = (unsigned) (data << 16) >> 16;
		}
		registers[rd] = data; // lw is not handled
		pc += 4;
	}
	else if (opcode == 19) {
		if (funct3 == 0) { // addi
			registers[rd] = registers[rs1] + immediate;
		}
		else if (funct3 == 2) { // slti
			if (registers[rs1] < immediate) registers[rd] = 1;
			else registers[rd] = 0;
		}
		else if (funct3 == 3) { // sltiu
			if ((unsigned) registers[rs1] < (unsigned) immediate) registers[rd] = 1;
			else registers[rd] = 0; 	
		}
		else if (funct3 == 4) { // xori
			registers[rd] = registers[rs1] ^ immediate;
		}
		else if (funct3 == 6) { // ori
			registers[rd] = registers[rs1] | immediate;
		}
		else if (funct3 == 7) { // andi
			registers[rd] = registers[rs1] & immediate;
		}
		pc += 4;
	}
	else if (opcode == 103) { // jalr
		printf("%.8x %.8x %d\n", pc, registers[rs1], rs1);
		registers[rd] = pc + 4;
		pc = registers[rs1] + immediate;
	}	

	// if rd is x0, reset the value to 0
	if (rd == 0) registers[rd] = 0;
}

void i_shift_format(unsigned int instruction, int funct3) {
	
	int rd;
	int rs1;
	int shamt;
	int funct7;
	
	rd = (instruction << 20) >> 27;
	rs1 = (instruction << 12) >> 27;
	shamt = (instruction << 7) >> 27;
	funct7 = instruction >> 25;

	if (funct3 == 1) registers[rd] = registers[rs1] << shamt;	// slli
	else if (funct3 == 5 && funct7 == 0) registers[rd] = ((unsigned) registers[rs1]) >> shamt; // srli
	else if (funct3 == 5 && funct7 == 32) registers[rd] = registers[rs1] >> shamt; // srai

	pc += 4;
	// if rd is x0, reset the value to 0
	if (rd == 0) registers[rd] = 0;
}

void sb_format(unsigned int instruction) {

	int funct3;
	int rs1;
	int rs2;
	int immediate;

	funct3 = (instruction << 17) >> 29;
	rs1 = (instruction << 12) >> 27;
	rs2 = (instruction << 7) >> 27;

	int imm12 = (instruction >> 31) << 12;
	int imm11 = ((instruction << 24) >> 31) << 11;
	int imm10_5 = ((instruction << 1) >> 26) << 5;
	int imm4_1 = ((instruction << 20) >> 28) << 1;
	immediate = (imm12 | imm11 | imm10_5 | imm4_1);
	immediate = (signed) (immediate << 19) >> 19;

	if (funct3 == 0) { // beq
		if (registers[rs1] == registers[rs2]) pc += immediate;
		else pc += 4;
	}
	else if (funct3 == 1) { // bne
		if (registers[rs1] != registers[rs2]) pc += immediate;
		else pc += 4;
	}
	else if (funct3 == 4) { // blt
		if (registers[rs1] < registers[rs2]) pc += immediate;
		else pc += 4;
	}
	else if (funct3 == 5) { // bge
		if (registers[rs1] >= registers[rs2]) pc += immediate;
		else pc += 4;
	}
	else if (funct3 == 6) { // bltu
		if ((unsigned) registers[rs1] < (unsigned) registers[rs2]) pc += immediate;
		else pc += 4;
	}
	else if (funct3 == 7) { // bgeu
		if ((unsigned) registers[rs1] >= (unsigned) registers[rs2]) pc += immediate;
		else pc += 4;
	}
}

void u_format(unsigned int instruction, int opcode) {

    int rd = (instruction << 20) >> 27;
    int immediate = (instruction >> 12) << 12;
    if (opcode == 55)  { // lui
		registers[rd] = immediate;
    }
    else if (opcode == 23) { // auipc
		registers[rd] = pc + immediate;
	}
	pc += 4;
	// if rd is x0, reset the value to 0
	if (rd == 0) registers[rd] = 0;
}

void j_format(unsigned int instruction) {
	
	int rd = (instruction << 20) >> 27;
	int immediate;

	int imm20 = (instruction >> 31) << 20;
	int imm10_1 = ((instruction << 1) >> 22) << 1;
	int imm11 = ((instruction << 11) >> 31) << 11;
	int imm19_12 = ((instruction << 12) >> 24) << 12;	
	immediate = imm20 | imm19_12 | imm11 | imm10_1;
	immediate = (immediate << 11) >> 11;

	// jal
	registers[rd] = pc + 4;
	pc += immediate;

	//if rd is x0, reset the value to 0
	if (rd == 0) registers[rd] = 0;
}

void store_format(unsigned int instruction, LinkedList *plist) {

	int funct3;
	int rs1;
	int rs2;
	int immediate;

	funct3 = (instruction << 17) >> 29;
	rs1 = (instruction << 12) >> 27;
	rs2 = (instruction << 7) >> 27;

	int imm11_5 = (instruction >> 25) << 5;
	int imm4_0 = (instruction << 20) >> 27;
	immediate = (imm11_5 | imm4_0);
	immediate = (immediate << 20) >> 20;

	if (funct3 == 0) { // sb
		int data = registers[rs2] & 0x000000ff;
		list_store(plist, registers[rs1] + immediate, data);
	}
	else if (funct3 == 1) { // sh
		int data = registers[rs2] & 0x0000ffff;
		list_store(plist, registers[rs1] + immediate, data);
	}
	else if (funct3 == 2) { // sw
		// printf("%.8x\n%.8x\n", registers[rs1], immediate);
		list_store(plist, registers[rs1] + immediate, registers[rs2]);
	}

	pc += 4;
}

void print_final_registers() {
	for (int i = 0; i < 32; i++) {
		printf("x%d: 0x%.8x\n", i, registers[i]); 
	}
}
