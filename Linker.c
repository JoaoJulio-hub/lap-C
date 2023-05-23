/*
max width = 100 columns
tab = 4 spaces
01234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789
*/

/*	Linguagens e Ambientes de Programaco - Projeto de 2022/2023

	Let's write a linker

 AUTHORS IDENTIFICATION
	Student 1: 61610, Joao Julio
	Student 2: 62942, Rodrigo Freitas

Comments:

O codigo esta confuso, ja no final percebemos que este nao era a melhor maneira de encarar o problema...
Mas ja era tarde para reverter a situacao, de qualquer modo comentamos o melhor para perceber a maneira como "atacamos" o projeto.

 Place here the names and numbers of the authors, plus some comments, as
Ā asked in the listing of the project. Do not deliver an anonymous file with
 unknown authors.
*/

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#define KIND_VARIABLE		'V'
#define KIND_FUNCTION		'F'

#define STATUS_PRIVATE		'P'
#define STATUS_PUBLIC		'X'
#define STATUS_UNDEFINED	'U'
#define STATUS_ABSOLUTE		'A'
#define STATUS_ERROR		'E'

#define MAX_NAME   64
#define MAX_LINE   1024

#define isInstruction(i)	((i) >= 0)
#define isNameRef(i)		((i) < 0)

#define MAX_FILES 10

typedef char Line[MAX_LINE];

typedef int EntryNumber;
typedef char Kind;
typedef char Status;
typedef int Address;
typedef char Name[MAX_NAME+1];
typedef int Instruction;

typedef struct {
	EntryNumber number;
	Kind kind;
	Status status;
	Address address;
	Name name;
} NameEntry; /*Estrutura que representa uma NameEntry*/

typedef struct {
	NameEntry *variables;
	NameEntry *functions;
	Instruction *code;
	int sizeVariables;
	int sizeFunctions;
	int sizeCode;
} File; /*Estrutura para guardar a informacao de um ficheiro:
variaveis, funcoes e codigo*/

typedef struct {
	Name name;
	Status status;
	int visited;
	int nrOccurences;
} PublicNameEntry; /*Estrutura para guardar numeros de ocorrencias e visitas
de variaveis und e public*/

typedef struct {
	Status status;
	Address address;
	Name name;
	int fileNumber;
} GlobalAdressEntry; /*Estrutura para guardar variaveis globais e o seu ficheiro
 respetivo*/

typedef struct {
    EntryNumber number;
    Kind kind;
    Status status;
    Address address;
    Name name;
    int fileNumber;
    Status lastStatus;
} FinalNameEntry; /*Estrutura para guardar as Name Entries finais*/

File files[MAX_FILES]; /*Apontador para estruturas file*/
int sizeOfFiles = 0;

int sizeOfPubAndUndVar = 0;
int sizeOfPubAndUndFunc = 0;
PublicNameEntry *pubAndUndVar, *pubAndUndFunc; /*Apontadores que guardam o
conjunto de variaveis e funcpes public e und*/

int globalAddressFuncSize = 0;
GlobalAdressEntry *globalAddressFunc; /*Apontador que guarda o
conjunto dos enderecos de todas as funcoes de um programa*/

int finalCodeSize = 0;
int finalNameEntriesSize = 0;
FinalNameEntry *finalNameEntries; /*Apontador que guarda as
name entries finais do programa*/
Instruction *finalCode;/*Apontador que guarda as
instrucoes/codigo final do programa*/

/*
	retorna o numero de ocorrencias de uma funçao publica com nome "name".
*/
int getNumberOfFuncPublics(Name name) {
	for(int i = 0; i < sizeOfPubAndUndFunc; i++) {
		if(strcmp(pubAndUndFunc[i].name, name) == 0 && pubAndUndFunc[i].status == STATUS_PUBLIC) {
			return pubAndUndFunc[i].nrOccurences;
		}
	}
	return -1;
}

/*
	retorna o numero de ocorrencias de uma variavel publica com nome "name".
*/
int getNumberOfVarPublics(Name name) {
	for(int i = 0; i < sizeOfPubAndUndVar; i++) {
		if(strcmp(pubAndUndVar[i].name, name) == 0 && pubAndUndVar[i].status == STATUS_PUBLIC) {
			return pubAndUndVar[i].nrOccurences;
		}
	}
	return -1;
}

/*
	retorna o numero de visitas a uma funçao publica com nome "name".
*/
int getNumberOfFuncPublicsVisited(Name name) {
	for(int i = 0; i < sizeOfPubAndUndFunc; i++) {
		if(strcmp(pubAndUndFunc[i].name, name) == 0 && pubAndUndFunc[i].status == STATUS_PUBLIC) {
			return pubAndUndFunc[i].visited;
		}
	}
	return -1;
}

/*
	retorna o numero de visitas a uma variavel publica com nome "name".
*/
int getNumberOfVarPublicsVisited(Name name) {
	for(int i = 0; i < sizeOfPubAndUndVar; i++) {
		if(strcmp(pubAndUndVar[i].name, name) == 0 && pubAndUndVar[i].status == STATUS_PUBLIC) {
			return pubAndUndVar[i].visited;
		}
	}
	return -1;
}

/*
	retorna o numero de visitas a uma funçao indefinida com nome "name".
*/
int getNumberOfFuncUndVisited(Name name) {
	for(int i = 0; i < sizeOfPubAndUndFunc; i++) {
		if(strcmp(pubAndUndFunc[i].name, name) == 0 && pubAndUndFunc[i].status == STATUS_UNDEFINED) {
			return pubAndUndFunc[i].visited;
		}
	}
	return -1;
}

/*
	retorna o numero de visitas a uma variavel indefinida com nome "name".
*/
int getNumberOfVarUndVisited(Name name) {
	for(int i = 0; i < sizeOfPubAndUndVar; i++) {
		if(strcmp(pubAndUndVar[i].name, name) == 0 && pubAndUndVar[i].status == STATUS_UNDEFINED) {
			return pubAndUndVar[i].visited;
		}
	}
	return -1;
}

/*
	verifica se existe uma funcao publica com nome "name".
*/
int hasPublicFunc(Name name) {
	for(int i = 0; i < sizeOfPubAndUndFunc; i++) {
		if(strcmp(pubAndUndFunc[i].name, name) == 0 && pubAndUndFunc[i].status == STATUS_PUBLIC) {
			return 1;
		}
	}
	return -1;
}

/*
	verifica se existe uma variavel indefinida com nome "name".
*/
int hasUndVar(Name name) {
	for(int i = 0; i < sizeOfPubAndUndVar; i++) {
		if(strcmp(pubAndUndVar[i].name, name) == 0 && pubAndUndVar[i].status == STATUS_UNDEFINED) {
			return 1;
		}
	}
	return -1;
}

/*
	verifica se existe uma funcao indefinida com nome "name".
*/
int hasUndFunc(Name name) {
	for(int i = 0; i < sizeOfPubAndUndFunc; i++) {
		if(strcmp(pubAndUndFunc[i].name, name) == 0 && pubAndUndFunc[i].status == STATUS_UNDEFINED) {
			return 1;
		}
	}
	return -1;
}

/*
	verifica se existe uma variavel publica com nome "name".
*/
int hasPublicVar(Name name) {
	for(int i = 0; i < sizeOfPubAndUndVar; i++) {
		if(strcmp(pubAndUndVar[i].name, name) == 0 && pubAndUndVar[i].status == STATUS_PUBLIC) {
			return 1;
		}
	}
	return -1;
}

/*
	Incrementa o numero de visitas a uma variavel publica com nome "name".
*/
void incrementPublicVarVisited(Name name) {
	for(int i = 0; i < sizeOfPubAndUndVar; i++) {
		if(strcmp(pubAndUndVar[i].name, name) == 0 && pubAndUndVar[i].status == STATUS_PUBLIC) {
			pubAndUndVar[i].visited += 1;
			return;
		}
	}
}

/*
	Incrementa o numero de visitas a uma funcao publica com nome "name".
*/
void incrementPublicFuncVisited(Name name) {
	for(int i = 0; i < sizeOfPubAndUndFunc; i++) {
		if(strcmp(pubAndUndFunc[i].name, name) == 0 && pubAndUndFunc[i].status == STATUS_PUBLIC) {
			pubAndUndFunc[i].visited += 1;
			return;
		}
	}
}

/*
	Incrementa o numero de visitas a uma variavel indefinida com nome "name".
*/
void incrementUndVarVisited(Name name) {
	for(int i = 0; i < sizeOfPubAndUndVar; i++) {
		if(strcmp(pubAndUndVar[i].name, name) == 0 && pubAndUndVar[i].status == STATUS_UNDEFINED) {
			pubAndUndVar[i].visited += 1;
			return;
		}
	}
}

/*
	Incrementa o numero de visitas a uma funcao indefinida com nome "name".
*/
void incrementUndFuncVisited(Name name) {
	for(int i = 0; i < sizeOfPubAndUndFunc; i++) {
		if(strcmp(pubAndUndFunc[i].name, name) == 0 && pubAndUndFunc[i].status == STATUS_UNDEFINED) {
			pubAndUndFunc[i].visited += 1;
			return;
		}
	}
}

/*
	Incrementa o numero de ocurrencias de uma variavel publica com nome "name".
*/
void incrementPublicVarOccurences(Name name) {
	for(int i = 0; i < sizeOfPubAndUndVar; i++) {
		if(strcmp(pubAndUndVar[i].name, name) == 0 && pubAndUndVar[i].status == STATUS_PUBLIC) {
			pubAndUndVar[i].nrOccurences += 1;
			return;
		}
	}
}

/*
	Incrementa o numero de ocurrencias de uma funcao publica com nome "name".
*/
void incrementPublicFuncOccurences(Name name) {
	for(int i = 0; i < sizeOfPubAndUndFunc; i++) {
		if(strcmp(pubAndUndFunc[i].name, name) == 0 && pubAndUndFunc[i].status == STATUS_PUBLIC) {
			pubAndUndFunc[i].nrOccurences += 1;
			return;
		}
	}
}

/*
	Adicionar uma variavel ao ficheiro correspondente.
*/
void addNameEntryVarToObj(int fileNumber, EntryNumber number, Status status, Address address, Name name) {
		int sizeVariables = files[fileNumber].sizeVariables;
		if(sizeVariables == 0) {
			files[fileNumber].variables = (NameEntry *) malloc(sizeof(NameEntry));
		} else {
			files[fileNumber].variables = (NameEntry *) realloc(files[fileNumber].variables, (sizeVariables + 1) * sizeof(NameEntry));
		}
		// Adicionar a variavel ao ficheiro
		files[fileNumber].variables[sizeVariables].number = number;
		files[fileNumber].variables[sizeVariables].kind = KIND_VARIABLE;
		files[fileNumber].variables[sizeVariables].status = status;
		files[fileNumber].variables[sizeVariables].address = address;
		strcpy(files[fileNumber].variables[sizeVariables].name, name);
		files[fileNumber].sizeVariables++;
		if(status == STATUS_PUBLIC || status == STATUS_UNDEFINED) {
			// Se for publica ou undefined va,os guardar a sua ocorrencia
			if(sizeOfPubAndUndVar == 0) {
				pubAndUndVar = (PublicNameEntry *) malloc(sizeof(PublicNameEntry));
				strcpy(pubAndUndVar[sizeOfPubAndUndVar].name, name);
				pubAndUndVar[sizeOfPubAndUndVar].nrOccurences = 1;
				pubAndUndVar[sizeOfPubAndUndVar].visited = 0;
				pubAndUndVar[sizeOfPubAndUndVar].status = status;
				sizeOfPubAndUndVar++;
			} else {
					if(status == STATUS_PUBLIC) {
						// Caso ainda nao tenha existido uma ocorrencia, guardamos
						if(hasPublicVar(name) == -1) {
							pubAndUndVar = (PublicNameEntry *) realloc(pubAndUndVar, (sizeOfPubAndUndVar + 1) * sizeof(PublicNameEntry));
							strcpy(pubAndUndVar[sizeOfPubAndUndVar].name, name);
							pubAndUndVar[sizeOfPubAndUndVar].nrOccurences = 1;
							pubAndUndVar[sizeOfPubAndUndVar].visited = 0;
							pubAndUndVar[sizeOfPubAndUndVar].status = status;
							sizeOfPubAndUndVar++;
						} else {
							// Caso contrario incrementamos a ocorrencia
						    incrementPublicVarOccurences(name);
						}
					} else if(status == STATUS_UNDEFINED)  {
						// No caso das indefinidas guardas apenas a primeira ocorrencia
						if(hasUndVar(name) == -1) {
							pubAndUndVar = (PublicNameEntry *) realloc(pubAndUndVar, (sizeOfPubAndUndVar + 1) * sizeof(PublicNameEntry));
							strcpy(pubAndUndVar[sizeOfPubAndUndVar].name, name);
							pubAndUndVar[sizeOfPubAndUndVar].nrOccurences = 1;
							pubAndUndVar[sizeOfPubAndUndVar].visited = 0;
							pubAndUndVar[sizeOfPubAndUndVar].status = status;
							sizeOfPubAndUndVar++;
						}
				   }
			}
		}
}

/*
	Adicionar uma funcao ao ficheiro correspondente.
	A logica e a mesma da funcao anterior.
*/
void addNameEntryFuncToObj(int fileNumber, EntryNumber number, Status status, Address address, Name name) {
		int sizeFunctions = files[fileNumber].sizeFunctions;
		if(sizeFunctions == 0) {
			files[fileNumber].functions = (NameEntry *) malloc(sizeof(NameEntry));
		} else {
			files[fileNumber].functions = (NameEntry *) realloc(files[fileNumber].functions, (sizeFunctions + 1) * sizeof(NameEntry));
		}
		files[fileNumber].functions[sizeFunctions].number = number;
		files[fileNumber].functions[sizeFunctions].kind = KIND_FUNCTION;
		files[fileNumber].functions[sizeFunctions].status = status;
		files[fileNumber].functions[sizeFunctions].address = address;
		strcpy(files[fileNumber].functions[sizeFunctions].name, name);
		files[fileNumber].sizeFunctions++;
		if(status== STATUS_PUBLIC) {
			if(hasPublicFunc(name) == -1) {
				if(sizeOfPubAndUndFunc == 0) {
					pubAndUndFunc = (PublicNameEntry *) malloc(sizeof(PublicNameEntry));
				} else {
					pubAndUndFunc = (PublicNameEntry *) realloc(pubAndUndFunc, (sizeOfPubAndUndFunc + 1) * sizeof(PublicNameEntry));
				}
				pubAndUndFunc[sizeOfPubAndUndFunc].nrOccurences = 1;
				strcpy(pubAndUndFunc[sizeOfPubAndUndFunc].name, name);
				pubAndUndFunc[sizeOfPubAndUndFunc].visited = 0;
				pubAndUndFunc[sizeOfPubAndUndFunc].status = status;
				sizeOfPubAndUndFunc++;
			} else {
				incrementPublicFuncOccurences(name);
			}
		} else if(status== STATUS_UNDEFINED)  {
			if(hasUndFunc(name) == -1) {
				if(sizeOfPubAndUndFunc == 0) {
					pubAndUndFunc = (PublicNameEntry *) malloc(sizeof(PublicNameEntry));
				} else {
					pubAndUndFunc = (PublicNameEntry *) realloc(pubAndUndFunc, (sizeOfPubAndUndFunc + 1) * sizeof(PublicNameEntry));
				}
				strcpy(pubAndUndFunc[sizeOfPubAndUndFunc].name, name);
				pubAndUndFunc[sizeOfPubAndUndFunc].nrOccurences = 1;
				pubAndUndFunc[sizeOfPubAndUndFunc].visited = 0;
				pubAndUndFunc[sizeOfPubAndUndFunc].status = status;
				sizeOfPubAndUndFunc++;
			}
		}
}

/*
	Adicionar uma instrucao/codigo ao ficheiro correspondente.
*/
void addCodeToObj(int fileNumber, Instruction codeNumber) {
	int sizeCode = files[fileNumber].sizeCode;
	if(sizeCode == 0) {
		files[fileNumber].code = (Instruction *) malloc(sizeof(Instruction));
	} else {
		files[fileNumber].code = (Instruction *) realloc(files[fileNumber].code, (sizeCode + 1) * sizeof(Instruction));
	}
	files[fileNumber].code[sizeCode] = codeNumber;
	files[fileNumber].sizeCode++;
}

/*
	Guardar o endereco global de uma funcao.
*/
void setFunGlobalAddresses() {
	int start = 0;
	for(int i = 0; i < sizeOfFiles; i++) {
		NameEntry *functions = files[i].functions;
		int sizeFunctions = files[i].sizeFunctions;
		for(int j = 0; j < sizeFunctions ; j++) {
			/* Caso a funcao seja publica, para guardarmos o seu endereco esta
			 * nao pode ter ocorrido mais que uma vez (se nao existe um erro).
			 * Tambem podemos guardar enderecos de funcoes privadas, pois estas
			 * nunca dao problemas. Undefined dao por isso nao sao guardadas.
			 */
			if((functions[j].status == STATUS_PUBLIC && getNumberOfFuncPublics(functions[j].name) < 2)
					|| functions[j].status == STATUS_PRIVATE) {
					if(globalAddressFuncSize == 0) {
						globalAddressFunc = (GlobalAdressEntry *) malloc(sizeof(GlobalAdressEntry));
					} else {
						globalAddressFunc = (GlobalAdressEntry *) realloc(globalAddressFunc,
								(globalAddressFuncSize + 1) * sizeof(GlobalAdressEntry));
					}
					globalAddressFunc[globalAddressFuncSize].status = functions[j].status;
					globalAddressFunc[globalAddressFuncSize].address = start + (functions[j].address);
					strcpy(globalAddressFunc[globalAddressFuncSize].name, functions[j].name);
					globalAddressFunc[globalAddressFuncSize].fileNumber = i;
					globalAddressFuncSize++;
				}
	 }
	 start += files[i].sizeCode * 4; // Proximo ficheiro ira comecar aqui
 }
}

/*
	Obter o endereco global de uma funcao com um dado nome.
*/
int getGlobalFuncAddress(Name name) {
	for(int i = 0; i < globalAddressFuncSize; i++) {
		if(strcmp(globalAddressFunc[i].name, name) == 0) {
			return globalAddressFunc[i].address;
		}
	}
	return -1;
}

/*
	Obter o endereco global de uma funcao privada.
*/
int getPrivateFuncAddress(Name name, int fileNumber) {
	for(int i = 0; i < globalAddressFuncSize; i++) {
		if(strcmp(globalAddressFunc[i].name, name) == 0 && globalAddressFunc[i].fileNumber == fileNumber) {
			return globalAddressFunc[i].address;
		}
	}
	return -1;
}

/*
 * Colocar todas as names entries no ficheiro executavel
 */
void setFinalNameEntries() {
	int currentNumber = 1;
	int currentAddress = 0;
	for(int i = 0; i < sizeOfFiles; i++) {
		int sizeVariables = files[i].sizeVariables;
		int sizeFunctions = files[i].sizeFunctions;
		for(int j = 0; j < sizeVariables ; j++) {
			if(files[i].variables[j].status == STATUS_UNDEFINED || files[i].variables[j].status == STATUS_PUBLIC) {
				if(hasPublicVar(files[i].variables[j].name) == 1) {
					// Se tem uma variavel publica associada e apenas uma
					if(getNumberOfVarPublicsVisited(files[i].variables[j].name) < 1 && getNumberOfVarPublics(files[i].variables[j].name) < 2
					 && getNumberOfVarUndVisited(files[i].variables[j].name) < 1) {
						/* Verificar se a variavel nao foi "visitada" (tanto a publica como a und) e se nao existem vars
						publicas com o mesmo nome, em seguida adicionar ao array de vars do "executavel" com estado
						sucesso e endereco proviniente do array de vars com os enderecos "globais" */
						if(finalNameEntriesSize == 0){
						    finalNameEntries = (FinalNameEntry *) malloc(sizeof(FinalNameEntry));
						} else {
						    finalNameEntries = (FinalNameEntry *) realloc(finalNameEntries, (finalNameEntriesSize + 1) * sizeof(FinalNameEntry));
						}
						finalNameEntries[finalNameEntriesSize].number = currentNumber;
						finalNameEntries[finalNameEntriesSize].kind = KIND_VARIABLE;
						finalNameEntries[finalNameEntriesSize].status = STATUS_ABSOLUTE;
						finalNameEntries[finalNameEntriesSize].address = currentAddress;
						strcpy(finalNameEntries[finalNameEntriesSize].name, files[i].variables[j].name);
						finalNameEntries[finalNameEntriesSize].fileNumber = i;
						finalNameEntries[finalNameEntriesSize].lastStatus = files[i].variables[j].status;
						incrementPublicVarVisited(files[i].variables[j].name);
						incrementUndVarVisited(files[i].variables[j].name);
						currentNumber += 1;
						currentAddress += 4;
						finalNameEntriesSize++;
					} else if(getNumberOfVarPublicsVisited(files[i].variables[j].name) < 1 && getNumberOfVarPublics(files[i].variables[j].name)  >= 2
						&& getNumberOfVarUndVisited(files[i].variables[j].name) < 1) {
					 /* Verificar se uma variavel nao foi "visitada" (tanto a publica como a und) e se existem vars publicas com o mesmo nome, em seguida
						adicionar ao array de vars do "executavel" com estado erro e endereco proviniente do array de funcoes
						com os enderecos "globais" */
						if(finalNameEntriesSize == 0) {
						    finalNameEntries = (FinalNameEntry *) malloc(sizeof(FinalNameEntry));
						} else {
						    finalNameEntries = (FinalNameEntry *) realloc(finalNameEntries, (finalNameEntriesSize + 1) * sizeof(FinalNameEntry));
						}
						finalNameEntries[finalNameEntriesSize].number = currentNumber;
						finalNameEntries[finalNameEntriesSize].kind = KIND_VARIABLE;
						finalNameEntries[finalNameEntriesSize].status = STATUS_ERROR;
						finalNameEntries[finalNameEntriesSize].address = 0;
						strcpy(finalNameEntries[finalNameEntriesSize].name, files[i].variables[j].name);
						finalNameEntries[finalNameEntriesSize].fileNumber = i;
						finalNameEntries[finalNameEntriesSize].lastStatus = files[i].variables[j].status;
						incrementPublicVarVisited(files[i].variables[j].name);
						incrementUndVarVisited(files[i].variables[j].name);
						currentNumber += 1;
						finalNameEntriesSize++;
					}
				} else {
				/* significa que e uma undefined cuja a funcao global nao existe, logo colocar com endereco erro,
				 *  ou seja, com endereco igual a 0 e fazer visitado mais um no endereco de arrays que nao existem*/
						if(getNumberOfVarUndVisited(files[i].variables[j].name) < 1) {
							if(finalNameEntriesSize == 0){
							    finalNameEntries = (FinalNameEntry *) malloc(sizeof(FinalNameEntry));
							} else {
							    finalNameEntries = (FinalNameEntry *) realloc(finalNameEntries, (finalNameEntriesSize + 1) * sizeof(FinalNameEntry));
							}
							finalNameEntries[finalNameEntriesSize].number = currentNumber;
							finalNameEntries[finalNameEntriesSize].kind = KIND_VARIABLE;
							finalNameEntries[finalNameEntriesSize].status = STATUS_ERROR;
							finalNameEntries[finalNameEntriesSize].address = 0;
							finalNameEntries[finalNameEntriesSize].fileNumber = i;
							finalNameEntries[finalNameEntriesSize].lastStatus = files[i].variables[j].status;
							strcpy(finalNameEntries[finalNameEntriesSize].name, files[i].variables[j].name);
							incrementUndVarVisited(files[i].variables[j].name);
							currentNumber += 1;
							finalNameEntriesSize++;
						}
				}
			} else {
				// É privada, logo adicionar ao array de funcoes com endereco com sucesso, ou seja, com endereco igual a currentAddress
				if(finalNameEntriesSize == 0){
				    finalNameEntries = (FinalNameEntry *) malloc(sizeof(FinalNameEntry));
				} else {
				    finalNameEntries = (FinalNameEntry *) realloc(finalNameEntries, (finalNameEntriesSize + 1) * sizeof(FinalNameEntry));
				}
				finalNameEntries[finalNameEntriesSize].number = currentNumber;
				finalNameEntries[finalNameEntriesSize].kind = KIND_VARIABLE;
				finalNameEntries[finalNameEntriesSize].status = STATUS_ABSOLUTE;
				finalNameEntries[finalNameEntriesSize].address = currentAddress;
				finalNameEntries[finalNameEntriesSize].fileNumber = i;
				finalNameEntries[finalNameEntriesSize].lastStatus = files[i].variables[j].status;
				strcpy(finalNameEntries[finalNameEntriesSize].name, files[i].variables[j].name);
				currentNumber += 1;
				currentAddress += 4;
				finalNameEntriesSize++;
			}
		}
		for(int k = 0; k < sizeFunctions ; k++) {
			if(files[i].functions[k].status == STATUS_UNDEFINED || files[i].functions[k].status == STATUS_PUBLIC) { // Se for undefined
				if(hasPublicFunc(files[i].functions[k].name) == 1) {
					// Se tem uima funcao publica associada e uma so
					if(getNumberOfFuncPublicsVisited(files[i].functions[k].name) < 1 && getNumberOfFuncPublics(files[i].functions[k].name) < 2
					 && getNumberOfFuncUndVisited(files[i].functions[k].name) < 1) {
						/* Verificar se a funcao nao foi "visitada" (tanto a publica como a und) e se nao existem funcoes
						publicas com o mesmo nome, em seguida adicionar ao array de funcoes do "executavel" com estado
						sucesso e endereco proviniente do array de funcoes com os enderecos "globais" */
						if(finalNameEntriesSize == 0){
							finalNameEntries = (FinalNameEntry *) malloc(sizeof(FinalNameEntry));
						} else {
							finalNameEntries = (FinalNameEntry *) realloc(finalNameEntries, (finalNameEntriesSize + 1) * sizeof(FinalNameEntry));
						}
						finalNameEntries[finalNameEntriesSize].number = currentNumber;
						finalNameEntries[finalNameEntriesSize].kind = KIND_FUNCTION;
						finalNameEntries[finalNameEntriesSize].status = STATUS_ABSOLUTE;
						finalNameEntries[finalNameEntriesSize].address = getGlobalFuncAddress(files[i].functions[k].name);
						finalNameEntries[finalNameEntriesSize].lastStatus = files[i].functions[k].status;
						strcpy(finalNameEntries[finalNameEntriesSize].name, files[i].functions[k].name);
						finalNameEntries[finalNameEntriesSize].fileNumber = i;
						incrementPublicFuncVisited(files[i].functions[k].name);
						incrementUndFuncVisited(files[i].functions[k].name);
						currentNumber += 1;
						finalNameEntriesSize++;
					} else if(getNumberOfFuncPublicsVisited(files[i].functions[k].name) < 1 && getNumberOfFuncPublics(files[i].functions[k].name)  >= 2
						&& getNumberOfFuncUndVisited(files[i].functions[k].name) < 1) {
					 /* Verificar se a funcao nao foi "visitada" (tanto a publica como a und) e se existem funcoes publicas com o mesmo nome, em seguida
						adicionar ao array de funcoes do "executavel" com estado erro e endereco proviniente do array de funcoes
						com os enderecos "globais" */
						if(finalNameEntriesSize == 0){
							finalNameEntries = (FinalNameEntry *) malloc(sizeof(FinalNameEntry));
						} else {
							finalNameEntries = (FinalNameEntry *) realloc(finalNameEntries, (finalNameEntriesSize + 1) * sizeof(FinalNameEntry));
						}
						finalNameEntries[finalNameEntriesSize].number = currentNumber;
						finalNameEntries[finalNameEntriesSize].kind = KIND_FUNCTION;
						finalNameEntries[finalNameEntriesSize].status = STATUS_ERROR;
						finalNameEntries[finalNameEntriesSize].address = 0;
						finalNameEntries[finalNameEntriesSize].lastStatus = files[i].functions[k].status;
						strcpy(finalNameEntries[finalNameEntriesSize].name, files[i].functions[k].name);
						incrementPublicFuncVisited(files[i].functions[k].name);
						incrementUndFuncVisited(files[i].functions[k].name);
						finalNameEntries[finalNameEntriesSize].fileNumber = i;
						currentNumber += 1;
						finalNameEntriesSize++;
					}
				} else {
					// significa que e uma undefined cuja a funcao global nao existe, logo colocar com endereco erro, ou seja, com endereco igual a 0 e fazer visitado mais um no endereco de arrays que nao existem
					if(getNumberOfFuncUndVisited(files[i].functions[k].name) < 1) {
						if(finalNameEntriesSize == 0){
							finalNameEntries = (FinalNameEntry *) malloc(sizeof(FinalNameEntry));
						} else {
							finalNameEntries = (FinalNameEntry *) realloc(finalNameEntries, (finalNameEntriesSize + 1) * sizeof(FinalNameEntry));
						}
						finalNameEntries[finalNameEntriesSize].number = currentNumber;
						finalNameEntries[finalNameEntriesSize].kind = KIND_FUNCTION;
						finalNameEntries[finalNameEntriesSize].status = STATUS_ERROR;
						finalNameEntries[finalNameEntriesSize].address = 0;
						finalNameEntries[finalNameEntriesSize].lastStatus = files[i].functions[k].status;
						strcpy(finalNameEntries[finalNameEntriesSize].name, files[i].functions[k].name);
						incrementUndFuncVisited(files[i].functions[k].name);
						finalNameEntries[finalNameEntriesSize].fileNumber = i;
						currentNumber += 1;
						finalNameEntriesSize++;
					}
				}
			} else {
				// É privado, logo adicionar ao array de funcoes com endereco com sucesso, ou seja, com endereco igual a currentAddress
				if(finalNameEntriesSize == 0){
					finalNameEntries = (FinalNameEntry *) malloc(sizeof(FinalNameEntry));
				} else {
					finalNameEntries = (FinalNameEntry *) realloc(finalNameEntries, (finalNameEntriesSize + 1) * sizeof(FinalNameEntry));
				}
				finalNameEntries[finalNameEntriesSize].number = currentNumber;
				finalNameEntries[finalNameEntriesSize].kind = KIND_FUNCTION;
				finalNameEntries[finalNameEntriesSize].status = STATUS_ABSOLUTE;
				finalNameEntries[finalNameEntriesSize].address = getPrivateFuncAddress(files[i].functions[k].name, i);
				finalNameEntries[finalNameEntriesSize].lastStatus = files[i].functions[k].status;
				strcpy(finalNameEntries[finalNameEntriesSize].name, files[i].functions[k].name);
				finalNameEntries[finalNameEntriesSize].fileNumber = i;
				currentNumber += 1;
				finalNameEntriesSize++;
			}
		}
	}
}

/*
	Obter o endereco global de uma variavel com um dado nome.
*/
Address getGlobalVarAddress(Name name) {
	for(int i = 0; i < finalNameEntriesSize; i++) {
		if(finalNameEntries[i].kind == KIND_VARIABLE && (finalNameEntries[i].lastStatus == STATUS_PUBLIC ||
			finalNameEntries[i].lastStatus == STATUS_UNDEFINED) && strcmp(finalNameEntries[i].name, name) == 0) {
			return finalNameEntries[i].address;
		}
	}
	return -1;
}

/*
	Obter o endereco global de uma variavel privada.
*/
Address getPrivateVarAddress(Name name, int fileNumber) {
	for(int i = 0; i < finalNameEntriesSize; i++) {
		if(finalNameEntries[i].kind == KIND_VARIABLE && finalNameEntries[i].lastStatus == STATUS_PRIVATE
				&& finalNameEntries[i].fileNumber == fileNumber
				&& strcmp(finalNameEntries[i].name, name) == 0) {
			return finalNameEntries[i].address;
		}
	}
	return -1;
}

/*
 * Colocar todas as instrucoes/code no ficheiro executavel
 */
void setFinalCodes() {
  for(int i = 0; i < sizeOfFiles; i++) {
	int sizeCode = files[i].sizeCode;
    for(int j = 0; j < sizeCode ; j++) {
       if(isInstruction(files[i].code[j]) == 0) {
    	int trueValue = abs(files[i].code[j]) - 1; // localizacao no apontador de variaveis
		if(trueValue <= files[i].sizeVariables - 1) {
		 if(finalCodeSize == 0) {
			finalCode = (Instruction *) malloc(sizeof(Instruction));
		 } else {
			finalCode = (Instruction *) realloc(finalCode, (finalCodeSize + 1) * sizeof(Instruction));
		 }
		 /*
		  * Buscar o endereco global de uma variavel
		  */
		 if(files[i].variables[trueValue].status == STATUS_PRIVATE) {
			 Address a = getPrivateVarAddress(files[i].variables[trueValue].name, i);
			 finalCode[finalCodeSize] = a;
			 finalCodeSize++;
		 } else {
			 Address a = getGlobalVarAddress(files[i].variables[trueValue].name);
			 finalCode[finalCodeSize] = a;
			 finalCodeSize++;
		 }
		} else {
		  int modValue = trueValue - files[i].sizeVariables; // localizacao no apontador de funcoes
		  if(finalCodeSize == 0) {
			finalCode = (Instruction *) malloc(sizeof(Instruction));
		  } else {
			finalCode = (Instruction *) realloc(finalCode, (finalCodeSize + 1) * sizeof(Instruction));
		  }

		  /*
		   * Buscar o endereco global de uma funcao
		   */
		  if(files[i].functions[modValue].status == STATUS_PRIVATE) {
			 Address a = getPrivateFuncAddress(files[i].functions[modValue].name, i);
			 finalCode[finalCodeSize] = a;
			 finalCodeSize++;
		  } else {
			 Address a = getGlobalFuncAddress(files[i].functions[modValue].name);
			 finalCode[finalCodeSize] = a;
			 finalCodeSize++;
		  }
		}
	   } else {
			 // Adicionar ao ficheiro executavel se for instrucao/codigo
		   if(finalCodeSize == 0) {
		   			finalCode = (Instruction *) malloc(sizeof(Instruction));
		   } else {
				finalCode = (Instruction *) realloc(finalCode, (finalCodeSize + 1) * sizeof(Instruction));
		   }
		   finalCode[finalCodeSize] = files[i].code[j];
		   finalCodeSize++;
	   }
  	  }
  	 }
}

/*
 * Ler o input
 */
void processFile(void){
 Line line;
 while(fgets(line, MAX_LINE, stdin) != NULL) {
   EntryNumber number;
   Kind kind;
   Status status;
   Address address;
   Name name;
   Instruction i;
   if(strcmp(line, "---\n") == 0) {
	sizeOfFiles++;
    processFile();
    break;
   } else if(sscanf(line, "%04d %c %c %04d %s", &number, &kind, &status, &address, name) == 5) {
	 int currentFile = sizeOfFiles - 1;
     if(kind==KIND_FUNCTION) {
       addNameEntryFuncToObj(currentFile, number, status, address, name);
     }
     else addNameEntryVarToObj(currentFile, number, status, address, name);
   } else if (sscanf(line, "%d", &i) == 1) {
	int currentFile = sizeOfFiles - 1;
    addCodeToObj(currentFile, i);
   } else if(strcmp(line, "\n") == 0) {
	   return;
   }
 }
 return;
}

/*
* Inicializar o linker
*/
void linker(void) {
 Line line;
 if(fgets(line, MAX_LINE, stdin) != NULL){
   if(strcmp(line, "---\n") == 0) {
	  sizeOfFiles++;
      processFile();
   }
 }
}

/*
* Libertar a memoria
*/
void free_memory() {
for(int i = 0; i < sizeOfFiles; i++) {
  free(files[i].functions);
  files[i].functions = NULL;
  free(files[i].variables);
  files[i].variables = NULL;
  free(files[i].code);
  files[i].code = NULL;
  free(files[i].code);
 }
 free(pubAndUndVar);
 free(pubAndUndFunc);
 free(globalAddressFunc);
 free(finalNameEntries);
 free(finalCode);
}

int main(void)
{
	linker();
	setFunGlobalAddresses();
	setFinalNameEntries();
	setFinalCodes();
	printf("+++\n");
	for(int j = 0; j < finalNameEntriesSize; j++) {
	 	printf("%04d %c %c %04d %s\n", finalNameEntries[j].number, finalNameEntries[j].kind, finalNameEntries[j].status,
	 			finalNameEntries[j].address, finalNameEntries[j].name);
	}
	for(int z = 0; z < finalCodeSize; z++) {
		printf("%04d\n", finalCode[z]);
	}

	free_memory();
	return 0;
}
