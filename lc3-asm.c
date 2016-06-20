/*
*
* by:    Jean-Philippe Legault
*       Mattias Shroer
*       Josh Tompkins
*/



/*
free little test file

;int x =10; int y = 15 ;
; int *ptrx = &x;
;*ptrx = * ptrx + 2 ;
; int *ptry = &y ;
;ptry = ptry ;

.ORIG 0X3000
LD r0, X
LEA R0, #11
ST R0, PTRX
loop
loop ADD R0,R2,#2
AND R5,R0,0x2
LEA R0, Y
ST R0, PTRY
LEA R0 PTRY
PUTS
HALT
X .FILL #10
Y .FILL #15
PTRX .FILL #0
PTRY .FILL #0
julio .STRINGZ "is my name"
cool .BLKW #4 #2
.END

***********************************/
/********************************************************************************************************************
cd into directory!
to compile copy this line: 

    clang translate.c 
    or
    gcc translate.c

to run with test file cd into directory :

    {output binaries name} {file to convert to hex} {args} 


{args}

-d          to run debugging mode                   
-v          to debug your .asm file 
-b          to get the binary format.


*****
all methods are implemented. i've added some error checked for LC3 code input
the output file is the same name as the input file except .hex is appended to it.


***********************************/


/*
                                                     :::::,,,,,,,,,,             
                                                  ::::::::,,,,,,,,,,,,        
                                                ::::::::::,,,,,,,,,,=+?+:       
                                              :~~~~77I?:::,,,,,,,,=????==?      
                                             ~~=+7:  ,I+::,,,,,,=???????~=+     
                                            ~=+7  =,, ?=,:,,,,,+?????????===    
                                           ~=+? 7 , =?? ::,,,=, ==+??????  +=  
                                          ~=+I7777II?= ::,,,+?:     ++????   =  
                                          ~+?7   ,,,,~~::,:????      ===++      
                                         ~=+I7  7I?+~~:::=?????                 
                                         ~=+?IIII?+=~~::=??????                 Nom nom nom...
                                        =~==+++++==~~::,    ~++     ???         
                            ~          =+~~~====~~~:::,               ?+   ? 
                      + +~:,~:         ++:~~~~~~~::::,                 I  II   
                    I:,    +:::    I=I=++::~::::::::, +???=         +   ?III+?? 
                     =:  =I+=:,,  ?: : ::::::::::::,,,,,???~      +III  ,,II??7 
                    ::::,    ~:,=:,   =: ,:::::::,,,,,,,,,,??   :?IIIII  ,,I+I7  
           :::::  ?,:         :::,,  :     :::::,,,,,,,,,,,,,?+???IIIII  ,,+77   
          ~,:   :I,,           =,: ~:       ,,,,,,,,,,,,,,,,,,,??IIII7I +=77I   
I=++=~   :,+:,  =, ,             ~:          ,,,,,,,,,,,,,,,,,,,,,?IIII==77I,   
,     ,:~?,    ::,,:                          ,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,    
~:,,   ?,,                                      ,,,,,,,,,,,,,,,,,,,,,,,,,,      
    ,::::                                         ,,,,,,,,,,,,,,,,,,,,,,        
                                                     ,,,,,,,,,,,,::::           
                                                            ,,                  
*/



#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

#define ADD     0x1000
#define AND     0x5000
#define BR      0X0000
#define BRN     0X0800
#define BRZ     0X0400
#define BRP     0X0200
#define BRNZ    0X0C00
#define BRZP    0X0600
#define BRNP    0X0A00
#define BRNZP   0X0E00
#define JMP     0XC000
#define JSR     0X4800
#define JSRR    0X4000
#define LD      0X2000
#define LDI     0XA000
#define LDR     0X6000
#define LEA     0xE000
#define NOT     0X903F
#define RET     0XC1C0
#define RTI     0X8000
#define ST      0x3000
#define STI     0XB000
#define STR     0X7000

#define PUTS    0xF022
#define HALT    0xF025

#define DR0     0x0000
#define DR1     0x0200
#define DR2     0x0400
#define DR3     0x0600
#define DR4     0x0800
#define DR5     0x0A00
#define DR6     0x0C00
#define DR7     0x0E00

#define SR0    0x0000
#define SR1    0x0040
#define SR2    0x0080
#define SR3    0x00C0
#define SR4    0x0100
#define SR5    0x0140
#define SR6    0x0180
#define SR7    0x01C0

#define imm5    0x0020

#define PCoffset    0x0001

#define wrong    0xBAAD

#define endME   0xDEAD

#define doNothing 0x1020

static bool debug = false;
static bool verbose = false;
static char *pointer[2048][10];
static int line[512];
static bool preHalt = true;
static int startAdress = 0;

static bool allInputOK = true;
static bool containsOrig = false;
static bool containsHalt = false;
static bool containsEnd = false;

// 512 lines max
// 10 token max
// 32 char tokean max.
static char code[2048][10][32];
static int sizeOfFile=0;


//skips comment,
void Tokenize(FILE *fileIn){
    int i = 0;
    int j = 0;
    int k = 0;

    char in = '\0';    
    bool freshLine = false;
    bool freshToken = false;
    bool munch = false;
    bool nestMe = false;
    
    while ((in = fgetc(fileIn)) != EOF){
        if(!munch && in == '"' && !nestMe){
            code[i][j++][k] = '\0';
            nestMe = true;
            k=0;
            code[i][j][k++] = in;
        }
        
        else if(!munch && in == '"' && nestMe){
            nestMe = false;
            freshToken = true;
            code[i][j][k++] = in;
            code[i][j][k] = '\0';
        }
        
        else if(nestMe && !munch &&((in >= 'A' && in <= 'Z') || (in >= 'a' && in <= 'z') || (in >= '0' && in <= '9') || in == '-' || in == '.' || in == '#' || in == ' ')){
            code[i][j][k++] = in;
        }
        
        else if(!nestMe && !munch &&((in >= 'A' && in <= 'Z') || (in >= 'a' && in <= 'z') || (in >= '0' && in <= '9') || in == '-' || in == '.' || in == '#')){
            if(freshLine){
                code[i][j++][k] = '\0';
                code[i][j][0] = '\n';
                freshLine = false;
                freshToken = false;
                k=0;
                j= 0;
                i++;
            }
            else if(freshToken){
                code[i][j][k] = '\0';
                freshToken = false;
                k=0;
                j++;
            }
            code[i][j][k++] = in;
        }
        else if(in == '\n' || in == '\r'){
            freshLine = true;
            munch = false;
            nestMe = false;
        }
        else if(in == ';'){
            munch = true;
            nestMe = false;
        }
        else if(!munch && !nestMe){
            freshToken = true;
        }
    }
    
    sizeOfFile = i+1;
}



void printOut(int howFar){
        int i;
        printf("\n-----------------------------------------\n");
        for(i =0; line[i] != endME && i< howFar; i++){
            printf("0x%04X \t %s \t 0x%0.4X\n", (i+startAdress),  pointer[i][0], line[i]);
        }
        printf("-----------------------------------------\n");
} 



//----------------------------------------------------------------------------------------------------------
//
//convert an Ascii to a Number if it exists
//-----------------------------------------
// 
int getNumber(char* number, int width, bool signedInt){
    
    if(strstr(number, "#")){
        
        int s =0;
        int maxValue = 1;
        while (s++<width){
           maxValue = maxValue * 2; 
        }
        int upperBound = maxValue;
        int lowerBound = 0;
        maxValue--;
        int out = (int)strtol(number +1,NULL,10);
        
        if(signedInt){
            lowerBound = -1*(upperBound/2);
            upperBound = upperBound/2-1;
        }
        if(out >=lowerBound && out <= upperBound){
            return maxValue & out; 
        }
        else{
            printf("value is out of bounds : %d, upper bound: %d, lower bound: %d \n", out, maxValue/2, (-1*maxValue/2)-1);
            allInputOK = false;
            return wrong;
        }
        
        
    }
    else if(strstr(number, "0X")|| strstr(number, "0x")){
        return strtol (number +2,NULL,16);

    }
    else if( strstr(number, "X") || strstr(number, "x")){
        return strtol (number +1,NULL,16);
    }
    else{
        allInputOK = false;
        return wrong;
    }
}


/**********************************************************************************************************

returns the hexadecimal start adress.
**********************************************************************************************************/

int setStartAdress(int index) {
    int output = wrong;
    int d=0;
    if(strcasecmp(code[index][d],".ORIG") == 0){
        d ++;
        int outcome = getNumber(code[index][d], 16, false);
        output = outcome;
    }
    startAdress = output;
    return output;
}


/*****************************************************************************************************
// Make a first pass and populate an array of string that include the LC3 pointers 
    if .orig     -> @
    if Halt      -> ^
    if .End      -> ~
    if opCode    -> $
    else it is a pointer(store It)
***********************************/

void copy(int *index, int codeLine, int startInd){
    int g =0;
    while (code[codeLine][g][0] != '\n'){
        pointer[*index][g+startInd] = code[codeLine][g];
        g++;
    }
    *index = *index+1;
}

int verifyEquality(int *index, int codeLine, char *compare, char *header){
    if(strcasecmp(code[codeLine][0],compare) == 0){
       pointer[*index][0] = header;
       return 1;       
    }
    else if(strcasecmp(code[codeLine][1],compare) == 0) {
        return 0;
        //no header because it has pointer.
    }   
    return -1;
}

bool Known(int *index, int codeLine){
    int startInd =-1;
    if(!containsOrig && codeLine< sizeOfFile){   
        if(strcasecmp(code[codeLine][0],".ORIG") == 0){
            pointer[*index][0] = ".ORIG";
            line[*index] = setStartAdress(codeLine);
            *index = *index+1;
            containsOrig = true;
        }
    }
    else if(!containsHalt && codeLine< sizeOfFile){
        if(strcasecmp(code[codeLine][0],"HALT") == 0){
            pointer[*index][0] = "HALT";
            line[*index] = HALT;
            *index = *index+1;
            preHalt=false;
            containsHalt = true;
        }
        
        else if((startInd = verifyEquality(index, codeLine, "BR", "  -")) >= 0){
            copy(index, codeLine, startInd);
        }
        else if((startInd = verifyEquality(index, codeLine, "BRN", "  -")) >= 0){
            copy(index, codeLine, startInd);
        }
        else if((startInd = verifyEquality(index, codeLine, "BRZ", "  -")) >= 0){
            copy(index, codeLine, startInd);
        }
        else if((startInd = verifyEquality(index, codeLine, "BRP", "  -")) >= 0){
            copy(index, codeLine, startInd);
        }
        else if((startInd = verifyEquality(index, codeLine, "BRNZ", "  -")) >= 0){
            copy(index, codeLine, startInd);
        }
        else if((startInd = verifyEquality(index, codeLine, "BRZP", "  -")) >= 0){
            copy(index, codeLine, startInd);
        }
        else if((startInd = verifyEquality(index, codeLine, "BRNP", "  -")) >= 0){
            copy(index, codeLine, startInd);
        }
        else if((startInd = verifyEquality(index, codeLine, "BRNZP", "  -")) >= 0){
            copy(index, codeLine, startInd);
        }
        else if((startInd = verifyEquality(index, codeLine, "JMP", "  -")) >= 0){
            copy(index, codeLine, startInd);
        }
        else if((startInd = verifyEquality(index, codeLine, "JSR", "  -")) >= 0){
            copy(index, codeLine, startInd);
        }
        else if((startInd = verifyEquality(index, codeLine, "JSRR", "  -")) >= 0){
            copy(index, codeLine, startInd);
        }
        else if((startInd = verifyEquality(index, codeLine, "LDI", "  -")) >= 0){
            copy(index, codeLine, startInd);
        }
        else if((startInd = verifyEquality(index, codeLine, "LDR", "  -")) >= 0){
            copy(index, codeLine, startInd);
        }
        else if((startInd = verifyEquality(index, codeLine, "NOT", "  -")) >= 0){
            copy(index, codeLine, startInd);
        }
        else if((startInd = verifyEquality(index, codeLine, "RET", "  -")) >= 0){
            copy(index, codeLine, startInd);
        }
        else if((startInd = verifyEquality(index, codeLine, "RTI", "  -")) >= 0){
            copy(index, codeLine, startInd);
        }
        else if((startInd = verifyEquality(index, codeLine, "STI", "  -")) >= 0){
            copy(index, codeLine, startInd);
        }
        else if((startInd = verifyEquality(index, codeLine, "STR", "  -")) >= 0){
            copy(index, codeLine, startInd);
        }
        else if((startInd = verifyEquality(index, codeLine, "PUTS", "  -")) >= 0){
            copy(index, codeLine, startInd);
        }
        else if((startInd = verifyEquality(index, codeLine, "LD", "  -")) >= 0){
            copy(index, codeLine, startInd);
        }
        else if((startInd = verifyEquality(index, codeLine, "ST", "  -")) >= 0){
            copy(index, codeLine, startInd);
        }
        else if((startInd = verifyEquality(index, codeLine, "ADD", "  -")) >= 0){
            copy(index, codeLine, startInd);
        }
        else if((startInd = verifyEquality(index, codeLine, "AND", "  -")) >= 0){
            copy(index, codeLine, startInd);
        }
        else if((startInd = verifyEquality(index, codeLine, "LEA", "  -")) >= 0){
            copy(index, codeLine, startInd);
        }
        else{
            copy(index, codeLine,0);
        }
    }
    else if(codeLine< sizeOfFile){
        if(strcasecmp(code[codeLine][0],".END") == 0){
            pointer[*index][0] = "~";
            line[*index] = endME;
            containsEnd = true;
            return true;
        }
        if(strcasecmp(code[codeLine][0],".FILL") == 0){
            pointer[*index][0] = "  -";
            line[*index] = getNumber(code[codeLine][1], 16, true);
            *index = *index+1;
        }
        else if(strcasecmp(code[codeLine][1],".FILL") == 0) {
            pointer[*index][0] = code[codeLine][0];
            line[*index] = getNumber(code[codeLine][2], 16, true);
            *index = *index+1;
        }   
    
        else if(strcasecmp(code[codeLine][0],".BLKW") == 0){
            int length;
            if(strstr(code[codeLine][1], "#")){
                length = strtol(code[codeLine][1]+1,NULL,10);
            }
            else{
                length = strtol(code[codeLine][1], NULL, 10);
            }
            int getNum = code[codeLine][2];
            int value = 0;
            if(getNum >= -32768 &&  getNum < 32768){
                int value = getNumber(getNum,16, true);
            }
            int endInd = length + *index;
            do{
                pointer[*index][0] = "  -";
                line[*index] = value;
                *index = *index+1;
            }while(*index<endInd);
        } 
        else if(strcasecmp(code[codeLine][1],".BLKW") == 0){
            int length;
            if(strstr(code[codeLine][2], "#")){
                length = strtol(code[codeLine][2]+1,NULL,10);
            }
            else{
                length = strtol(code[codeLine][2], NULL, 10);
            }
            int getNum = code[codeLine][3];
            int value = 0;
            if(getNum >= -32768 &&  getNum < 32768){
                int value = getNumber(getNum,16, true);
            }
            int endInd = length + *index;
            pointer[*index][0] = code[codeLine][0];
            line[*index] = value;
            *index = *index+1;
            while(*index<endInd){
                pointer[*index][0] = "  -";
                line[*index] = value;
                *index = *index+1;
            }
        } 
        
        else if(strcasecmp(code[codeLine][0],".STRINGZ") == 0){
            //parse the string
            int b=0;
            pointer[*index][0] = "  -";

            while(code[codeLine][1][b] == '"' && code[codeLine][1][b] != '\0' ){
                b++;
            }
            while(code[codeLine][1][b] != '"' && code[codeLine][1][b] != '\0' ){
                line[*index] = code[codeLine][1][b++];
                pointer[*index][0] = "  -";
                *index = *index+1;
            }
        } 
        
        else if(strcasecmp(code[codeLine][1],".STRINGZ") == 0){
            //parse the string
            int b=0;
            int starterIndex = *index;
            pointer[*index][0] = code[codeLine][0];
            while(code[codeLine][2][b] == '"' && code[codeLine][2][b] != '\0' ){
                b++;
                
            }
            while(code[codeLine][2][b] != '"' && code[codeLine][2][b] != '\0' ){
                line[*index] = code[codeLine][2][b++];
                if(*index != starterIndex){
                    pointer[*index][0] = "  -";
                }
                *index = *index+1;
            }   
        } 

        else{
            printf("crashed on: %s , %s, %s, %s",code[codeLine][0], code[codeLine][1] , code[codeLine][2], code[codeLine][3]);
            return true;
        }
    }
    else{
        return true;
    }
    return false;
}

/**********************************************************************************************************

parse aPointer or a number based on first chars.
************************************************/

//look out for pointer.
int parsePointerOrNumber(int index, int token){
    
    int i =0;
    while(strcasecmp(pointer[index][token], pointer[i][0]) != 0 && strcasecmp(pointer[i][0], "~") != 0){
        i ++;
    }
    if(strcasecmp(pointer[index][token], pointer[i][0]) == 0){
        if(verbose){
            printf("found matching ptr : %s at Addr. 0x%04x \n", pointer[index][token], (i+ startAdress));
        }
        return i - index + 1; //pc offset +1;
    }
    else{
        return getNumber(pointer[index][token],9, true);
    }
}


//look out for register.
int parseRegisterOrNumber(int index, int token){

    if(strstr(pointer[index][token], "R") || strstr(pointer[index][token], "r")){
        int outcome = strtol(pointer[index][token] +1,NULL,10);
        if(outcome >=0 && outcome <= 7){
            return outcome;
        }
        else{
            if(verbose){
                
                printf("invalid Register. must be between 0 and 7 incl. %s \n", pointer[index][token]);
            }
            allInputOK = false;
            return wrong;
        }
    }
    else{
        return getNumber(pointer[index][token],5, true)+ imm5;
    }
}

int offSet11(int index, int token){
    return getNumber(pointer[index][token],11, true);
}

int offSet6(int index, int token){
    return getNumber(pointer[index][token],6, true);
}

/**********************************************************************************************************

parse Registers based on their location diferent output.

************************************************/

//----------------------------------------------------------------------------------------------------------
//
//1st register
//-----------------------------------------
int dr(int index, int token){
    int output = parseRegisterOrNumber(index, token);
    if(output == 0){
        output = DR0;
    }
    else if(output == 1){
        output = DR1;
    }
    else if(output == 2){
        output = DR2;
    }
    else if(output == 3){
        output = DR3;
    }
    else if(output == 4){
        output = DR4;
    }
    else if(output == 5){
        output = DR5;
    }
    else if(output == 6){
        output = DR6;
    }
    else if(output == 7){
        output = DR7;
    }

    else{
        if(verbose){
            printf("wrong destination register: %s \n", pointer[index][token]);
        }
        allInputOK = false;
        output = wrong;
    }
    return output;
}


//----------------------------------------------------------------------------------------------------------
//
//2nd register
//-----------------------------------------
int sr(int index, int token){

    int output = parseRegisterOrNumber(index, token);

    if(output == 0){
        output = SR0;
    }
    else if(output == 1){
        output = SR1;
    }
    else if(output == 2){
        output = SR2;
    }
    else if(output == 3){
        output = SR3;
    }
    else if(output == 4){
        output = SR4;
    }
    else if(output == 5){
        output = SR5;
    }
    else if(output == 6){
        output = SR6;
    }
    else if(output == 7){
        output = SR7;
    }
    else{
        if(verbose){
            printf("wrong source register: %s \n", pointer[index][token]);
        }
        allInputOK = false;
        output = wrong;
    }
    return output;
}


/**********************************************************************************************************

read the OpCode and then pass on to sub method for reading
**********************************************************************************************************/
void writeOpCode(int index, int token){
    int regD = wrong;
    int regS = wrong;
    int ptrNum = wrong;
    int regNum = wrong;
    int off6 = wrong;
    int off11 = wrong;

    if(pointer[index][token] == NULL){
        line[index]= doNothing;
    }

    else if(strcasecmp(pointer[index][token],"BR") == 0){
        if((ptrNum = parsePointerOrNumber(index, token+2))!=wrong){
            line[index]= BR+ ptrNum;
        }
        else{
            allInputOK = false;
            line[index]= wrong;
        }
    }
    else if(strcasecmp(pointer[index][token],"BRN") == 0){
        if((ptrNum = parsePointerOrNumber(index, token+2))!=wrong){
            line[index]= BRN+ ptrNum;
        }
        else{
            allInputOK = false;
            line[index]= wrong;
        }
    }
    else if(strcasecmp(pointer[index][token],"BRZ") == 0){
        if((ptrNum = parsePointerOrNumber(index, token+2))!=wrong){
            line[index]= BRZ+ ptrNum;
        }
        else{
            allInputOK = false;
            line[index]= wrong;
        }
    }
    else if(strcasecmp(pointer[index][token],"BRP") == 0){
        if((ptrNum = parsePointerOrNumber(index, token+2))!=wrong){
            line[index]= BRP+ ptrNum;
        }
        else{
            allInputOK = false;
            line[index]= wrong;
        }
    }
    else if(strcasecmp(pointer[index][token],"BRNZ") == 0){
        if((ptrNum = parsePointerOrNumber(index, token+2))!=wrong){
            line[index]= BRNZ+ ptrNum;
        }
        else{
            allInputOK = false;
            line[index]= wrong;
        }
    }
    else if(strcasecmp(pointer[index][token],"BRZP") == 0){
        if((ptrNum = parsePointerOrNumber(index, token+2))!=wrong){
            line[index]= BRZP+ ptrNum;
        }
        else{
            allInputOK = false;
            line[index]= wrong;
        }
    }
    else if(strcasecmp(pointer[index][token],"BRNP") == 0){
        if((ptrNum = parsePointerOrNumber(index, token+2))!=wrong){
            line[index]= BRNP+ ptrNum;
        }
        else{
            allInputOK = false;
            line[index]= wrong;
        }
    }
    else if(strcasecmp(pointer[index][token],"BRNZP") == 0){
        if((ptrNum = parsePointerOrNumber(index, token+2))!=wrong){
            line[index]= BRNZP+ ptrNum;
        }
        else{
            allInputOK = false;
            line[index]= wrong;
        }
    }
    else if(strcasecmp(pointer[index][token],"JMP") == 0){
        if((regS = sr(index, token+2))!=wrong){
            line[index]= JMP+ regS;
        }
        else{
            allInputOK = false;
            line[index]= wrong;
        }
    }
    else if(strcasecmp(pointer[index][token],"JSR") == 0){
        if((off11 = offSet11(index, token+1)) != wrong){
            line[index]= JSR +off11;
        }
        else{
            allInputOK = false;
            line[index]= wrong;
        }
    }
    else if(strcasecmp(pointer[index][token],"JSRR") == 0){
        if((regS = sr(index, token+2))!=wrong){
            line[index]= JSRR+ regS;
        }
        else{
            allInputOK = false;
            line[index]= wrong;
        }
    }
    else if(strcasecmp(pointer[index][token],"LDI") == 0){
        if((regD = dr(index, token+1))!=wrong&&(ptrNum = parsePointerOrNumber(index, token+2))!=wrong){
            line[index]= LDI + regD + ptrNum;
        }
        else{
            allInputOK = false;
            line[index]= wrong;
        }
    }
    else if(strcasecmp(pointer[index][token],"LDR") == 0){
        if((regD = dr(index, token+1))!=wrong&&(regS = sr(index, token+2))!=wrong &&(off6 = offSet6(index, token+3)) != wrong){
            line[index]= LDR + regD + regS + off6;
        }
        else{
            allInputOK = false;
            line[index]= wrong;
        }
    }
    else if(strcasecmp(pointer[index][token],"NOT") == 0){
        if((regD = dr(index, token+1))!=wrong&&(regS = sr(index, token+2))!=wrong){
            line[index]= NOT + regD + regS;
        }
        else{
            allInputOK = false;
            line[index]= wrong;
        }
    }
    else if(strcasecmp(pointer[index][token],"RET") == 0){
            line[index]= RET;
    }
    else if(strcasecmp(pointer[index][token],"RTI") == 0){
            line[index]= RTI;
    }
    else if(strcasecmp(pointer[index][token],"STI") == 0){
        if((regD = dr(index, token+1))!=wrong&&(ptrNum = parsePointerOrNumber(index, token+2))!=wrong){
            line[index]= STI + regD + ptrNum;
        }
        else{
            allInputOK = false;
            line[index]= wrong;
        }
    }
    else if(strcasecmp(pointer[index][token],"STR") == 0){
        if((regD = dr(index, token+1))!=wrong&&(regS = sr(index, token+2))!=wrong &&(off6 = offSet6(index, token+3)) != wrong){
            line[index]= STR + regD + regS + off6;
        }
        else{
            allInputOK = false;
            line[index]= wrong;
        }
    }
    else if(strcasecmp(pointer[index][token],"LD") == 0){
        if((regD = dr(index, token+1))!=wrong&&(ptrNum = parsePointerOrNumber(index, token+2))!=wrong){
            line[index]= LD + regD + ptrNum;
        }
        else{
            allInputOK = false;
            line[index]= wrong;
        }
    }
    else if(strcasecmp(pointer[index][token],"ST") == 0){
        if((regD = dr(index, token+1))!=wrong&&(ptrNum = parsePointerOrNumber(index, token+2))!=wrong){
            line[index]= ST + regD + ptrNum;
        }
        else{
            allInputOK = false;
            line[index]= wrong;
        }
    }
    else if(strcasecmp(pointer[index][token],"ADD") == 0){
        if((regD = dr(index, token+1))!=wrong&&(regS = sr(index, token+2))!=wrong&&(regNum = parseRegisterOrNumber(index, token+3))!=wrong){
            line[index]= ADD + regD + regS + regNum;
        }
        else{
            allInputOK = false;
            line[index]= wrong;
        }
    }
    else if(strcasecmp(pointer[index][token],"AND") == 0){
        if((regD = dr(index, token+1))!=wrong&&(regS = sr(index, token+2))!=wrong&&(regNum = parseRegisterOrNumber(index, token+3))!=wrong){
            line[index]= AND + regD + regS + regNum;
        }
        else{
            allInputOK = false;
            line[index]= wrong;
        }
    }
    else if(strcasecmp(pointer[index][token],"LEA") == 0){
        if((regD = dr(index, token+1))!=wrong&&(ptrNum = parsePointerOrNumber(index, token+2))!=wrong){
            line[index]= LEA + regD + ptrNum;
        }
        else{
            allInputOK = false;
            line[index]= wrong;
        }
    }
    else if(strcasecmp(pointer[index][token],"PUTS") == 0){
        line[index]= PUTS;
    }
    else{
        if(verbose){  
            printf("invalid opcode: %s \n", pointer[index][token]);
        }
        allInputOK = false;
        line[index]= wrong;
    }
}


//---------------------
//https://en.wikipedia.org/wiki/Bitwise_operations_in_C
//---------------------
void showbits(int length, FILE *outFile){
    int i; 
    int u;
    for(u=0; u<length; u++){
        for(i=15; i>=12; i--){
            (line[u]&(1u<<i))?fputc('1',outFile):fputc('0',outFile);
        }
        fputc(' ',outFile);
        for(; i>=8; i--){
            (line[u]&(1u<<i))?fputc('1',outFile):fputc('0',outFile);
        }
        fputc(' ',outFile);
        for(; i>=4; i--){
            (line[u]&(1u<<i))?fputc('1',outFile):fputc('0',outFile);
        }
        fputc(' ',outFile);
        for(; i>=0; i--){
            (line[u]&(1u<<i))?fputc('1',outFile):fputc('0',outFile);
        }
        
        fputc('\n',outFile);
    }
}


/*$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$

main
**********************************************************************************************************/

int main(int argc, char **args){

    bool binRequest = false;
    
    // pass debug arg
    if(argc >2){
        if(strcasecmp(args[2],"-d") ==0){
            debug = true;   
            verbose = true;
        }
        if(strcasecmp(args[2],"-v") ==0){
            verbose = true;   
        }
        if(strcasecmp(args[2],"-b") ==0){
            binRequest = true;   
        }
    }

    // pass filename arg
    FILE *fp;
    char fileName[64]; 
    int z =0;
    while(args[1][z] != '\0'){
        fileName[z] = args[1][z];
        z++;
    }
    fileName[z]='\0';
    
    fp = fopen(fileName, "r");
    if (fp == NULL) {
        printf("Can't open input file!\n");
        exit(1);
    }

    //copy to memmory
    Tokenize(fp);
    fclose(fp);
    
    
    //1st pass
    bool endOfFile =false;
    int lineNb = 0;
    int expand =0;
    int *letItGrow;
    letItGrow = &expand;
    
    while (!endOfFile){
        endOfFile = Known(letItGrow, lineNb++);
        if(debug){
            printOut(expand);
        }
    }

    
    
    int hexLine =0;
    //if good->
    if(containsHalt && containsOrig && containsEnd && allInputOK){
        bool codeHasError = false;
        bool halted = false;
        
        //skip starting address
        while(strcasecmp(pointer[hexLine][0],".ORIG") != 0){
            hexLine++;
        }

        //2nd pass
        while(strcasecmp(pointer[++hexLine][0],"HALT") != 0){
            writeOpCode(hexLine, 1);
            if(line[hexLine] == wrong){
                codeHasError = true;
                if(verbose){
                    printf("error on line : %d \n", hexLine);
                }
            }
        }
        
        //check if error were reported
        if(!codeHasError && allInputOK){
            if(verbose){
                printOut(expand);
            }
            
            
            //crop filename and append .hex
            char fileOut[64];
            int i = 0;
            while(fileName[i] != '.' && fileName[i] != '\0'){
                fileOut[i] = fileName[i];
                i++;
            }
            fileOut[i]='\0';
            
            char out[1024] = "";
            strcat( out, fileOut); 
            strcat( out, ".hex");
            
            // output to file.
            FILE *f = fopen(out, "w+");
            if (f == NULL){
                printf("Error opening file!\n");
                exit(1);
            }
            for(i = 0; i < expand; i++){
                fprintf(f, "%.04X \n", line[i]);
            }
            //if binary is required
            if(binRequest){
                char outBin[1024] = "";
                strcat( outBin, fileOut); 
                strcat( outBin, ".bin");
                FILE *binaryFile = fopen(outBin, "w+");
                if (f == NULL){
                    printf("Error opening file!\n");
                    exit(1);
                }
                showbits(expand, binaryFile);
            }
            
            
            printf("DONE!");
            exit(0);
        }
        else{
            if(verbose){
                printOut(expand);
            }
            printf("cannot compile check your code. \n");
        }
        
    }
    else{
        printf("wrong code structure.");
        if(!containsHalt){
            printf("does not contain HALT. ");
        }
        if(!containsOrig){
            printf("does not contain .ORIG. ");
        }
        if(!containsEnd){
            printf("does not contain .END. ");
        }
        printf("\n");
    }
}
