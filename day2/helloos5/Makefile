
# if only input "make", execute the default command

default :
	../../tolset_h/z_tools/make.exe img

# the generation rules(the left of ':' is the output, the right of ':' is the input)
# If make.exe need the left file, and the right condition is fully met, it would execute the following commands

ipl.bin : ipl.nas Makefile
	../../tolset_h/z_tools/nask.exe ipl.nas ipl.bin ipl.lst

helloos.img : ipl.bin Makefile
	../../tolset_h/z_tools/edimg.exe   imgin:../../tolset_h/z_tools/fdimg0at.tek \
		wbinimg src:ipl.bin len:512 from:0 to:0   imgout:helloos.img

# make + [option]
# e.g. You input "make img", it is equal to input "../../tolset_h/z_tools/make.exe -r helloos.img"

asm :
	../../tolset_h/z_tools/make.exe -r ipl.bin

img :
	../../tolset_h/z_tools/make.exe -r helloos.img

run :
	../../tolset_h/z_tools/make.exe img
	copy helloos.img ..\..\tolset_h\z_tools\qemu\fdimage0.bin
	../../tolset_h/z_tools/make.exe -C ../../tolset_h/z_tools/qemu

install :
	../../tolset_h/z_tools/make.exe img
	../../tolset_h/z_tools/imgtol.com w a: helloos.img

clean :
	-del ipl.bin
	-del ipl.lst

src_only :
	../../tolset_h/z_tools/make.exe clean
	-del helloos.img
