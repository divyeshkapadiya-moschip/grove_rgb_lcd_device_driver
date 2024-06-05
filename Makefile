obj-m += lcd_driver.o
 
KDIR = /lib/modules/$(shell uname -r)/build
 
 
all:
	make -C $(KDIR)  M=$(shell pwd) modules
	@make -s -C app -f Makefile
 
clean:
	make -C $(KDIR)  M=$(shell pwd) clean
	@rm -rf app/lcd_test_app
