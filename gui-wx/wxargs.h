#ifndef WXARGS_H
#define WXARGS_H

char **ArgAddPtr (char **pp, int size, char *str);
char **ArgDelPtr (char **pp, int size, int ncell); // удаление строки и указателя
char **ArgInsPtr (char **pp, int size, int ncell, char *str); //вставка строки и указателя

#endif // WXARGS_H
