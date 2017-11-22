/* Define memory regions. */
MEMORY
{
	rom (rx) : ORIGIN = 0x08002000, LENGTH = 120K
	ram (rwx) : ORIGIN = 0x20000000, LENGTH = 20K
}

/* Include the common ld script. */
INCLUDE libopencm3_stm32f1.ld
