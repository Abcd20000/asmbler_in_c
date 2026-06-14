#include <stdio.h>
#include <stdio.h>
//const int sttatic arr_optcode[]
const int OPCODE_MOVE_RIGHT=26;
const int RS_MOVE_RIGHT = 20;
const int RT_MOVE_RIGHT = 16;
const int RD_MOVE_RIGHT = 11;
const int SHAMT_MOVE_RIGHT = 6;
const int FUNCT_MOVE_RIGHT = 0;
const int I_MOVE_RIGHT = 0;
const int SIZE_SMALL_FIELD=5;
const int SIZE_MEDIUM_FILED = 6;
const int SIZE_I_IMMEDIATE = 16;
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
    if (opcode ==0)//if opcode is 0 then it is type R
    {

    }
}
void I_parse(int commend)
{
    int opcode = (commend << OPCODE_MOVE_RIGHT) &&SIZE_MEDIUM_FILED;
    int rs = (commend << RS_MOVE_RIGHT) && SIZE_SMALL_FIELD;
    int rt = (commend << RT_MOVE_RIGHT) && SIZE_SMALL_FIELD;
    int immediate = (commend<< I_MOVE_RIGHT)&&SIZE_I_IMMEDIATE;
}
void R_parse()
{

}
void J_parse()
{

}