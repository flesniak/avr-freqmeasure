avr-gcc -DF_CPU=8000000 -Os -mmcu=atmega8 -std=gnu99 -c -o hd44780.o hd44780.c
avr-gcc -DF_CPU=8000000 -Os -mmcu=atmega8 -std=gnu99 -o main main.c hd44780.o
avr-objcopy -O ihex main main.hex
#avrdude -p m8 -c usbasp -eU flash:w:main.hex
