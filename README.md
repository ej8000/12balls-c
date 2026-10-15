# 12balls-c
12 balls riddle game


First
windres 12balls.rc -O coff -o 12balls_res.o

Second

gcc -o 12balls_c.exe 12balls.c 12balls_res.o -mwindows -lgdi32 -luser32
