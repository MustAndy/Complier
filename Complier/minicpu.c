#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define MEMSIZE (65536 >> 2)
#define REGSIZE 32
#define BUFSIZE 256

#define MEM(addr) (Mem[addr >> 2])
#define PC (Reg[REGSIZE + 0])
#define Status (Reg[REGSIZE + 1])
#define LO (Reg[REGSIZE + 2])
#define HI (Reg[REGSIZE + 3])

#define PCINIT 0x4000
#define SPINIT 0xFFFC
#define GPINIT 0xC000
#define NREG(name) (*Nreg(name))


static unsigned Mem[MEMSIZE];
static unsigned Reg[REGSIZE + 4];
const char RedirNull[] = "";
const char RedirPrefix[] = ">";
static char Buf[BUFSIZE];
static int Halt = 0;
static FILE *FP;
static char *Redir = (char *)RedirNull;
unsigned instruction;
unsigned op,	// instruction [31-26]
r1,	// instruction [25-21]
r2,	// instruction [20-16]
r3,	// instruction [15-11]
funct,	// instruction [5-0]
offset,	// instruction [15-0]
jsec;	// instruction [25-0]

		// control signals
unsigned data1, data2;
unsigned extended_value;
unsigned ALUresult;
char Zero;
unsigned memdata;

const char RegName[REGSIZE + 4][6] = {
	"$zero", "$at", "$v0", "$v1", "$a0", "$a1", "$a2", "$a3",
	"$t0", "$t1", "$t2", "$t3", "$t4", "$t5", "$t6", "$t7",
	"$s0", "$s1", "$s2", "$s3", "$s4", "$s5", "$s6", "$s7",
	"$t8", "$t9", "$k0", "$k1", "$gp", "$sp", "$fp", "$ra",
	"$pc", "$stat", "$lo", "$hi" };

typedef struct
{
	char RegDst;
	char Jump;
	char Branch;//PCsrc
	char MemRead;
	char MemtoReg;
	char ALUOp;//ALUctr
	char MemWrite;
	char ALUSrc;
	char RegWrite;
}struct_controls;

struct_controls controls;

#pragma region Define of Component

/* ALU */
void ALU(unsigned A, unsigned B, char ALUControl, unsigned *ALUresult, char *Zero);

/* fetch instruction from memory */
int instruction_fetch(unsigned *Mem, unsigned *instruction);

/* instruction partition */
void instruction_partition(unsigned instruction, unsigned *op, unsigned *r1, unsigned *r2, unsigned *r3, unsigned *funct, unsigned *offset, unsigned *jsec);

/* instruction decode */
int instruction_decode(unsigned op, struct_controls *controls);

/* read_register */
void read_register(unsigned r1, unsigned r2, unsigned *Reg, unsigned *data1, unsigned *data2);

/* sign_extend */
void sign_extend(unsigned offset, unsigned *extended_value);

/* ALU */
int ALU_operations(unsigned data1, unsigned data2, unsigned extended_value, unsigned funct, char ALUOp, char ALUSrc, unsigned *ALUresult, char *Zero);

/* read/write memory */
int rw_memory(unsigned ALUresult, unsigned data2, char MemWrite, char MemRead, unsigned *memdata, unsigned *Mem);

/* write to register */
void write_register(unsigned r2, unsigned r3, unsigned memdata, unsigned ALUresult, char RegWrite, char RegDst, char MemtoReg, unsigned *Reg);

/* PC update */
void PC_update(unsigned jsec, unsigned extended_value, char Branch, char Jump, char Zero);
/* ALU */
#pragma endregion


#pragma region Component.c
/*In 'ALU' function, to judge which op I need to do
  according to the ALUControl.
*/
void ALU(unsigned A, unsigned B, char ALUControl, unsigned *ALUresult, char *Zero)
{
	int ALUControlInt = (int)ALUControl;
	switch (ALUControlInt) {
	case 0: //add
		*ALUresult = A + B;
		break;
	case 1: //sub
		*ALUresult = A - B;
		break;
	case 2: //slt
		if (A < B) {
			*ALUresult = 1;
		}
		else {
			*ALUresult = 0;
		}
		break;
	case 3: //sltu
		if (A < B) {
			*ALUresult = 1;
		}
		else {
			*ALUresult = 0;
		}
		break;
	case 4: //and
		*ALUresult = A & B;
		break;
	case 5: //or
		*ALUresult = A | B;
		break;
	case 6: //shl not setting alu
		B << 16;
		break;
	case 7: //bitwise not, set all to complement in A
		*ALUresult = ~A;
		break;
	}
	//set zero to 1 if alu is 0
	if (*ALUresult == 0) {
		*Zero = 1;
	}
	else {
		*Zero = 0;
	}
}

/* In 'instruction_fetch', I make an initialization to
   PC, because i don't know why it will be initialized to
   PCINIT. Then, i use (PC / 4) + 4096 to get next instruction
   from mem.
*/
int instruction_fetch(unsigned *Mem, unsigned *instruction)
{
	if (PC == PCINIT)
	{
		PC = 0;
	}
	if (PC % 4 != 0) {
		return 1; 
	}
	*instruction = Mem[(PC / 4) + 4096]; //find instruction in mem by PC offset by memory size

	return 0;
}


/* instruction partition */
void instruction_partition(unsigned instruction, unsigned *op, unsigned *r1, unsigned *r2, unsigned *r3, unsigned *funct, unsigned *offset, unsigned *jsec)
{

	*op = (instruction >> 26) & 0x0000003f;	
	//op; 6 bits; 26-31

	*r1 = (instruction >> 21) & 0x0000001f;	
	//r1; 5 bits;  21-25

	*r2 = (instruction >> 16) & 0x0000001f;
	//r2 = 16-20

	*r3 = (instruction >> 11) & 0x0000001f;
	//r3 = 11 - 15

	*funct = instruction & 0x0000003f;
	// funct; 6 bits back from 0; 0-5

	*offset = instruction & 0x0000ffff;
	//offset; 16 bits back from 0; 0-15

	*jsec = instruction & 0x03ffffff;
	//jsec; 26 bits back from 0; 0-25
}

/* In 'instruction decode' function. I change the 
   control single through the op code.
*/
int instruction_decode(unsigned op, struct_controls *controls)
{
	// switch by op to set the struct
	switch (op) {
	case 0:
		//r-type
		controls->RegWrite = 1;
		controls->MemWrite = 0;
		controls->MemtoReg = 0;
		controls->Branch = 2;
		controls->MemRead = 0;
		controls->RegDst = 1;
		controls->ALUSrc = 0;
		controls->ALUOp = 7;
		controls->Jump = 0;
		return 0;
	case 2:
		// j-type
		controls->RegWrite = 0;
		controls->Jump = 1;
		controls->ALUOp = 0;
		controls->Branch = 0;
		controls->MemRead = 0;
		controls->MemtoReg = 2;
		controls->ALUSrc = 0;
		controls->MemWrite = 0;
		controls->RegDst = 2;
		return 0;
		
	case 4: //beq
		controls->RegDst = 2;
		controls->Jump = 0;
		controls->Branch = 1;
		controls->RegWrite = 0;
		controls->MemRead = 0;
		controls->MemtoReg = 2;
		controls->MemWrite = 0;
		controls->ALUSrc = 0;
		controls->ALUOp = 1;
		break;
	case 8: //adi
		controls->RegDst = 0;
		controls->Jump = 0;
		controls->ALUSrc = 1;
		controls->Branch = 0;
		controls->MemRead = 0;
		controls->RegWrite = 1;
		controls->MemtoReg = 0;
		controls->ALUOp = 0;
		controls->MemWrite = 0;
		break;
	case 9: //adiu

		controls->MemWrite = 0;
		controls->ALUSrc = 1;
		controls->Branch = 0;
		controls->RegDst = 0;
		controls->Jump = 0;
		controls->MemtoReg = 0;
		controls->ALUOp = 2;
		controls->MemRead = 0;
		controls->RegWrite = 1;
		break;
	case 10: //slti
		controls->RegDst = 0;
		controls->Jump = 0;
		controls->MemtoReg = 0;
		controls->ALUOp = 2;
		controls->MemWrite = 0;
		controls->ALUSrc = 1;
		controls->RegWrite = 1;
		controls->Branch = 0;
		controls->MemRead = 0;
		break;
	case 11: //sltiu
		controls->RegDst = 0;
		controls->MemRead = 0;
		controls->MemtoReg = 0;
		controls->ALUOp = 3;
		controls->MemWrite = 0;
		controls->Jump = 0;
		controls->Branch = 0;
		controls->ALUSrc = 1;
		controls->RegWrite = 1;
		break;
	case 15: //lui
		controls->RegDst = 0;
		controls->Jump = 0;
		controls->MemRead = 0;
		controls->MemtoReg = 0;
		controls->ALUOp = 6;
		controls->MemWrite = 0;
		controls->ALUSrc = 1;
		controls->Branch = 0;
		controls->RegWrite = 1;
		break;
	case 35: //lw
		controls->RegDst = 0;
		controls->Jump = 0;
		controls->MemRead = 1;
		controls->MemtoReg = 1;
		controls->ALUOp = 0;
		controls->MemWrite = 0;
		controls->ALUSrc = 1;
		controls->Branch = 0;
		controls->RegWrite = 1;
		break;
	case 43: //sw
		controls->RegDst = 2;
		controls->Jump = 0;
		controls->ALUSrc = 1;
		controls->Branch = 0;
		controls->MemRead = 0;
		controls->MemtoReg = 2;
		controls->ALUOp = 0;
		controls->MemWrite = 1;
		controls->RegWrite = 0;
		break;
	default:
		return 1; //returning false, no op found
	}
	return 0; //returning true
}

/* Just for read register */
void read_register(unsigned r1, unsigned r2, unsigned *Reg, unsigned *data1, unsigned *data2)
{
	
	*data1 = Reg[r1];
	*data2 = Reg[r2];
}


/* In 'Sign Extend' function. We get the first bit
   of offset to judge if negative or positive.
   If positive,keep 16bits, else extend to 32 bits.
*/
void sign_extend(unsigned offset, unsigned *extended_value)
{
	unsigned sign = offset >> 15;
	if (sign == 0)
	{
		*extended_value = offset & 0x0000ffff;
		/* 1111 1111 1111 1111 */
	}
	else
	{
		*extended_value = offset | 0xFFFF0000;
		/*1111 1111 1111 1111 0000 0000 0000 0000 */
	}
}

/* ALU operations */
int ALU_operations(unsigned data1, unsigned data2, unsigned extended_value, unsigned funct, char ALUOp, char ALUSrc, unsigned *ALUresult, char *Zero)
{
	// Call the function ALU if the op is between 0 and 6
	if (ALUOp >= 0 && ALUOp < 7) {
		if (ALUSrc == 1) {
			data2 = extended_value;
		}
		ALU(data1, data2, ALUOp, ALUresult, Zero);
		return 0;
	}
	if (ALUOp == 7)//R-type
	{
		unsigned ALUcontinue;
		switch (funct) {
		case 6:
			ALUcontinue = 6;
			break;
		case 32:
			ALUcontinue = 0;
			break;
		case 34:
			ALUcontinue = 1;
			break;
		case 36:
			ALUcontinue = 4;
			break;
		case 37:
			ALUcontinue = 5;
			break;
		case 39:
			ALUcontinue = 7;
			break;
		case 42:
			ALUcontinue = 2;
			break;
		case 43:
			ALUcontinue = 3;
			break;
		default:
			return 1; // return false if no function found
		}
		if (ALUSrc == 1) {
			data2 = extended_value;
		}
		ALU(data1, data2, ALUcontinue, ALUresult, Zero);
		return 0; //return true
	}
	else {
		return 1;
	}
}

/* In 'Read / Write Memory' function, I remove the last
   two bits of ALUresult. Then judge the mem address if is vaild.
   Then to read or write.
*/
int rw_memory(unsigned ALUresult, unsigned data2, char MemWrite, char MemRead, unsigned *memdata, unsigned *Mem)
{
	
	unsigned addressToUse = ALUresult >> 2;
	if (MemRead == 1)
	{if (ALUresult % 4 != 0) {
		printf("rw-memory");
		return 1; //false
	}
		*memdata = Mem[addressToUse];
	}
	if (MemWrite == 1)
	{if (ALUresult % 4 != 0) {
		printf("rw-memory");
		return 1; //false
	}
		Mem[addressToUse] = data2;
	}
	return 0;
}


/* In 'Write Register' function. We use 'RegWrite' and 'MemToReg'
   to judge what data I should put in the Reg. Then, wu use 'RegDst' to
   decide put in which Reg.
*/
void write_register(unsigned r2, unsigned r3, unsigned memdata, unsigned ALUresult, char RegWrite, char RegDst, char MemtoReg, unsigned *Reg)
{

	if (RegWrite == 1 && MemtoReg == 1) {
		if (RegDst == 0) {
			Reg[r2] = memdata;
		}
		else if (RegDst == 1) {
			Reg[r3] = memdata;
		}
	}
	if (RegWrite == 1 && MemtoReg == 0) {
		if (RegDst == 0) {
			Reg[r2] = ALUresult;
		}
		else if (RegDst == 1) {
			Reg[r3] = ALUresult;
		}
	}
}

/* In 'PC update' function, first let PC add 4 for next
   instruction. Then, decide if there have the jump or branch
   instruction.
*/
void PC_update(unsigned jsec, unsigned extended_value, char Branch, char Jump, char Zero)
{
	PC += 4;
	if (Branch == 1)
	{
		if (Zero == 1) {
			PC = (extended_value << 2);
		}
	}
	if (Jump == 1)
	{
		PC = (jsec << 2);
	}
}
#pragma endregion


unsigned *Nreg(char *name)
{
	int i;

	for (i = 0; i < REGSIZE + 4; i++)
	{
		if (strcmp(name, RegName[i]) == 0)
			return &Reg[i];
		if (strcmp(name, RegName[i] + 1) == 0)
			return &Reg[i];
	}
	return NULL;
}

void Init(void)
{
	memset(Reg, 0, (REGSIZE + 4) * sizeof(unsigned));
	NREG("pc") = PCINIT;
	NREG("sp") = SPINIT;
	NREG("gp") = GPINIT;
}

void DisplayControlSignals(void)
{
	fprintf(stdout, "\tControl Signals: %0x%0x%0x%0x%03x%0x%0x%0x%0x\n",
		controls.RegDst,
		controls.Jump,
		controls.Branch,
		controls.MemRead,
		controls.MemtoReg,
		controls.ALUOp,
		controls.MemWrite,
		controls.ALUSrc,
		controls.RegWrite);
}

void Step(void)
{
	/* fetch instruction from memory */
	Halt = instruction_fetch(Mem, &instruction);

	if (!Halt)
	{
		/* partition the instruction */
		instruction_partition(instruction, &op, &r1, &r2, &r3, &funct, &offset, &jsec);

		/* instruction decode */
		Halt = instruction_decode(op, &controls);
	}

	if (!Halt)
	{
		/* read_register */
		read_register(r1, r2, Reg, &data1, &data2);

		/* sign_extend */
		sign_extend(offset, &extended_value);

		/* ALU */
		Halt = ALU_operations(data1, data2, extended_value, funct, controls.ALUOp, controls.ALUSrc, &ALUresult, &Zero);
	}

	if (!Halt)
	{
		/* read/write memory */
		Halt = rw_memory(ALUresult, data2, controls.MemWrite, controls.MemRead, &memdata, Mem);
	}

	if (!Halt)
	{
		/* write to register */
		write_register(r2, r3, memdata, ALUresult, controls.RegWrite, controls.RegDst, controls.MemtoReg, Reg);

		/* PC update */
		PC_update(jsec, extended_value, controls.Branch, controls.Jump, Zero);
	}
}

void DumpReg(void)
{
	int i;
	char bb[] = "     ";

	for (i = 0; i < REGSIZE + 4; i++)
	{
		fprintf(stdout, "%s %s%s %08x%s",
			(i % 4 == 0) ? Redir : "",
			RegName[i], bb + strlen(RegName[i]) * sizeof(char),
			Reg[i], (i % 4 == 3) ? "\n" : "     ");
	}
}

// Dump Memory Content where the addresses are in decimal format
void DumpMem(int from, int to)
{
	int i, mt, ma;

	(to < from) && (to = from);
	if (from == to)
	{
		fprintf(stdout, "%s %05d        %08x\n", Redir, from, Mem[from]);
	}
	else
	{
		mt = Mem[ma = from];
		for (i = from + 1; i <= to; i++)
		{
			if (i == to || Mem[i] != mt)
			{
				if (i == ma + 1)
					fprintf(stdout, "%s %05d        %08x\n",
						Redir, ma, mt);
				else
					fprintf(stdout, "%s %05d-%05d  %08x\n",
						Redir, ma, i - 1, mt);
				(i != to) && (mt = Mem[ma = i]);
			}
		}
	}
}

// Dump Memory Content in Hex format
void DumpMemHex(int from, int to)
{
	int i, mt, ma;

	(to < from) && (to = from);
	if (from == to)
	{
		fprintf(stdout, "%s %05x        %08x\n", Redir, from * 4, Mem[from]);
	}
	else
	{
		mt = Mem[ma = from];
		for (i = from + 1; i <= to; i++)
		{
			if (i == to || Mem[i] != mt)
			{
				if (i == ma + 1)
					fprintf(stdout, "%s %05x        %08x\n",
						Redir, ma * 4, mt);
				else
					fprintf(stdout, "%s %05x-%05x  %08x\n",
						Redir, ma * 4, (i - 1) * 4, mt);
				(i != to) && (mt = Mem[ma = i]);
			}
		}
	}
}


void DumpHex(int from, int to)
{
	int i, j;

	if (to < from)
	{
		for (i = from, j = 0; i >= to; i--, j++)
		{
			if (j % 4 == 0)
				fprintf(stdout, "%s %04x  ", Redir, (i << 2) + 3);
			fprintf(stdout, " %08x%s", Mem[i], (j % 4 == 3) ? "\n" : "");
		}
	}
	else
	{
		for (i = from, j = 0; i <= to; i++, j++)
		{
			if (j % 4 == 0)
				fprintf(stdout, "%s %04x  ", Redir, i << 2);
			fprintf(stdout, " %08x%s", Mem[i], (j % 4 == 3) ? "\n" : "");
		}
	}
	if (j % 4 != 0)
		fputc('\n', stdout);
}

void Loop(void)
{
	char *tp;
	int sc;

	Init();
	for (;;)
	{
		fprintf(stdout, "\n%s cmd: ", Redir);
		Buf[0] = '\0';
		if (fgets(Buf, BUFSIZE, stdin) == NULL)
			continue;
		if ((tp = strtok(Buf, " ,.\t\n\r")) == NULL)
			continue;
		fputc('\n', stdout);
		switch (*tp)
		{
		case 'g': case 'G':
			DisplayControlSignals();
			break;
		case 'r': case 'R':
			DumpReg();
			break;
		case 'm': case 'M':
			if ((tp = strtok(NULL, " ,.\t\n\r")) == NULL)
			{
				DumpMemHex(0, MEMSIZE);
			}
			else
			{
				sc = (int)strtoul(tp, (char **)NULL, 10);
				if ((tp = strtok(NULL, " ,.\t\n\r")) == NULL)
				{
					DumpMemHex(sc, MEMSIZE);
				}
				else
				{
					DumpMemHex(sc, (int)strtoul(tp, (char **)NULL, 10));
				}
			}
			break;
		case 's': case 'S':
			if ((tp = strtok(NULL, " ,.\t\n\r")) == NULL)
				sc = 1;
			else
				sc = (int)strtoul(tp, (char **)NULL, 10);
			while (sc-- > 0 && !Halt)
				Step();
			fprintf(stdout, "%s step\n", Redir);
			break;
		case 'c': case 'C':
			while (!Halt)
				Step();
			fprintf(stdout, "%s cont\n", Redir);
			break;
		case 'h': case 'H':
			fprintf(stdout, "%s %s\n", Redir, Halt ? "true" : "false");
			break;
		case 'p': case 'P':
			rewind(FP);
			sc = 0;
			while (!feof(FP))
			{
				if (fgets(Buf, BUFSIZE, FP))
					fprintf(stdout, "%s % 5d  %s", Redir, sc++, Buf);
			}
			break;
		case 'i': case 'I':
			fprintf(stdout, "%s %d\n", Redir, MEMSIZE);
			break;
		case 'd': case 'D':
			if ((tp = strtok(NULL, " ,.\t\n\r")) == NULL)
			{
				fprintf(stdout, "%s invalid cmd\n", Redir);
				break;
			}
			sc = (int)strtoul(tp, (char **)NULL, 10);
			if ((tp = strtok(NULL, " ,.\t\n\r")) == NULL)
			{
				fprintf(stdout, "%sinvalid cmd\n", Redir);
				break;
			}
			DumpHex(sc, (int)strtoul(tp, (char **)NULL, 10));
			break;
		case 'x': case 'X': case 'q': case 'Q':
			fprintf(stdout, "%s quit\n", Redir);
			if (Redir == (char *)RedirPrefix)
			{
				fprintf(stdout, "%s%s\n", Redir, Redir);
			}
			return;
		default:
			fprintf(stdout, "%s invalid cmd\n", Redir);
			break;
		}
		if (Redir == (char *)RedirPrefix)
		{
			fprintf(stdout, "%s%s\n", Redir, Redir);
		}
	}
}

//int main(int argc, char **argv)
int main(void)
{
	int i;
	unsigned long t;

	//setvbuf(stdout, (char *) NULL, _IOLBF, 0);
	/*if (argc != 2 && argc != 3)
	{
	fprintf(stderr, "syntax: %s input_file [-r]\n", argv[0]);
	return 1;
	}
	if (*argv[1] == '-')
	{
	fprintf(stderr, "syntax: %s input_file [-r]\n", argv[0]);
	return 1;
	}
	if ((FP = fopen(argv[1], "r")) == NULL)
	{
	fprintf(stderr, "%s: cannot open input file %s\n", argv[0], argv[1]);
	return 1;
	//}*/
	//if (argc == 3)
	//{
	//	if (strcmp(argv[2], "-r") == 0)
	//	{
	FP = fopen("test01.asc", "r");
	Redir = (char *)RedirPrefix;
	/*	fprintf(stdout, "%s\n", argv[0]);
	}*/
	/*else
	{*/
	//		fprintf(stderr, "syntax: %s input_file [-r]\n", argv[0]);
	//		return 1;
	//	}
	//}
	memset(Mem, 0, MEMSIZE * sizeof(unsigned));
	for (i = PCINIT; !feof(FP); i += 4)
	{
		if (fgets(Buf, BUFSIZE, FP) == NULL)
		{
			if (feof(FP))
				break;

			return 1;
		}

		if (scanf_s(Buf, "%lx", &t) == 1)
		{
			MEM(i) = 0;
		}
		else
		{
			MEM(i) = strtoul(Buf, (char **)NULL, 16);
		}
	}

	Loop();
	fclose(FP);
	system("pause");
	return 0;
}
//Complier.exe test01.asm -r

