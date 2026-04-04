# 12balls-c
12 balls riddle game


First, used windres to assemble 12balls.rc into a Windows.res file.

Second,

gcc -o 12balls_c.exe 12balls.c 12balls.res -mwindows -lgdi32 -luser32
