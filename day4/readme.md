## why different function name leads to right call?
"_io_hlt"and"_write_mem8" in naskfunc.nas  

but "io_hlt" and "write_mem8" in bootpack.c  

Gemini 1.5 pro experiment anwsers:  
> it depends on the Name Mangling 
> compiler and assembler

## a
write _write_mem8 in naskfunc.nas  

and code your bootpack.c to paint all pixels white

## b
Change to striped pattern(every 16 bytes)

## c
using C Language rather than assembly to achieve *project b*

## d and e
another way to write "pointer" in *project c*

## f
add palette
(origin: 320*200(2^8 colors), update: 320\*200(2^8\*2^8\*2^8 colors))

add i/o and io_cli function(Assembly)

## g
draw boxes

## h
draw an OS Interface(Initial)