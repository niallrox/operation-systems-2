CONFIG_MODULE_SIG=n
obj-m = kernellmodule.o
KERNEL = $(shell uname -r)

all:
	make -C /lib/modules/$(KERNEL)/build M=$(shell pwd) modules
	gcc kernellmodule_client.c -o kernellmodule_client

clean:
	make -C /lib/modules/$(KERNEL)/build M=$(shell pwd) clean
	rm kernellmodule_client
