#include <stdio.h>
#include <stdlib.h>

enum registers { x0 = 0, x1, x2, x3, x4, x5, x6, x7, x8, x9, x10, x11, x12, x13, x14, x15, x16, x17, x18, x19, x20, x21, x22, x23, x24, x25, x26, x27, x28, x29, x30, x31 };

void register_print(int register_num);
void r_format(unsigned int instruction, int opcode);
void i_format(unsigned int instruction, int opcode);
void sb_format(unsigned int instruction);
void u_format(unsigned int instruction, int opcode);
void j_format(unsigned int instruction);
void store_format(unsigned int instruction);

int main(int argc, char *argv[]) {
	
	FILE *fs = fopen(argv[1], "rb");
	
	int buffer;
	int instruction_number = 0;
	unsigned int opcode;
	
	while (fread(&buffer, 4, 1, fs) == 1) {
		opcode = (buffer << 25) >> 25;
		
		printf("inst %d: %.8x ", instruction_number++, buffer);
		if (opcode == 51) {
			r_format(buffer, opcode);
		}
		else if (opcode == 3 || opcode == 19 || opcode == 103) {
			i_format(buffer, opcode);
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
	}

	fclose(fs);
	return 0;
}

void register_print(int register_num) {
	if (register_num == x0) printf("x0");
	else if (register_num == x1) printf("x1");
	else if (register_num == x2) printf("x2");
	else if (register_num == x3) printf("x3");
	else if (register_num == x4) printf("x4");
	else if (register_num == x5) printf("x5");
	else if (register_num == x6) printf("x6");
	else if (register_num == x7) printf("x7");
	else if (register_num == x8) printf("x8");
	else if (register_num == x9) printf("x9");
	else if (register_num == x10) printf("x10");
	else if (register_num == x11) printf("x11");
	else if (register_num == x12) printf("x12");
	else if (register_num == x13) printf("x13");
	else if (register_num == x14) printf("x14");
	else if (register_num == x15) printf("x15");
	else if (register_num == x16) printf("x16");
	else if (register_num == x17) printf("x17");
	else if (register_num == x18) printf("x18");
	else if (register_num == x19) printf("x19");
	else if (register_num == x20) printf("x20");
	else if (register_num == x21) printf("x21");
	else if (register_num == x22) printf("x22");
	else if (register_num == x23) printf("x23");
	else if (register_num == x24) printf("x24");
	else if (register_num == x25) printf("x25");
	else if (register_num == x26) printf("x26");
	else if (register_num == x27) printf("x27");
	else if (register_num == x28) printf("x28");
	else if (register_num == x29) printf("x29");
	else if (register_num == x30) printf("x30");
	else if (register_num == x31) printf("x31");
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
		int shamt = rs2;

		if (funct3 == 1) instruction_name = "slli";
	        else if (funct3 == 5 && funct7 == 0) instruction_name = "srli";
		else if (funct3 == 5 && funct7 == 32) instruction_name = "srai";
	}
	else if (funct7 == 0) {
		if (funct3 == 0) instruction_name = "add";
		else if (funct3 == 1) instruction_name = "sll";
		else if (funct3 == 2) instruction_name = "slt";
		else if (funct3 == 3) instruction_name = "sltu";
		else if (funct3 == 4) instruction_name = "xor";
		else if (funct3 == 5) instruction_name = "srl";
		else if (funct3 == 6) instruction_name = "or";
		else if (funct3 == 7) instruction_name = "and";
	}
	else if (funct7 == 32) {
		if (funct3 == 0) instruction_name = "sub";
		else if (funct3 == 5) instruction_name = "sra";
	}
	if (instruction_name == NULL) printf("unknown instruction");
	
	printf("%s ", instruction_name);
	register_print(rd);
	printf(", ");
	register_print(rs1);
	printf(", ");
	register_print(rs2);
	printf("\n");
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
		if (funct3 == 0) instruction_name = "addi";
		else if (funct3 == 2) instruction_name = "slti";
		else if (funct3 == 3) instruction_name = "sltiu";
		else if (funct3 == 4) instruction_name = "xori";
		else if (funct3 == 6) instruction_name = "ori";
		else if (funct3 == 7) instruction_name = "andi";
	}
	else if (opcode == 103) {
		instruction_name = "jalr";
	}	

	if (instruction_name == NULL) {
		printf("unknown instruction");
	}

	if (opcode == 3) {
		printf("%s ", instruction_name);
		register_print(rd);
		printf(", %d(", immediate);
		register_print(rs1);
		printf(")\n");
		return;
	}

	printf("%s ", instruction_name);
	register_print(rd);
	printf(", ");
	register_print(rs1);
	printf(", %d\n", immediate);
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
	int imm4_1 = ((instruction << 13) >> 28) << 1;
	immediate = imm12 | imm11 | imm10_5 | imm4_1;


	if (funct3 == 0) instruction_name = "beq";
	else if (funct3 == 1) instruction_name = "bne";
	else if (funct3 == 4) instruction_name = "blt";
	else if (funct3 == 5) instruction_name = "bge";
	else if (funct3 == 6) instruction_name = "bltu";
	else if (funct3 == 7) instruction_name = "bgeu";

	if (instruction_name == NULL) {
		printf("unknown instruction");
	}
	
	printf("%s ", instruction_name);
	register_print(rs1);
	printf(", ");
	register_print(rs2);
	printf(", %d\n", immediate);
}

void u_format(unsigned int instruction, int opcode) {

        int rd = (instruction << 20) >> 27;
    	int immediate = instruction >> 12;

    	char *instruction_name = NULL;

        if (opcode == 55) instruction_name = "lui";
        else if (opcode == 23) instruction_name = "auipc";

        if (instruction_name == NULL) printf("unknown instruction");
	
	printf("%s ", instruction_name);
	register_print(rd);
	printf(", %d\n", immediate);
}

void j_format(unsigned int instruction) {
	
	int rd = (instruction << 20) >> 27;
	int immediate;

	char *instruction_name = NULL;

	int imm20 = instruction >> 11;
	int imm10_1 = (instruction << 1) >> 21;
	int imm11 = (instruction << 11) >> 20;
	int imm19_12 = (instruction << 12) >> 12;	
	immediate = imm20 | imm19_12 | imm11 | imm10_1;

	instruction_name = "jal";
	
	printf("%s ", instruction_name);
	register_print(rd);
	printf(", %d\n", immediate);

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

	int imm11_5 = instruction >> 20;
	int imm4_0 = instruction << 13;
	immediate = (signed) (imm11_5 | (1 & imm4_0)) >> 20;

	if (funct3 == 0) instruction_name = "sb";
	else if (funct3 == 1) instruction_name = "sh";
	else if (funct3 == 2) instruction_name = "sw";

	if (instruction_name == NULL) {
		printf("unknown instruction");
	}
	
	printf("%s ", instruction_name);
	register_print(rs2);
	printf(", %d(", immediate);
	register_print(rs1);
	printf(")\n");
}
