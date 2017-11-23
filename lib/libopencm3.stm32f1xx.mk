DEFS += -DF_CPU=72000000
DEFS += -DF_APB1=3600000
LDSCRIPT = lib/stm32.ld
DFU_ADDRESS = 0x08002000
LIBNAME		= opencm3_stm32f1
DEFS		+= -DSTM32F1

FP_FLAGS	?= -msoft-float
ARCH_FLAGS	= -mthumb -mcpu=cortex-m3 $(FP_FLAGS) -mfix-cortex-m3-ldrd

include lib/librfn.rules.mk
include lib/librfm3.rules.mk
include lib/libopencm3.rules.mk
