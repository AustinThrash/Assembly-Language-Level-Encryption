// EncryptData.cpp
//
// This file uses the input data and key information to encrypt the input data
//

#include "Main.h"

void encryptData_01(char* data, int datalength)
{
	int resulti = 0;

	__asm
	{
		//Starting_index
		xor eax, eax                					//Clearing eax
		xor ecx, ecx									//Clearing ecx
		mov esi, gptrPasswordHash   					//moving gptrPasswordHash into esi
		mov al, byte ptr[esi] 							//moving gPasswordHas[0+round*4] into al
		shl ax, 8		    							//equivlent to multiplying by 256, shifting left is like multipling my power of two
		mov cl, byte ptr[esi + 1] 						// gPasswordHash[1+round*4] into cl
		add ax, cx		    							//Starting_index[round] = gPasswordHash[0+round*4] * 256 + gPasswordHash[1+round*4];
													
		//mov index, byte ptr[eax + round]	
		//xor eax, eax				

		xor ebx, ebx		    						//This will be for dataLength
		xor ecx, ecx		    						//This will be for x or loop control
		mov ebx, datalength								//ebx equals dataLength
		cmp ebx, 0										//checking length of data
														//if ebx equals 0 then jump to exit_datazero
		jbe EXIT_DATAZERO
		mov edi, data									//putting data into edi
		mov esi, gptrKey								//putting gkey into esi
LOOP_FOR :
		//data[x] = data[x] ^ gkey[index] xoring each byte of data
		mov dl, byte ptr[edi + ecx]						//gets data[x] into al
		xor dl, byte ptr[esi + eax]						//data[x] ^ gkey[index] in al
		mov byte ptr[edi + ecx], dl						//putting al back into data[x]

		add ecx, 1 										//increment ecx(loop control) by 1
		cmp ecx, ebx									//compare x to datalength
		je EXIT_FOR										//if greater than, exit for
		jmp LOOP_FOR									//if not, jump back to start of loop
EXIT_DATAZERO :
		sub ecx, 1										//dec ecx making it equal -1
		jmp EXIT_ALL									//jump to exit to pass fail because of data length
EXIT_FOR :
		xor ecx, ecx									//if all loops completed clear ecx
EXIT_ALL :
		mov resulti, ecx								//return ecx
	}
	if (resulti == -1) {
		fprintf(stderr, "Error - Input file dataLength equals 0.\n\n");
	}
	else {
		fprintf(stderr, "File has been encrypted.\n\n");
	}
	return;
} // encryptData_01

void encryptData_02(char* data, int datalength)
{
	int resulti = 0;
	int bitLoop = 0;

	__asm
	{
		//Starting_index
		xor eax, eax                					//Clearing eax
		xor ecx, ecx									//Clearing ecx
		mov esi, gptrPasswordHash   					//moving gptrPasswordHash into esi
		mov al, byte ptr[esi] 					//moving gPasswordHas[0+round*4] into al
		shl ax, 8		    							//equivlent to multiplying by 256, shifting left is like multipling my power of two
		mov cl, byte ptr[esi + 1] 				// gPasswordHash[1+round*4] into cl
		add ax, cx		    							//Starting_index[round] = gPasswordHash[0+round*4] * 256 + gPasswordHash[1+round*4];
														//Not sure if this is correct
														//index = Starting_index[round];
		//mov [index], eax	
		//xor eax, eax				

		xor ebx, ebx		    						//This will be for dataLength
		xor ecx, ecx		    						//This will be for x or loop control
		cmp datalength, 0										//checking length of data
														//if ebx equals 0 then jump to exit_datazero
		jbe EXIT_DATAZERO
		mov edi, data									//putting data into edi
		mov esi, gptrKey								//putting gkey into esi
LOOP_FOR :
		//data[x] = data[x] ^ gkey[index] xoring each byte of data
		mov dl, byte ptr[edi + ecx]						//gets data[x] into al
		xor dl, byte ptr[esi + eax]						//data[x] ^ gkey[index] in al
		mov byte ptr[edi + ecx], dl						//putting al back into data[x]

		//#A - CodeTable Swap
		mov dl, byte ptr[edi + ecx]						//gets data[x] into dl
		mov bl, dl										//getting displacement from dl
		mov dl, byte ptr[gEncodeTable + ebx]			//Codetable swap using bl as displacement
		xor ebx, ebx									//Clearing ebx for next loop
		mov byte ptr[edi + ecx], dl						//putting dl back into data[x]

		//#E - Rotate 3 Bits Left
		rol dl, 3										//Rotate left 3 bits
		mov byte ptr[edi + ecx], dl						//putting dl back into data[x]
		add bitLoop, 8									//Setting loop controll for reversing bit order

		//#B - Reverse Bit Order
LOOP_BITORDER:
		rcr dl, 1										//Rotating bit out of dl
		rcl bl, 1										//Rotating that bit into bl
		dec bitLoop										//decrementing bitLoop
		cmp bitLoop, 0									//checking for loop exit
		jne LOOP_BITORDER								//Loop if not equal to zero
		mov dl, bl										//Move reversed bits back into dl
		xor ebx, ebx									//clearing ebx for later use
		mov byte ptr[edi + ecx], dl						//putting dl back into data[x]

		//#D - Invert Bits 1,5, and 6
		xor dl, 98										//invert bits 1,5,6

		//#C - Nibble Rotate Left 1

LOOP_NOCARRY:
		mov bl, dl										//copy byte into bl
		and dl, 0xF0									//isolate nibble
		shl dl, 1										//shift out a bit
		jc NIBBLE_CARRY									//if carry set then jump

		jmp LOOP_LOWERNIBBLE							//jump over if carry not set

NIBBLE_CARRY :
		or dl, 16										//add 1 on nibble

LOOP_LOWERNIBBLE:
		and bl, 0xF										//isolate nibble
		shl bl, 1										//shift nibble left 1

		test bl, 0x10									//test the shifted bit
		jz NIBBLE_ZERO									//if zero then jump

		or bl, 1										//if not add 1 to nibble
NIBBLE_ZERO:
		and bl, 0x0F									//isolate nibble
		or dl, bl										//combine nibbles

		mov byte ptr[edi + ecx], dl						//putting dl back into data[x]

		add ecx, 1 										//increment ecx(loop control) by 1
		cmp ecx, datalength								//compare x to datalength
		je EXIT_FOR										//if greater than, exit for
		jmp LOOP_FOR									//if not, jump back to start of loop
EXIT_DATAZERO :
		sub ecx, 1										//dec ecx making it equal -1
		jmp EXIT_ALL									//jump to exit to pass fail because of data length
EXIT_FOR :
		xor ecx, ecx									//if all loops completed clear ecx
EXIT_ALL :
		mov resulti, ecx								//return ecx
	}
	if (resulti == -1) {
		fprintf(stderr, "Error - Input file dataLength equals 0.\n\n");
	}
	else {
		fprintf(stderr, "File has been encrypted.\n\n");
	}
	return;
} // encryptData_02

void encryptData_03(char* data, int datalength)
{
	int resulti = 0;
	int bitLoop = 0;
	int rounds = 0;
	int index;
	int hop;
	__asm
	{
LOOP_ROUNDS:
		xor eax, eax                					//Clearing eax
		xor ecx, ecx									//Clearing ecx
		xor ebx, ebx
		mov ebx, [rounds]
		shl ebx, 2
		mov esi, gptrPasswordHash   					//moving gptrPasswordHash into esi
		mov al, byte ptr[esi + ebx] 					//moving gPasswordHas[0+round*4] into al
		shl ax, 8		    							//equivlent to multiplying by 256, shifting left is like multipling my power of two
		mov cl, byte ptr[esi + ebx + 1] 				// gPasswordHash[1+round*4] into cl
		add ax, cx		    							//Starting_index[round] = gPasswordHash[0+round*4] * 256 + gPasswordHash[1+round*4];
														//Not sure if this is correct
														//index = Starting_index[round];
		mov [index], eax
		xor eax, eax
		xor ecx, ecx


		mov al, byte ptr[esi + ebx + 2] 				//moving gPasswordHas[2+round*4] into al
		shl ax, 8		    							//equivlent to multiplying by 256, shifting left is like multipling my power of two
		mov cl, byte ptr[esi + ebx + 3] 				// gPasswordHash[3+round*4] into cl
		add ax, cx		    							//hop_count[round] = gPasswordHash[2+round*4] * 256 + gPasswordHash[3+round*4];

		cmp eax, 0
		je LOOP_HOPCOUNT

		jmp LOOP_HOPCOUNTJUMP
LOOP_HOPCOUNT:
		mov eax, 0xFFFF

LOOP_HOPCOUNTJUMP:
		mov [hop], eax
		xor ebx, ebx		    						//This will be for dataLength
		xor ecx, ecx		    						//This will be for x or loop control
		cmp datalength, 0								//checking length of data
														//if ebx equals 0 then jump to exit_datazero
		jbe EXIT_DATAZERO
		mov edi, data									//putting data into edi
		mov esi, gptrKey								//putting gkey into esi
LOOP_FOR :
		//data[x] = data[x] ^ gkey[index] xoring each byte of data
		mov eax, [index]
		mov dl, byte ptr[edi + ecx]						//gets data[x] into al
		xor dl, byte ptr[esi + eax]						//data[x] ^ gkey[index] in al
		mov byte ptr[edi + ecx], dl						//putting al back into data[x]

		//index = index + hop_count[round]
		mov eax, [hop]
		add eax, [index]
		cmp eax, 65537
		jge LOOP_GREATER

		jmp LOOP_LESS
LOOP_GREATER:
		sub eax, 65537
LOOP_LESS:
		mov [index], eax
		//#A - CodeTable Swap
		mov dl, byte ptr[edi + ecx]						//gets data[x] into dl
		mov bl, dl										//getting displacement from dl
		mov dl, byte ptr[gEncodeTable + ebx]			//Codetable swap using bl as displacement
		xor ebx, ebx									//Clearing ebx for next loop
		mov byte ptr[edi + ecx], dl						//putting dl back into data[x]

		//#E - Rotate 3 Bits Left
		rol dl, 3										//Rotate left 3 bits
		mov byte ptr[edi + ecx], dl						//putting dl back into data[x]
		add bitLoop, 8									//Setting loop controll for reversing bit order

		//#B - Reverse Bit Order
LOOP_BITORDER:
		rcr dl, 1										//Rotating bit out of dl
		rcl bl, 1										//Rotating that bit into bl
		dec bitLoop										//decrementing bitLoop
		cmp bitLoop, 0									//checking for loop exit
		jne LOOP_BITORDER								//Loop if not equal to zero
		mov dl, bl										//Move reversed bits back into dl
		xor ebx, ebx									//clearing ebx for later use
		mov byte ptr[edi + ecx], dl						//putting dl back into data[x]

		//#D - Invert Bits 1,5, and 6
		xor dl, 98										//invert bits 1,5,6

		//#C - Nibble Rotate Left 1

LOOP_NOCARRY:
		mov bl, dl										//copy byte into bl
		and dl, 0xF0									//isolate nibble
		shl dl, 1										//shift out a bit
		jc NIBBLE_CARRY									//if carry set then jump

		jmp LOOP_LOWERNIBBLE							//jump over if carry not set

NIBBLE_CARRY :
		or dl, 16										//add 1 on nibble

LOOP_LOWERNIBBLE :
		and bl, 0xF										//isolate nibble
		shl bl, 1										//shift nibble left 1

		test bl, 0x10									//test the shifted bit
		jz NIBBLE_ZERO									//if zero then jump

		or bl, 1										//if not add 1 to nibble
NIBBLE_ZERO:
		and bl, 0x0F									//isolate nibble
		or dl, bl										//combine nibbles

		mov byte ptr[edi + ecx], dl						//putting dl back into data[x]

		add ecx, 1 										//increment ecx(loop control) by 1
		cmp ecx, datalength								//compare x to datalength
		je EXIT_FOR										//if greater than, exit for
		jmp LOOP_FOR									//if not, jump back to start of loop
EXIT_DATAZERO :
		sub ecx, 1										//dec ecx making it equal -1
		jmp EXIT_ALL									//jump to exit to pass fail because of data length
EXIT_FOR :
		xor ecx, ecx									//if all loops completed clear ecx
		mov ecx, [gNumRounds]
		inc rounds
		cmp rounds, ecx
		jl LOOP_ROUNDS

		xor ecx, ecx
EXIT_ALL :
		mov resulti, ecx								//return ecx
	}
	if (resulti == -1) {
		fprintf(stderr, "Error - Input file dataLength equals 0.\n\n");
	}
	else {
		fprintf(stderr, "File has been encrypted.\n\n");
	}
	return;
} // encryptData_02

