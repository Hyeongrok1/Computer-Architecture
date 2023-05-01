#include <stdio.h>
#include <stdlib.h>

int registers[32];
int current_inst_memory = 0x10000000;

void r_format(unsigned int instruction, int opcode);
void i_format(unsigned int instruction, int opcode);
void i_shift_format(unsigned int instruction, int funct3);
void sb_format(unsigned int instruction);
void u_format(signed int instruction, int opcode);
void j_format(unsigned int instruction);
void store_format(unsigned int instruction);
void print_final_registers();

int main(int argc, char *argv[]) {
	
	FILE *fs_inst = NULL;
	FILE *fs_data = NULL;	

	unsigned int buffer;
	int instruction_number = 0;
	int opcode;
	int funct3;
	int executed_instruction_num = 0;

	if (argc == 3) {
		FILE *fs_inst = fopen(argv[1], "rb");
		instruction_number = atoi(argv[2]);

		while (fread(&buffer, 4, 1, fs_inst) == 1) {
			opcode = (buffer << 25) >> 25;
			funct3 = (buffer << 17) >> 29;

			if (opcode == 51) {
				r_format(buffer, opcode);
			}
			else if (opcode == 3 || opcode == 103) {
				i_format(buffer, opcode);
			}
			else if (opcode == 19) {
				if (funct3 == 1 || funct3 == 5) i_shift_format(buffer, funct3);
				else i_format(buffer, opcode);
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
				store_format(buffer);		
			}
			else {
				printf("unknown instruction\n");
			}
			executed_instruction_num++;
			if (executed_instruction_num == instruction_number) break;
		}
		fclose(fs_inst);
	}
	else if (argc == 4) {
		FILE *fs_inst = fopen(argv[1], "rb");
		FILE *fs_data = fopen(argv[2], "rb");
		instruction_number = atoi(argv[3]);

		while (fread(&buffer, 4, 1, fs_inst) == 1) {
			current_inst_memory += 4;
		}
		fclose(fs_inst);
		
		current_inst_memory = 0x10000000;
		/*
		 * write code about data file
		 *
		 * */

		fs_inst = fopen(argv[1], "rb");
		while (fread(&buffer, 4, 1, fs_inst) == 1) {
			opcode = (buffer << 25) >> 25;
			funct3 = (buffer << 17) >> 29;

			if (opcode == 51) {
				r_format(buffer, opcode);
			}
			else if (opcode == 3 || opcode == 103) {
				i_format(buffer, opcode);
			}
			else if (opcode == 19) {
				if (funct3 == 1 || funct3 == 5) i_shift_format(buffer, funct3);
				else i_format(buffer, opcode);
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
				store_format(buffer);		
			}
			else {
				printf("unknown instruction\n");
			}
			executed_instruction_num++;
			if (executed_instruction_num == instruction_number) break;
		}
		fclose(fs_inst);
		fclose(fs_data);
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
	
	char *instruction_name = NULL;
	
	rd = (instruction << 20) >> 27;
	funct3 = (instruction << 17) >> 29;
	rs1 = (instruction << 12) >> 27;
	rs2 = (instruction << 7) >> 27;
	funct7 = instruction >> 25;
	
	if (opcode == 19) {
		if (funct3 == 1) registers[rd] = registers[rs1] << registers[rs2];	// slli
	        else if (funct3 == 5 && funct7 == 0) registers[rd] = (unsigned) registers[rs1] >> registers[rs2]; // srli
		else if (funct3 == 5 && funct7 == 32) registers[rd] = registers[rs1] >> registers[rs2]; // srai
	}
	else if (funct7 == 0) {
		if (funct3 == 0) { // add
			registers[rd] = registers[rs1] + registers[rs2];
		}
		else if (funct3 == 1) registers[rd] = registers[rs1] << registers[rs2]; // sll
		else if (funct3 == 2) { // slt
			if (registers[rs1] < registers[rs2]) registers[rd] = 1;
			else registers[rd] = 0;
		}
		else if (funct3 == 3) { // sltu
			if ((unsigned) registers[rs1] < (unsigned) registers[rs2]) registers[rd] = 1;
			else registers[rd] = 0;
		}
		else if (funct3 == 4) registers[rd] = registers[rs1] ^ registers[rs2];	// xor
		else if (funct3 == 5) registers[rd] = (unsigned) registers[rs1] >> registers[rs2]; // srl
		else if (funct3 == 6) registers[rd] = registers[rs1] | registers[rs2];	// or
		else if (funct3 == 7) registers[rd] = registers[rs1] & registers[rs2];	//and
	}
	else if (funct7 == 32) {
		if (funct3 == 0) registers[rd] = registers[rs1] - registers[rs2]; 	// sub
		else if (funct3 == 5) registers[rd] = registers[rs1] >> registers[rs2];	// sra
	}

	if (instruction_name == NULL) printf("unknown instruction\n");
	
	// if rd is x0, reset the value to 0
	if (rd == 0) registers[rd] = 0;
}

void i_format(unsigned int instruction, int opcode) {

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
		if (funct3 == 0) instruction_name = "lb";
		else if (funct3 == 1) instruction_name = "lh";
		else if (funct3 == 2) instruction_name = "lw";
		else if (funct3 == 4) instruction_name = "lbu";
		else if (funct3 == 5) instruction_name = "lhu";
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
			if (registers[rs1] < (unsigned) immediate) registers[rd] = 1;
			else registers[rd] = 0; 	//// check again
		}
		else if (funct3 == 4) { // xori
			registers[rd] = registers[rs1] ^ immediate;
		}
		else if (funct3 == 6) { // ori
			registers[rd] = registers[rs1] | immediate;
		}
		else if (funct3 == 7) { // andi;
			registers[rd] = registers[rs1] & immediate;
		}
	}
	else if (opcode == 103) {
		instruction_name = "jalr";
	}	

	// if rd is x0, reset the value to 0
	if (rd == 0) registers[rd] = 0;
}

void i_shift_format(unsigned int instruction, int funct3) {
	
	int rd;
	int rs1;
	int shamt;
	int funct7;
	
	char *instruction_name = NULL;
	
	rd = (instruction << 20) >> 27;
	rs1 = (instruction << 12) >> 27;
	shamt = (instruction << 7) >> 27;
	funct7 = instruction >> 25;

	if (funct3 == 1) registers[rd] = registers[rs1] << shamt;	// slli
	else if (funct3 == 5 && funct7 == 0) registers[rd] = (unsigned) registers[rs1] >> shamt; // srli
	else if (funct3 == 5 && funct7 == 32) registers[rd] = registers[rs1] >> shamt; // srai

	if (instruction_name == NULL) printf("unknown instruction\n");

	// if rd is x0, reset the value to 0
	if (rd == 0) registers[rd] = 0;
}

void sb_format(unsigned int instruction) {

	int funct3;
	int rs1;
	int rs2;
	int immediate;
	
	char *instruction_name = NULL;

	funct3 = (instruction << 17) >> 29;
	rs1 = (instruction << 12) >> 27;
	rs2 = (instruction << 7) >> 27;

	int imm12 = (instruction >> 31) << 12;
	int imm11 = ((instruction << 24) >> 31) << 11;
	int imm10_5 = ((instruction << 1) >> 26) << 5;
	int imm4_1 = ((instruction << 20) >> 28) << 1;
	immediate = (imm12 | imm11 | imm10_5 | imm4_1);
	immediate = (immediate << 19) >> 19;

	if (funct3 == 0) instruction_name = "beq";
	else if (funct3 == 1) instruction_name = "bne";
	else if (funct3 == 4) instruction_name = "blt";
	else if (funct3 == 5) instruction_name = "bge";
	else if (funct3 == 6) instruction_name = "bltu";
	else if (funct3 == 7) instruction_name = "bgeu";

	if (instruction_name == NULL) {
		printf("unknown instruction\n");
	}
}

void u_format(signed int instruction, int opcode) {

        int rd = (instruction << 20) >> 27;
    	int immediate = (instruction >> 12) << 12;

    	char *instruction_name = NULL;

        if (opcode == 55) instruction_name = "lui";
        else if (opcode == 23) instruction_name = "auipc";

        if (instruction_name == NULL) printf("unknown instruction\n");

	// if rd is x0, reset the value to 0
	if (rd == 0) registers[rd] = 0;
}

void j_format(unsigned int instruction) {
	
	int rd = (instruction << 20) >> 27;
	int immediate;

	char *instruction_name = NULL;

	int imm20 = (instruction >> 31) << 20;
	int imm10_1 = ((instruction << 1) >> 22) << 1;
	int imm11 = ((instruction << 11) >> 31) << 11;
	int imm19_12 = ((instruction << 12) >> 24) << 12;	
	immediate = imm20 | imm19_12 | imm11 | imm10_1;
	immediate = (immediate << 11) >> 11;

	instruction_name = "jal";

	//if rd is x0, reset the value to 0
	if (rd == 0) registers[rd] = 0;
}

void store_format(unsigned int instruction) {

	int funct3;
	int rs1;
	int rs2;
	int immediate;
	
	char *instruction_name = NULL;

	funct3 = (instruction << 17) >> 29;
	rs1 = (instruction << 12) >> 27;
	rs2 = (instruction << 7) >> 27;

	int imm11_5 = (instruction >> 25) << 5;
	int imm4_0 = (instruction << 20) >> 27;
	immediate = (imm11_5 | imm4_0);
	immediate = (immediate << 20) >> 20;

	if (funct3 == 0) instruction_name = "sb";
	else if (funct3 == 1) instruction_name = "sh";
	else if (funct3 == 2) instruction_name = "sw";

	if (instruction_name == NULL) {
		printf("unknown instruction\n");
	}
}

void print_final_registers() {
	for (int i = 0; i < 32; i++) {
		printf("x%d: 0x%.8x\n", i, registers[i]); 
	}
}
