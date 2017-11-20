/*
Kyle O'Neill
KO217135
Rahul Singh
Friday 11:30
This program allows a user to print a list of the variable and flow control labels
used in a MAL program. It accepts an input MAL file as well as a flag to indicate
which label list is desired. The flags are -v -f and -b, where -v is for variable labels,
-f is for flow control labels, and -b is for both. The input format is:
ProgramName -flag inputFile outputFile
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct Node{ //Creates a struct object which will be used to create a linked list
	char text[255];
	struct Node *next;
}Node;

typedef struct IdentifierStruct//Creates a struct object which will hold data about MAL labels
{
	char identifier[20];
	Node* uses;
}IdentifierStruct;

IdentifierStruct identifierArray[100];//declares an array for variable labels
IdentifierStruct flowControlArray[100];//declares an array for flow control labels

//NODE FUNCTIONS---------------------------------------------------------------------

Node* createNode(char* text, Node* next)//Creates a new node object. Takes an input of a string and a pointer to a node
{
	Node* nextNode = (Node*)malloc(sizeof(Node));//Allocates memory for a node and verifies that the node was created
	if(nextNode == NULL){
		printf("Error creating a new node.");
		fflush(stdout);
		exit(0);
	}
	strcpy(nextNode->text, text);//Places the input string and pointer into our node object
	nextNode->next = next;
	return nextNode;
}

Node* returnTail(Node* head) //Returns the tail node (the final node) of our linked list
{
	if(head == NULL)//If the list is empty, return null
		return NULL;
	Node* current = head;
	while(current->next != NULL){//Iterates through linked list until the final node is reached, and returns it
		current = current->next;	
	}
	return current;
}
	
void pushBack(Node **head, char* str)//Creates a new tail node.
{
	Node* newTail = createNode(str, NULL);
	if(*head == NULL)//Sets the new node as the head if there is no current head node
	{
		*head = newTail;
	}
	else
	{
		Node* currentTail = returnTail(*head);
		currentTail->next = newTail;//The old tail node must be linked to the new tail node
	}
}

void prn(Node *head, FILE *file){//Prints a linked list into output file.
	if(head == NULL){
		return;
	}
	Node* current = head;
	while(current!= NULL){//
		fprintf(file,"%s", current->text);
		current = current->next;
	}
}

//Helper functions------------------------------------------------------------------------

//Function iterates through a file. Returns true if an input string is found in the file,
//else returns false
int getLineAndFind(FILE *file, char *buffer, const char *target)
{
	char* token;
	if(fgets(buffer, 255, file) == NULL)//Iterate a line
	{
		return 1;
	}
	token = strtok(buffer, " \t\r\n");//Tokenize the line
	while(token != NULL)//Check to see what the token contains, returns 0 if it does not contain the target value
	{
		if(!strncmp(token, target, strlen(target)))//strncmp and strlen here are used to prevent the target from being misread if a newline comes after them
		{
			return 1;
		}
		else
			token = strtok(NULL, " \t\r\n");
	}
	return 0;
}

//Reads through a file and stores each encountered variable label in our struct object array
//Returns the size of the array
int readVariables(const char* fileName)
{
	FILE *tempFile;
	char *tempWord;
	int i = 0;
	char currentLine[255];
	tempFile = fopen(fileName, "r");
	while(!getLineAndFind(tempFile, currentLine, ".data"));//Skips to the ".data" line of MAL program
	while(!getLineAndFind(tempFile, currentLine, ".text"))//Reads each variable declaration line until ".text" is reached
	{
		//If loop filters out empty lines and comment lines
		if((*currentLine) && (*currentLine != '#') && (*currentLine != '\r') && (*currentLine != '\n'))
		{
			tempWord = strtok(currentLine, ":");
			strcpy(identifierArray[i].identifier, tempWord);
			identifierArray[i].uses = NULL;
			i++;
		}
	}
	fclose(tempFile);
	return i;
}

//Reads through a file and stores each encountered flow control label in our struct object array
//Returns the size of the array
int readFlowControl(const char* fileName)
{
	FILE *tempFile;
	char *tempWord;
	int i = 0;
	char currentLine[255];
	tempFile = fopen(fileName, "r");
	while(!getLineAndFind(tempFile, currentLine, ".text"));//Skips to the ".text" line of MAL program
	while((fgets(currentLine, 255, tempFile)) != NULL)//Reads each line until the program ends
	{
		if(strchr(currentLine, ':'))
		{
			tempWord = strtok(currentLine, ":");
			strcpy(flowControlArray[i].identifier, tempWord);
			flowControlArray[i].uses = NULL;
			i++;
		}
	}
	fclose(tempFile);
	return i;
}

//Along with prn function, prints linked list into output file
void printList(IdentifierStruct labels[100], int arrSize, const char* labelType, FILE *file)
{
	int i = 0;
	for(i = 0; i < arrSize; i++)
	{
		fprintf(file,"%s ID -%s-\n", labelType, labels[i].identifier);
		prn(labels[i].uses, file);
	}
}

//Locates when a variable label or flow control label is used. Stores the line
//they're used on in a linked list
void findVariableUse(const char* fileName,IdentifierStruct labels[100], int arrSize)
{
	FILE *tempFile;
	char currentLine[255], tempLine[255], *token;
	int i;
	tempFile = fopen(fileName, "r");
	while((strstr(fgets(currentLine,255,tempFile),".text"))==NULL);//Skips to ".text" line to read program
	while((fgets(currentLine,255,tempFile))!=NULL)//Read until end of file
	{
		//if loop makes sure line being read isn't empty or a comment
		if((*currentLine) && (*currentLine != '#') && (*currentLine != '\r') && (*currentLine != '\n'))
		{
			strcpy(tempLine, currentLine);
			token = strtok(tempLine, " \t\r\n,");//tokenize current line
			while(token != NULL && *token != '#' && *token != '.')//Make sure token is not a comment
			{
				for(i = 0; i < arrSize; i++)
				{
					if(!strcmp(labels[i].identifier, token))//compares token to list of labels
					{
						pushBack(&labels[i].uses, currentLine);//If they match, adds the line the token came from into linked list
					}
				}
				token = strtok(NULL, " \t\r\n,");
			}
		}
	}
}

//Main-------------------------------------------------
int main (int argc, char *argv[])
{
	FILE *inputFile, *outputFile;
	int variablesArraySize;
	int flowControlArraySize;
	
	if(argc < 3)
	{
		fprintf(stderr, "Missing inputs.\n");
		fflush(stdout);
		exit(1);
	}
	if ((inputFile = fopen(argv[2], "r")) == NULL){//Verifies input file exists
		fprintf(stderr,"Can't open %s.\n", argv[2]);
		fflush(stdout);
		exit(1);
	}
	fclose(inputFile);
	if ((outputFile = fopen(argv[3], "w")) == NULL) {//Verifies output file can be opened
		fprintf(stderr,"Can't open %s.\n", argv[3]);
		fflush(stdout);
		exit(1);
	}
	
	variablesArraySize = readVariables(argv[2]);//Creates variable label array
	flowControlArraySize = readFlowControl(argv[2]);//Creates flow control label array
	
	if(!strcmp(argv[1],"-v"))//Creates an a list of variable labels and saves it to the output file.
	{
		findVariableUse(argv[2], identifierArray, variablesArraySize);
		printList(identifierArray, variablesArraySize, "Variable", outputFile);
	}
	else if(!strcmp(argv[1],"-f"))//Creates an a list of flow control labels and saves it to the output file.
	{
		findVariableUse(argv[2], flowControlArray, flowControlArraySize);
		printList(flowControlArray, flowControlArraySize, "Flow Control", outputFile);
	}
	else if(!strcmp(argv[1],"-b"))//Performs the operations of the -v and -f flags
	{
		findVariableUse(argv[2], identifierArray, variablesArraySize);
		findVariableUse(argv[2], flowControlArray, flowControlArraySize);
		printList(identifierArray, variablesArraySize, "Variable", outputFile);
		printList(flowControlArray, flowControlArraySize, "Flow Control", outputFile);
	}
	else
	{
		fprintf(stderr, "Invalid flag.\n");
	}
	fclose(outputFile);
	return 0;
}