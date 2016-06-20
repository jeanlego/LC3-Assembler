# LC3-Assembler
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

