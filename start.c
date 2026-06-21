#include <stdio.h>
#include <stdio.h>
#include <math.h>
//const int sttatic arr_optcode[]
//R type locitions
const int   OPCODE_MOVE_RIGHT=26;
const int   RS_MOVE_RIGHT = 20;
const int   RT_MOVE_RIGHT = 16;
const int   RD_MOVE_RIGHT = 11;
const int   SHAMT_MOVE_RIGHT = 6;
const int   FUNCT_MOVE_RIGHT = 0;
//I type locitions
const int   I_MOVE_RIGHT = 0;
//J type locitions
const int   ADDRESS_MOVE_RIGHT = 26;
//type sizes the amount of ones is the size
const int   OPCODE_SIZE = 2<<6-1;//6
const int   RS_SIZE = 2<<5-1;//5
const int   RT_SIZE = 2<<5-1;//5
const int   RD_SIZE = 2<<5-1;//5
const int   SHAMT_SIZE = 2<<5-1;//5
const int   FUNCT_SIZE = 2<<6-1;//6
const int   IMMEDIATE_SIZE = 2<<15-1;//15
const int   ADDRESS_SIZE = 2<<26-1;//26
//OPT_CODES
const int R_OPTCODE = 0;
const int J_OPTCODE = 5;//need to check
int main()
{   
    int IC=100;
    while(IC)
    {
        IC+=4;
    }
}
void commend_parse(int commend)
{
    int opcode = commend << OPCODE_MOVE_RIGHT;
    if (opcode ==R_OPTCODE)//if opcode is 0 then it is type R
    {
        R_parse(commend);
    }
    else
    {
        if(opcode == J_OPTCODE)
        {
            J_parse(commend);
        }
    }
    I_parse(commend);
}
void I_parse(int commend)
{
    int opcode = (commend << OPCODE_MOVE_RIGHT) && OPCODE_SIZE;
    int rs = (commend << RS_MOVE_RIGHT) && RS_SIZE;
    int rt = (commend << RT_MOVE_RIGHT) && RT_SIZE;
    int immediate = (commend<< I_MOVE_RIGHT)&&IMMEDIATE_SIZE;
}
void R_parse(int commend)
{
    int opcode = (commend << OPCODE_MOVE_RIGHT) && OPCODE_SIZE;
    int rs = (commend << RS_MOVE_RIGHT) && RS_SIZE;
    int rt = (commend << RT_MOVE_RIGHT) && RT_SIZE;
    
}
void J_parse(int commend)
{

}