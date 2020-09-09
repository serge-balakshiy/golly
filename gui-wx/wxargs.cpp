
#include <iostream>
#include <string.h>
#include "wxargs.h"

char **ArgAddPtr (char **pp, int size, char *str)
{
    if(size == 0){
        pp = new char *[size+1];
    }
    else{                      
        char **copy = new char* [size+1];
        for(int i = 0; i < size; i++)
        {
            copy[i] = pp[i];
        }  
 
        delete [] pp;      
        pp = copy;     
    }
    pp[size] = new char [strlen(str) + 1];
    strcpy(pp[size], str);
 
    return pp;
}

char **ArgDelPtr (char **pp, int size, int ncell)
{
    char **copy = new char* [size-1]; //временная копия
        //копируем адреса указателей, кроме ячейки с номером ncell
    for(int i = 0;  i < ncell; i++)
    {
        copy[i] = pp[i];
    }  
    for(int i = ncell;  i < size-1; i++)
    {
        copy[i] = pp[i+1];
    }
 
    delete [] pp[ncell]; // освобождаем память занимаемую строкой
    delete [] pp; //освобождаем память занимаемую массивом указателей
    pp = copy; //показываем, какой адрес хранить
 
    return pp;
}
 
char **ArgInsPtr (char **pp, int size, int ncell, char *str)
{
    char **copy = new char* [size+1]; //временная копия
    for(int i = 0;  i < ncell; i++)
    {
        copy[i] = pp[i];
    }  
 
    copy[ncell] = new char[strlen(str) + 1]; //выделяем память для новой строки
    strcpy(copy[ncell], str);
 
    for(int i = ncell+1;  i < size+1; i++)
    {
        copy[i] = pp[i-1];
    }
    delete [] pp;
    pp = copy; 
 
    return pp;
}

