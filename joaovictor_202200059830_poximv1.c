//
// Poxim-V C simulator example
// 
// (C) Copyright 2024 Bruno Otavio Piedade Prado
//
// This file is part of Poxim-V.
//
// Poxim-V is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Poxim-V is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Poxim-V.  If not, see <https://www.gnu.org/licenses/>.
//

// How to build and run:
// $ gcc -Wall -O3 nomesobrenome_123456789012_exemplo.c -o nomesobrenome_123456789012_exemplo.elf
// $ ./nomesobrenome_123456789012_exemplo.elf input.hex output.out

// Standard integer library
#include <stdint.h>
// Standard library
#include <stdlib.h>
// Standard I/O library
#include <stdio.h>
#include <string.h>

/**
 * Main function
 * @param argc	Number of command line arguments
 * @param argv	Command line arguments
 * @return		Returns the program execution status
 */
int main(int argc, char* argv[]) {
	// Outputting separator
	printf("--------------------------------------------------------------------------------\n");
	// Iterating over arguments
	for(uint32_t i = 0; i < argc; i++) {
		// Outputting argument
		printf("argv[%i] = %s\n", i, argv[i]);
	}
	// Opening input and output files using proper permissions
	FILE* input = fopen(argv[1], "r");
	FILE* output = fopen(argv[2], "w");
	// Setting memory offset to 0x80000000
	const uint32_t offset = 0x80000000;
	// Creating 32 registers initialized with zero and labels
	uint32_t x[32] = { 0 };
	const char* x_label[32] = { "zero", "ra", "sp", "gp", "tp", "t0", "t1", "t2", "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7", "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6" };
	// Creating pc register initialized with memory offset
	uint32_t pc = offset;
	// Creating 32 KiB memory for both data and instructions
	uint8_t* mem = (uint8_t*)(malloc(32 * 1024));
	// Reading memory contents from input hexadecimal file
	
	
	// Processando o arquivo linha por linha
	size_t num_bytes = 0;  // Contador de bytes carregados
	char linha[256];       // Buffer para armazenar linhas do arquivo

	while (fgets(linha, sizeof(linha), input)) {
		// Ignorar linhas que começam com '@'
		if (linha[0] == '@') {
			continue;
		}

		// Processar cada par de caracteres como um byte hexadecimal
		for (size_t i = 0; linha[i] != '\0'; i += 3) {  // Avança 3 posições (2 caracteres + espaço)
			if (linha[i] == '\n' || linha[i] == '\r' || linha[i] == '\0') {
				break;  // Ignorar quebras de linha ou fim da string
			}

			uint8_t valor;
			if (sscanf(&linha[i], "%2hhX", &valor) == 1) {  // Lê 2 caracteres como um byte hexadecimal
				mem[num_bytes++] = valor;
				

				// Verifica se ultrapassou o limite de memória
				if (num_bytes >= 32 * 1024) {
					fprintf(stderr, "Erro: Arquivo excede o limite de memória de 32 KiB\n");
					fclose(input);
					fclose(output);
					return 1;
				}
			} else {
				fprintf(stderr, "Erro ao converter os dados na posição %zu: %s\n", i, linha);
			}
		}
	}





	// Outputting separator
	printf("--------------------------------------------------------------------------------\n");
	// Setting run condition
	uint8_t run = 1;
	// Loop while condition is true
	while(run) {
		// Reading instruction from memory (4 byte alignment)
		const uint32_t instruction = ((uint32_t*)(mem))[(pc - offset) >> 2];
		// Retrieving instruction opcode (6:0)
		const uint8_t opcode = instruction & 0b1111111;
		// Retrieving instruction fields
		const uint8_t funct7 = instruction >> 25;
		const uint16_t imm = instruction >> 20;
		const uint8_t uimm = (instruction & (0b11111 << 20)) >> 20;
		const uint8_t rs1 = (instruction & (0b11111 << 15)) >> 15;
		const uint8_t rs2 = (instruction >> 20) & 0b11111;
		const uint8_t funct3 = (instruction & (0b111 << 12)) >> 12;
		const uint8_t rd = (instruction & (0b11111 << 7)) >> 7;
		const uint32_t imm20 = ((instruction >> 31) << 19) | (((instruction & (0b11111111 << 12)) >> 12) << 11) | (((instruction & (0b1 << 20)) >> 20) << 10) | ((instruction & (0b1111111111 << 21)) >> 21);
		// Checking instruction opcode
		//    fprintf(output,"rs2: 0x%08x\n",rs2);
		//  fprintf(output,"rs1: 0x%08x\n",rs1);
		switch(opcode) {
			// I type (0010011)
			case 0b0010011:
				// slli (funct3 == 001 and funct7 == 0000000)
				if(funct3 == 0b001 && funct7 == 0b0000000) {
					// Calculating operation data
					const uint32_t data = x[rs1] << uimm;
					// Outputting instruction to console
					fprintf(output,"0x%08x:slli   %s,%s,%u  %s=0x%08x<<%u=0x%08x\n", pc, x_label[rd], x_label[rs1], imm, x_label[rd], x[rs1], imm, data);
					// Updating register if not x[0] (zero)
					if(rd != 0) x[rd] = data;
				}
				// addi 
				if (funct3 == 0b000) {
					// Extensão de sinal do imediato de 12 bits
					int32_t imm_sext = (int32_t)(imm << 20) >> 20;

					// Calcula o resultado da soma
					const uint32_t data = x[rs1] + imm_sext;

					// Exibe a instrução no console com o imediato em hexadecimal (sinalizado)
					fprintf(output,"0x%08x:addi   %s,%s,0x%03x   %s=0x%08x+0x%08x=0x%08x\n",
						pc, x_label[rd], x_label[rs1], (uint32_t)imm_sext & 0xFFF,  // Imediato formatado
						x_label[rd], x[rs1], imm_sext, data);

					// Atualiza o registrador de destino, exceto x[0] (zero)
					if (rd != 0) {
						x[rd] = data;
					}
				}
				//xori
				if (funct3 == 0b100)
				{
					int32_t imm_xori = (int32_t)(imm << 20) >> 20;

					// Calcula o resultado da soma
					const uint32_t data = x[rs1] ^ imm_xori;

					// Exibe a instrução no console com o imediato em hexadecimal (sinalizado)
					fprintf(output,"0x%08x:xori   %s,%s,0x%03x   %s=0x%08x^0x%08x=0x%08x\n",
						pc, x_label[rd], x_label[rs1], (uint32_t)imm_xori & 0xFFF,  // Imediato formatado
						x_label[rd], x[rs1], imm_xori, data);

					// Atualiza o registrador de destino, exceto x[0] (zero)
					if (rd != 0) {
						x[rd] = data;
					}
				}
				//ori
				if (funct3 == 0b110)
				{
					int32_t imm_ori = (int32_t)((imm << 20) >> 20);  // Extensão de sinal correta

					// Se o registrador rs1 for zero, o resultado será apenas o imediato
					uint32_t data = (rs1 == 0) ? imm_ori : x[rs1] | (uint32_t)imm_ori;  // OR com o imediato sinalizado ou com rs1

					// Exibe a instrução corretamente no console
					fprintf(output, "0x%08x:ori   %s,%s,0x%03x   %s=0x%08x|0x%08x=0x%08x\n",
							pc, x_label[rd], x_label[rs1], imm & 0xFFF,  // Mostrar imediato corretamente
							x_label[rd], x[rs1], imm_ori, data);

					// Atualiza o registrador de destino, exceto x0
					if (rd != 0) {
						x[rd] = data;
					}
				}
				//andi
				if (funct3 == 0b111)
				{
					int32_t imm_andi = (int32_t)(imm << 20) >> 20;

					// Calcula o resultado da soma
					const uint32_t data = x[rs1] & imm_andi;

					// Exibe a instrução no console com o imediato em hexadecimal (sinalizado)
					fprintf(output,"0x%08x:andi   %s,%s,0x%03x   %s=0x%08x&0x%08x=0x%08x\n",
						pc, x_label[rd], x_label[rs1], (uint32_t)imm_andi & 0xFFF,  // Imediato formatado
						x_label[rd], x[rs1], imm_andi, data);

					// Atualiza o registrador de destino, exceto x[0] (zero)
					if (rd != 0) {
						x[rd] = data;
					}
				}
				//srli
				if (funct3 == 0b101 && funct7 == 0b00000000)
				{
					uint32_t shamt = (instruction >> 20) & 0x1F;
					uint32_t valor_original = (uint32_t)x[rs1]; // Garante unsigned
					uint32_t resultado = valor_original >> shamt;  // Deslocamento lógico

					x[rd] = (int32_t)resultado;  // Armazena corretamente em x[rd]

					fprintf(output, "0x%08x:srli   %s,%s,%d          %s=0x%08x>>%d=0x%08x\n",
						pc, x_label[rd], x_label[rs1], shamt, x_label[rd], valor_original, shamt, resultado);

				}
				//srai
				if (funct3 == 0b101 && funct7 == 0b0100000)
				{
					uint32_t shamt = (instruction >> 20) & 0x1F;
					int32_t valor_original = (int32_t)x[rs1];  // Converte explicitamente para signed
					int32_t resultado = valor_original >> shamt;  // Deslocamento aritmético

					x[rd] = resultado;  // Armazena corretamente o valor deslocado

					fprintf(output, "0x%08x:srai   %s,%s,%d          %s=0x%08x>>>%d=0x%08x\n",
						pc, x_label[rd], x_label[rs1], shamt, x_label[rd], (uint32_t)valor_original, shamt, (uint32_t)resultado);
				}

				
				//sltiu
				if (funct3 == 0b011) {	
					// Extração correta do imediato de 12 bits
					int32_t imm = (int32_t)(instruction >> 20);  // Captura os 12 bits superiores
					// Correção: Sign-extend (extensão de sinal para 32 bits)
					if (imm & 0x800) {  // Se o bit 11 estiver definido (sinal negativo)
						imm |= 0xFFFFF000;  // Preenche os bits superiores com 1s
					}

					// Comparação sem sinal (unsigned)
					if ((uint32_t)x[rs1] < (uint32_t)imm) {
						x[rd] = 1;
					} else {
						x[rd] = 0;
					}

					// Impressão corrigida
					fprintf(output, "0x%08x:sltiu   %s,%s,0x%03x       %s=(0x%08x<0x%08x)=%d\n", 
						pc, x_label[rd], x_label[rs1], (imm & 0xFFF), x_label[rd], x[rs1], imm, x[rd]);
				}
				//slti
				if (funct3 == 0b010)
				{
					int32_t imm = (int32_t)(instruction >> 20);  // Captura os 12 bits superiores
					// Correção: Sign-extend (extensão de sinal para 32 bits)
					if (imm & 0x800) {  // Se o bit 11 estiver definido (sinal negativo)
						imm |= 0xFFFFF000;  // Preenche os bits superiores com 1s
					}


					// Comparação sem sinal (unsigned)
					if ((int32_t)x[rs1] < (int32_t)imm) {
						x[rd] = 1;
					} else {
						x[rd] = 0;
					}

					// Impressão corrigida
					fprintf(output, "0x%08x:slti   %s,%s,0x%03x       %s=(0x%08x<0x%08x)=%d\n", 
						pc, x_label[rd], x_label[rs1], (imm & 0xFFF), x_label[rd], x[rs1], imm, x[rd]);
				
				}
				
				
				
				
				

				// Breaking case
				break;
			// U Type (0010111)
			case 0b0010111:
				{
					// Calcula o imediato deslocado (imm << 12)
					const uint32_t imm_auipc = ((instruction >> 12) & 0xFFFFF) << 12;
					// Exibe a instrução no console
					fprintf(output,"0x%08x:auipc  %s,0x%05x    %s=0x%08x+0x%08x=0x%08x\n",pc, x_label[rd], (imm_auipc >> 12), x_label[rd], pc, imm_auipc, pc + imm_auipc);


					// Atualiza o registrador de destino, exceto x[0] (zero)
					if (rd != 0) {
						x[rd] = pc+imm_auipc;
					}
				}
				break;			

			// U type (0110111) 
			case 0b0110111:
				uint32_t imm_lui = instruction & 0xFFFFF000;

				x[rd] = imm_lui;

				fprintf(output,"0x%08x:lui    %s,0x%05x     %s=0x%08x\n", 
					pc, x_label[rd], imm_lui >> 12, x_label[rd], x[rd]);
			
			break;

			// R type (0110011)
			case 0b0110011:
			{
				uint8_t funct3 = (instruction >> 12) & 0x7;
        		uint8_t funct7 = (instruction >> 25) & 0x7F;

				// Verifica se a instrução é ADD
				if (funct3 == 0b000 && funct7 == 0b0000000) {
					uint32_t data;

					// Se rs1 for zero, o resultado será apenas o valor de rs2
					if (rs1 == 0) {
						data = x[rs2];  // Não soma nada, apenas copia rs2
					}
					if (rs2 == 0)
					{
						data = x[rs1];
					}
					
					 else {
						data = x[rs1] + x[rs2];  // Caso contrário, realiza a soma normal
					}

					// Exibe a instrução corretamente no console
					fprintf(output, "0x%08x:add    %s,%s,%s       %s=0x%08x+0x%08x=0x%08x\n",
							pc, x_label[rd], x_label[rs1], x_label[rs2],
							x_label[rd], x[rs1], x[rs2], data);

					// Atualiza o registrador de destino, exceto x0
					if (rd != 0) {
						x[rd] = data;
					}
				}
				
					//mul
				if (funct3 == 0b000 && funct7 == 0b0000001 ) { 
					// Realiza a multiplicação
					int32_t rs1_print = x[rs1];
					int32_t result = (int32_t)x[rs1] * (int32_t)x[rs2];

					// Armazena o resultado em rd
					x[rd] = result;

					// Imprime a instrução e o resultado
					fprintf(output,"0x%08x:mul    %s,%s,%s         %s=0x%08x*0x%08x=0x%08x\n", 
						pc, x_label[rd], x_label[rs1], x_label[rs2], x_label[rd], rs1_print, x[rs2], result);
				}
					//mulh
				if (funct3 == 0b001 && funct7 == 0b0000001 ) {  
					// Realiza a multiplicação de 64 bits e pega a parte alta
					int64_t result = (int64_t)((int32_t)x[rs1]) * (int64_t)((int32_t)x[rs2]);

					// Armazena a parte alta do resultado em rd (32 bits mais significativos)
					x[rd] = (uint32_t)(result >> 32);

					// Imprime a instrução e o resultado
					fprintf(output,"0x%08x:mulh   %s,%s,%s         %s=0x%08x*0x%08x=0x%08x\n", 
						pc, x_label[rd], x_label[rs1], x_label[rs2], x_label[rd], x[rs1], x[rs2], x[rd]);
				}
					//mulhsu
				 if (funct3 == 0b010 && funct7 == 0b0000001) {
						// Realiza a multiplicação de um número assinado (rs1) com um número não assinado (rs2)
						int32_t signed_rs1 = (int32_t)x[rs1];   // rs1 é tratado como assinado
						uint32_t unsigned_rs2 = (uint32_t)x[rs2];  // rs2 é tratado como não assinado

						// Realiza a multiplicação de 64 bits
						int64_t result = (int64_t)signed_rs1 * (int64_t)unsigned_rs2;

						// Armazena a parte alta (high) do resultado no registrador rd
						x[rd] = (uint32_t)(result >> 32);  // Pegando os 32 bits mais significativos

						// Imprime a instrução e o resultado
						fprintf(output,"0x%08x:mulhsu %s,%s,%s         %s=0x%08x*0x%08x=0x%08x\n", 
							pc, x_label[rd], x_label[rs1], x_label[rs2], x_label[rd], signed_rs1, unsigned_rs2, x[rd]);
					}
					// mulhu
				if (funct3 == 0b011 && funct7 == 0b0000001) {
						// Realiza a multiplicação de dois números não assinados (rs1 * rs2)
						uint32_t unsigned_rs1 = (uint32_t)x[rs1];   // rs1 é tratado como não assinado
						uint32_t unsigned_rs2 = (uint32_t)x[rs2];   // rs2 é tratado como não assinado

						// Realiza a multiplicação de 64 bits
						uint64_t result = (uint64_t)unsigned_rs1 * (uint64_t)unsigned_rs2;

						// Armazena a parte alta (high) do resultado no registrador rd
						x[rd] = (uint32_t)(result >> 32);  // Pegando os 32 bits mais significativos

						// Imprime a instrução e o resultado
						fprintf(output,"0x%08x:mulhu  %s,%s,%s         %s=0x%08x*0x%08x=0x%08x\n", 
							pc, x_label[rd], x_label[rs1], x_label[rs2], x_label[rd], unsigned_rs1, unsigned_rs2, x[rd]);
					}
					//div
				if( funct3 == 0b100 && funct7 == 0b0000001){
					int32_t dividend = (int32_t)x[rs1];  
		            int32_t divisor = (int32_t)x[rs2];   
        		    int32_t result;

					if (divisor == 0) {
						// Quando o divisor for zero, o resultado é 0xffffffff
						result = 0xffffffff;
						fprintf(output,"0x%08x:div    %s,%s,%s         %s=0x%08x/0x%08x=0x%08x\n", 
							pc,x_label[rd], x_label[rs1], x_label[rs2], x_label[rd], dividend, divisor, result);
						
						break;
						
					}

					result = dividend / divisor;

					x[rd] = result;

					fprintf(output,"0x%08x:div    %s,%s,%s         %s=0x%08x/0x%08x=0x%08x\n", 
                   		pc, x_label[rd], x_label[rs1], x_label[rs2], x_label[rd], dividend, divisor, result);

				}
					//divu
				if(funct3 == 0b101 && funct7 == 0b0000001){
					uint32_t dividend = (int32_t)x[rs1];  
		            uint32_t divisor = (int32_t)x[rs2];   
        		    uint32_t result;

					if (divisor == 0) {
						// Quando o divisor for zero, o resultado é 0xffffffff
						result = 0xffffffff;
						fprintf(output,"0x%08x:divu    %s,%s,%s         %s=0x%08x/0x%08x=0x%08x\n", 
							pc,x_label[rd], x_label[rs1], x_label[rs2], x_label[rd], dividend, divisor, result);
						
						break;
					}

					result = dividend / divisor;

					x[rd] = result;

					fprintf(output,"0x%08x:divu    %s,%s,%s         %s=0x%08x/0x%08x=0x%08x\n", 
                   		pc, x_label[rd], x_label[rs1], x_label[rs2], x_label[rd], dividend, divisor, result);

				}
				//rem 
				if (funct3 == 0b110 && funct7 == 0b0000001)
				{
					int32_t dividend = x[rs1];  
		            int32_t divisor = x[rs2];   
        		    int32_t result;


					if(divisor == 0){
						result = dividend;
					}
					else{
						result = dividend % divisor;
					}

					x[rd] = result;

					fprintf(output,"0x%08x:rem    %s,%s,%s         %s=0x%08x%%0x%08x=0x%08x\n",
           					pc, x_label[rd], x_label[rs1], x_label[rs2], x_label[rd], dividend, divisor, result);
				}
				//remu 
				if (funct3 == 0b111 && funct7 == 0b0000001)
				{
					uint32_t dividend = x[rs1];  
		            uint32_t divisor = x[rs2];   
        		    uint32_t result;


					if(divisor == 0){
						result = dividend;
					}
					else{
						result = dividend % divisor;
					}

					x[rd] = result;

					fprintf(output,"0x%08x:remu    %s,%s,%s         %s=0x%08x%%0x%08x=0x%08x\n",
           					pc, x_label[rd], x_label[rs1], x_label[rs2], x_label[rd], dividend, divisor, result);
				}
				//sub
				if (funct3 == 0b000 && funct7 == 0b0100000)
				{
					uint32_t data;

					// Se rs1 for zero, o resultado será apenas o valor de rs2
					if (rs1 == 0) {
						data = x[rs2];  // Não soma nada, apenas copia rs2
					}
					if (rs2 == 0)
					{
						data = x[rs1];
					}
					
					 else {
						data = x[rs1] - x[rs2];  // Caso contrário, realiza a soma normal
					}

					// Exibe a instrução corretamente no console
					fprintf(output, "0x%08x:sub    %s,%s,%s       %s=0x%08x-0x%08x=0x%08x\n",
							pc, x_label[rd], x_label[rs1], x_label[rs2], x_label[rd], x[rs1], x[rs2], data);

					// Atualiza o registrador de destino, exceto x0
					if (rd != 0) {
						x[rd] = data;
					}
				}
				//xor
				if (funct3 == 0b100 && funct7 == 0b00000000)
				{
					uint32_t data;

					// Se rs1 for zero, o resultado será apenas o valor de rs2
					if (rs1 == 0) {
						data = x[rs2];  // Não soma nada, apenas copia rs2
					}
					if (rs2 == 0)
					{
						data = x[rs1];
					}
					
					 else {
						data = x[rs1] ^ x[rs2];  // Caso contrário, realiza a soma normal
					}
					
					fprintf(output,"0x%08x:xor    %s,%s,%s       %s=0x%08x^0x%08x=0x%08x\n", 
						pc, x_label[rd], x_label[rs1], x_label[rs2], x_label[rd], x[rs1], x[rs2], data);
						
					if (rd != 0) {
						x[rd] = data;
					}	
				}
				//or
				if (funct3 == 0b110 && funct7 == 0b00000000)
				{

					uint32_t data;
					if (rs1 == 0) {
						data = x[rs2];  // Não soma nada, apenas copia rs2
					}
					if (rs2 == 0)
					{
						data = x[rs1];
					}
					
					if (rs1 != 0 && rs2 != 0)
					 {
						data = x[rs1] |  x[rs2];  // Caso contrário, realiza a soma normal
					}
					
					fprintf(output,"0x%08x:or    %s,%s,%s       %s=0x%08x|0x%08x=0x%08x\n", 
						pc, x_label[rd], x_label[rs1], x_label[rs2], x_label[rd], x[rs1], x[rs2], data);
					if (rd != 0) {
						x[rd] = data;
					}	
				}
				//and 
				if (funct3 == 0b111 && funct7 == 0b00000000)
				{
					uint32_t data;
					 if (rs1 == 0 || rs2 == 0) {
						data = 0x00000000;
					} else {
						data = x[rs1] & x[rs2];  // Operação normal de AND
					}
	
					
					fprintf(output,"0x%08x:and    %s,%s,%s       %s=0x%08x&0x%08x=0x%08x\n", 
						pc, x_label[rd], x_label[rs1], x_label[rs2], x_label[rd], x[rs1], x[rs2], data);
					
					if (rd != 0) {
						x[rd] = data;
					}	
					
				}
				//sll
				if (funct3 == 0b001 && funct7 == 0b00000000)
				{
					uint32_t shift_amount = x[rs2] & 0x1F;  // Apenas os 5 bits menos significativos
					uint32_t result_sll = x[rs1] << shift_amount;

					x[rd] = result_sll;

					fprintf(output,"0x%08x:sll    %s,%s,%s       %s=0x%08x<<%d=0x%08x\n", 
						pc, x_label[rd], x_label[rs1], x_label[rs2], x_label[rd], x[rs1], shift_amount, result_sll);
				}
				//srl
				if (funct3 == 0b101 && funct7 == 0b00000000)
				{
					uint32_t shamt = x[rs2] & 0x1F; // Pega apenas os 5 bits menos significativos para o deslocamento
					uint32_t result_srl = x[rs1] >> shamt; 
					
					x[rd] = result_srl; 

					fprintf(output,"0x%08x:srl    %s,%s,%s       %s=0x%08x>>%d=0x%08x\n", 
							pc, x_label[rd], x_label[rs1], x_label[rs2], 
							x_label[rd], x[rs1], shamt, result_srl);
				}
				//sra
				if (funct3 == 0b101 && funct7 == 0b0100000 )
				{
					int32_t sig_value = (int32_t)x[rs1];
					int32_t shamt = x[rs2] & 0x1F;  // Apenas os 5 bits menos significativos
					int32_t result_sra = sig_value >> shamt;

					x[rd] = result_sra;

					fprintf(output,"0x%08x:sra    %s,%s,%s       %s=0x%08x>>>%d=0x%08x\n", 
							pc, x_label[rd], x_label[rs1], x_label[rs2], 
							x_label[rd], x[rs1], shamt, result_sra);
				}
				//slt
				if (funct3 == 0b010 && funct7 == 0b00000000)
				{
					int32_t rs1_value = (rs1 == 0) ? 0 : (int32_t)x[rs1];  // Força rs1 = 0 se for zero
					int32_t rs2_value = (int32_t)x[rs2];
					int32_t result = (rs1_value < rs2_value) ? 1 : 0;

					// Apenas escreve se rd != x0
					if (rd != 0) {
						x[rd] = result;
					}

					// Usa rs1_value (corrigido) para exibição correta
					fprintf(output, "0x%08x:slt     %s,%s,%s         %s=(0x%08x<0x%08x)=%d\n", 
							pc, x_label[rd], x_label[rs1], x_label[rs2], 
							x_label[rd], rs1_value, rs2_value, result);
					
				}

					
				
				//sltu
				if (funct3 == 0b011 && funct7 == 0b00000000)
				{
					uint32_t rs1_value = (rs1 == 0) ? 0 : (uint32_t)x[rs1];  // Força rs1 = 0 se for zero
					uint32_t rs2_value = (uint32_t)x[rs2];
					uint32_t result = (rs1_value < rs2_value) ? 1 : 0;

					// Apenas escreve se rd != x0
					if (rd != 0) {
						x[rd] = result;
					}

					// Usa rs1_value (corrigido) para exibição correta
					fprintf(output, "0x%08x:sltu     %s,%s,%s         %s=(0x%08x<0x%08x)=%d\n", 
							pc, x_label[rd], x_label[rs1], x_label[rs2], 
							x_label[rd], rs1_value, rs2_value, result);
					
				}
				

	
			break;
	}

			// S Type (0100011)
			case 0b0100011:
			// sw (funct3 == 010)
				if (funct3 == 0b010) {
					// Cálculo do imediato para SW (S-type)
					uint32_t imm_sw = ((instruction >> 25) << 5) | ((instruction >> 7) & 0x1F);

					// Extensão de sinal para 12 bits
					int32_t simm_sw = ((int32_t)(imm_sw << 20)) >> 20;

					// Cálculo do endereço de memória (rs1 + offset)
					uint32_t address = x[rs1] + simm_sw;

					// Armazena a palavra (32 bits) de rs2 na memória (endereçamento por byte)
					*(uint32_t*)(mem + (address - offset)) = x[rs2];

					// Exibe a instrução no console
					fprintf(output,"0x%08x:sw     %s,0x%03x(%s)    mem[0x%08x]=0x%08x\n",
						pc, x_label[rs2], simm_sw, x_label[rs1], address, x[rs2]);
				}
				//sb
				if (funct3 == 0b000)
				{
					// Cálculo do imediato para SW (S-type)
					uint32_t imm_sb = ((instruction >> 25) << 5) | ((instruction >> 7) & 0x1F);

					// Extensão de sinal para 12 bits
					int32_t simm_sb = ((int32_t)(imm_sb << 20)) >> 20;

					// Cálculo do endereço de memória (rs1 + offset)
					uint32_t address = x[rs1] + simm_sb;

					// Armazena a palavra (32 bits) de rs2 na memória (endereçamento por byte)
					*(uint8_t*)(mem + (address - offset)) =(uint8_t)x[rs2];

					// Exibe a instrução no console
					fprintf(output,"0x%08x:sb    %s,0x%03x(%s)    mem[0x%08x]=0x%02x\n",
						pc, x_label[rs2], simm_sb, x_label[rs1], address, (uint8_t)x[rs2]);
				}
				//sh
				if (funct3 == 0b001)
				{
					// Cálculo do imediato para SW (S-type)
					uint32_t imm_sh = ((instruction >> 25) << 5) | ((instruction >> 7) & 0x1F);

					// Extensão de sinal para 12 bits
					int32_t simm_sh = ((int32_t)(imm_sh << 20)) >> 20;

					// Cálculo do endereço de memória (rs1 + offset)
					uint32_t address = x[rs1] + simm_sh;

					// Armazena a palavra (32 bits) de rs2 na memória (endereçamento por byte)
					*(uint16_t*)(mem + (address - offset)) = (uint16_t)x[rs2];

					// Exibe a instrução no console
					fprintf(output,"0x%08x:sw     %s,0x%03x(%s)    mem[0x%08x]=0x%08x\n",
						pc, x_label[rs2], simm_sh, x_label[rs1], address, (uint16_t)x[rs2]);
				}
				
				
				break;
			// I type (1110011)
			case 0b1110011:
				// ebreak (funct3 == 000 and imm == 1)
				if(funct3 == 0b000 && imm == 1) {
					// Outputting instruction to console
					fprintf(output,"0x%08x:ebreak\n", pc);
					// Retrieving previous and next instructions
					const uint32_t previous = ((uint32_t*)(mem))[(pc - 4 - offset) >> 2];
					const uint32_t next = ((uint32_t*)(mem))[(pc + 4 - offset) >> 2];
					// Halting condition
            		if(previous == 0x01f01013 && next == 0x40705013) run = 0;
				}
				// Breaking case
				break;
			// blt Type (110011)
			case 0b1100011:
				//blt
				if(funct3 == 0b100){
					// Extração do imediato para instruções de branch
					int32_t imm_b = 0;

					imm_b |= ((instruction >> 31) & 0x1) << 12;  // Bit 12 (sinal)
					imm_b |= ((instruction >> 7) & 0x1) << 11;   // Bit 11
					imm_b |= ((instruction >> 25) & 0x3F) << 5;  // Bits 10-5
					imm_b |= ((instruction >> 8) & 0xF) << 1;    // Bits 4-1

					// Extensão de sinal explícita
					if (imm_b & 0x1000) {  // Se o bit 12 (sinal) for 1
					    imm_b |= 0xFFFFE000;  // Estende para 32 bits negativos
					}
 

					

						uint32_t pc_original = pc;
						
						int condition = ((int32_t)x[rs1] < (int32_t)x[rs2]) ? 1 : 0;

						pc = condition ? (pc + imm_b) : (pc + 4);

						

						// Impressão ajustada para exibir o imediato corretamente
						fprintf(output,"0x%08x:blt    %s,%s,0x%03x   (0x%08x<0x%08x)=%d->pc=0x%08x\n",
							pc_original, x_label[rs1], x_label[rs2],
							(uint32_t)((imm_b >> 1) & 0xFFF),  
							x[rs1], x[rs2], condition, pc);

						
					
				}
				//bne
				if (funct3 == 0b001)
				{
					int32_t imm_b = 0;

					// Extração do imediato (formato de branch)
					imm_b |= ((instruction >> 31) & 0x1) << 12;  // Bit 12 (sinal)
					imm_b |= ((instruction >> 7) & 0x1) << 11;   // Bit 11
					imm_b |= ((instruction >> 25) & 0x3F) << 5;  // Bits 10-5
					imm_b |= ((instruction >> 8) & 0xF) << 1;    // Bits 4-1

					// Extensão de sinal
					imm_b = (imm_b << 19) >> 19;

					uint32_t pc_original = pc;  // Salva o PC atual para impressão
					int condition = (x[rs1] != x[rs2]);  // Verifica se rs1 é diferente de rs2

					// Atualiza o PC dependendo da condição
					if (condition) {
						pc = pc + imm_b;
					} else {
						pc = pc + 4;
					}

					// Impressão detalhada
					fprintf(output,"0x%08x:bne    %s,%s,0x%03x   (0x%08x!=0x%08x)=%d->pc=0x%08x\n", 
						pc_original, x_label[rs1], x_label[rs2], 
						(uint32_t)((imm_b >> 1) & 0xFFF), x[rs1], x[rs2], condition, pc);
				}
				//beq
				if (funct3 == 0b000)
				{
					int32_t imm_b = 0;

					imm_b |= ((instruction >> 31) & 0x1) << 12;  // Bit 12 (sinal)
					imm_b |= ((instruction >> 7) & 0x1) << 11;   // Bit 11
					imm_b |= ((instruction >> 25) & 0x3F) << 5;  // Bits 10-5
					imm_b |= ((instruction >> 8) & 0xF) << 1;    // Bits 4-1

					// Extensão de sinal explícita
					if (imm_b & 0x1000) {  // Se o bit 12 (sinal) for 1, é negativo
						imm_b |= 0xFFFFE000;  // Estende para 32 bits com 1s
					}
					uint32_t pc_original = pc;
					int condition;
					if ((int32_t)x[rs1] == (int32_t)x[rs2]) {
						condition = 1;
					} else {
						condition = 0;
					}

					if (condition) {
						pc = pc + imm_b;
					} else {
						pc = pc + 4;
					}
						
					
					// Impressão ajustada para exibir o imediato corretamente
					fprintf(output,"0x%08x:beq    %s,%s,0x%03x   (0x%08x==0x%08x)=%d->pc=0x%08x\n",
							pc_original, x_label[rs1], x_label[rs2],
							(uint32_t)(imm_b & 0xFFF),  
							x[rs1], x[rs2], condition, pc);
			
				}
				//bge
				if (funct3 == 0b101)
				{
					int32_t imm_b = 0;

					imm_b |= ((instruction >> 31) & 0x1) << 12;  // Bit 12 (sinal)
					imm_b |= ((instruction >> 7) & 0x1) << 11;   // Bit 11
					imm_b |= ((instruction >> 25) & 0x3F) << 5;  // Bits 10-5
					imm_b |= ((instruction >> 8) & 0xF) << 1;    // Bits 4-1

					// Extensão de sinal explícita
					imm_b = (imm_b << 19) >> 19;

					uint32_t pc_original = pc;
					int condition = ((int32_t)x[rs1] >= (int32_t)x[rs2]) ? 1 : 0;
					pc = condition ? (pc + imm_b) : (pc + 4);
						
					
					// Impressão ajustada para exibir o imediato corretamente
					fprintf(output,"0x%08x:bge    %s,%s,0x%03x   (0x%08x>=0x%08x)=%d->pc=0x%08x\n",
							pc_original, x_label[rs1], x_label[rs2],
							(uint32_t)((imm_b >> 1)& 0xFFF),  
							x[rs1], x[rs2], condition, pc);
				}
				//bltu
				if (funct3 == 0b110)
				{
					// Extração do imediato para instruções de branch
					int32_t imm_b = 0;

					imm_b |= ((instruction >> 31) & 0x1) << 12;  // Bit 12 (sinal)
					imm_b |= ((instruction >> 7) & 0x1) << 11;   // Bit 11
					imm_b |= ((instruction >> 25) & 0x3F) << 5;  // Bits 10-5
					imm_b |= ((instruction >> 8) & 0xF) << 1;    // Bits 4-1

					// Extensão de sinal explícita
					imm_b = (imm_b << 19) >> 19;
; 
					uint32_t pc_original = pc;
					int condition = ((uint32_t)x[rs1] < (uint32_t)x[rs2]) ? 1 : 0;
					pc = condition ? (pc + imm_b) : (pc + 4);

						

					// Impressão ajustada para exibir o imediato corretamente
					fprintf(output,"0x%08x:bltu    %s,%s,0x%03x   (0x%08x<0x%08x)=%d->pc=0x%08x\n",
						pc_original, x_label[rs1], x_label[rs2],
						(uint32_t)((imm_b >>1) & 0xFFF),  
						x[rs1], x[rs2], condition, pc);

						
					
				}
				//bgeu
				if (funct3 == 0b111)
				{
					int32_t imm_b = 0;

					imm_b |= ((instruction >> 31) & 0x1) << 12;  // Bit 12 (sinal)
					imm_b |= ((instruction >> 7) & 0x1) << 11;   // Bit 11
					imm_b |= ((instruction >> 25) & 0x3F) << 5;  // Bits 10-5
					imm_b |= ((instruction >> 8) & 0xF) << 1;    // Bits 4-1

					// Extensão de sinal explícita
					imm_b = (imm_b << 19) >> 19;
 
					uint32_t pc_original = pc;
					int condition = ((uint32_t)x[rs1] >= (uint32_t)x[rs2]) ? 1 : 0;
					pc = condition ? (pc + imm_b) : (pc + 4);
						
					
					// Impressão ajustada para exibir o imediato corretamente
					fprintf(output,"0x%08x:bgeu    %s,%s,0x%03x   (0x%08x>=0x%08x)=%d->pc=0x%08x\n",
							pc_original, x_label[rs1], x_label[rs2],
							(uint32_t)((imm_b >>1) & 0xFFF),  
							x[rs1], x[rs2], condition, pc);
				}
				
				
				
				

				pc -= 4;
				
				break;
			case 0b1100111:   // JALR (Jump And Link Register)

				// Calculando o imediato de 12 bits (extração de bits do campo da instrução)
				int32_t imm_jarl = (int32_t)(instruction >> 20);  // Extraindo o imediato de 12 bits
				imm_jarl = (imm_jarl << 20) >> 20;  // Extensão de sinal para garantir um valor de 32 bits com sinal

				// Calculando o endereço de salto, utilizando rs1 e o imediato
				uint32_t target_address = (x[rs1] + imm_jarl) & ~1;  // Mascarando para garantir alinhamento
				int32_t pc_impressao = pc +4;

				// Imprimindo a instrução JALR de forma mais detalhada (ajustando para a saída esperada)
				fprintf(output,"0x%08x:jalr   %s,%s,0x%03x   pc=0x%08x+0x%08x,%s=0x%08x\n", 
					pc, x_label[rd], x_label[rs1], imm_jarl, target_address, imm_jarl, x_label[rd], pc_impressao);

				// Atualizando o registrador rd com o endereço de retorno (pc + 4)
				if (rd != 0) {
					x[rd] = pc + 4;
				}

				// Atualizando o PC com o endereço de destino
				pc = target_address - 4;

				break;

			// I type(0000011)
			case 0b0000011:{ 
				uint8_t funct3 = (instruction >> 12) & 0x7; // Extrair o campo funct3
				//lw
				if (funct3 == 0b010) {
					int32_t imm = (int32_t)(instruction >> 20); 
					imm = (imm << 20) >> 20; 

					// Calculando o endereço de memória
					uint32_t address = x[rs1] + imm;
					
					if(address >= offset && address < 0x80008000){
						x[rd] = *(uint32_t*)&mem[address - offset];
					}

					fprintf(output,"0x%08x:lw     %s,0x%03x(%s)       %s=mem[0x%08x]=0x%08x\n", 
            		pc, x_label[rd], imm, x_label[rs1], (x_label[rd]),address, x[rd]);
				}	

				//lb
				if (funct3 == 0b000)
				{	
					int32_t imm = (int32_t)(instruction >> 20); 
					imm = (imm << 20) >> 20;  // Extensão de sinal para o imediato

					// Calculando o endereço de memória
					uint32_t address = x[rs1] + imm;

					if (address >= offset && address < 0x80008000) {
						// Carrega um byte da memória e estende o sinal para 32 bits
						uint8_t byte = *(uint8_t*)&mem[address - offset];  // Leitura de 1 byte da memória
						x[rd] = (int32_t)(int8_t)byte;  // Extensão de sinal do byte para 32 bits
					}

					fprintf(output,"0x%08x:lb     %s,0x%03x(%s)       %s=mem[0x%08x]=0x%08x\n", 
						pc, x_label[rd], imm, x_label[rs1], x_label[rd], address, x[rd]);
				}
				//lh
				if (funct3 == 0b001)
				{
					int32_t imm = (int32_t)(instruction >> 20); 
					imm = (imm << 20) >> 20;  // Extensão de sinal para o imediato

					// Calculando o endereço de memória
					uint32_t address = x[rs1] + imm;

					if (address >= offset && address < 0x80008000) {
						// Carrega um byte da memória e estende o sinal para 32 bits
						uint8_t byte = *(uint16_t*)&mem[address - offset];  // Leitura de 1 byte da memória
						x[rd] = (int32_t)(int16_t)byte;  // Extensão de sinal do byte para 32 bits
					}

					fprintf(output,"0x%08x:lh     %s,0x%03x(%s)       %s=mem[0x%08x]=0x%08x\n", 
						pc, x_label[rd], imm, x_label[rs1], x_label[rd], address, x[rd]);
				}
				//lbu
				if (funct3 == 0b100)
				{	
					int32_t imm = (int32_t)(instruction >> 20); 
					imm = (imm << 20) >> 20;  // Extensão de sinal para o imediato

					// Calculando o endereço de memória
					uint32_t address = x[rs1] + imm;

					if (address >= offset && address < 0x80008000) {
						// Carrega um byte da memória e estende o sinal para 32 bits
						uint8_t byte = *(uint8_t*)&mem[address - offset];  // Leitura de 1 byte da memória
						x[rd] = (uint32_t)byte;  // Extensão de sinal do byte para 32 bits
					}

					fprintf(output,"0x%08x:lbu     %s,0x%03x(%s)       %s=mem[0x%08x]=0x%08x\n", 
						pc, x_label[rd], imm, x_label[rs1], x_label[rd], address, x[rd]);
				}
				//lhu
				if (funct3 == 0b101)
				{
					int32_t imm = (int32_t)(instruction >> 20); 
					imm = (imm << 20) >> 20;  // Extensão de sinal para o imediato

					// Calculando o endereço de memória
					uint32_t address = x[rs1] + imm;

					if (address >= offset && address < 0x80008000) {
						// Carrega um halfword (16 bits) da memória e estende para 32 bits com zeros
						uint16_t halfword = *(uint16_t*)&mem[address - offset];  // Leitura de 2 bytes da memória
						x[rd] = (uint32_t)halfword;  // Extensão sem sinal para 32 bits
					}

					fprintf(output, "0x%08x:lhu     %s,0x%03x(%s)       %s=mem[0x%08x]=0x%08x\n", 
							pc, x_label[rd], imm, x_label[rs1], x_label[rd], address, x[rd]);
								
				}
				
				
				
				
					    		
			break;
		}
			
			
			// Jal type (1101111)
			case 0b1101111:
				// Calculating operation address

				// Extensão de sinal correta para 21 bits
				 const int32_t simm = ((int32_t)(imm20 << 11)) >> 11;
				// Extensão de sinal auxiliar 

				uint32_t simm_aux = ((imm20 >> 20)) ? (0xFFF00000) : (imm20);
				simm_aux |= (simm_aux & 0x800) ? 0xFFFFF000 : 0x00000000;
				const uint32_t address = pc + (simm_aux<<1 ) ;
				// Outputting instruction to console
				fprintf(output,"0x%08x:jal    %s,0x%05x    pc=0x%08x,%s=0x%08x\n", pc, x_label[rd], simm  , address , x_label[rd], pc + 4);
				// Updating register if not x[0] (zero)
				if(rd != 0) x[rd] = pc + 4;
				// Setting next pc minus 4
				pc = address - 4;
				// Breaking case
				break;

				
			// Unknown
			default:
				// Outputting error message
				fprintf(output,"error: unknown instruction opcode at pc = 0x%08x\n", pc);
				// Halting simulation
				run = 0;
		}
		// Incrementing pc by 4
		pc = pc + 4;
	}
	// Closing input and output files
	// fclose(input);
	// fclose(output);
	// Outputting separator
	printf("--------------------------------------------------------------------------------\n");
	// Returning success status
	return 0;
}
