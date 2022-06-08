TARGET_MODULE := logger

#$(TARGET_MODULE)-objs := source_1.o source_2.o … source_n.o
obj-m += $(TARGET_MODULE).o
logger-objs := logger_main.o logger_procfs.o

MODULE_DIR = "/lib/modules/`uname -r`/build"

all:
	$(MAKE) -C $(MODULE_DIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(MODULE_DIR) M=$(PWD) clean

load:
	/sbin/insmod ./$(TARGET_MODULE).ko

unload:
	/sbin/rmmod ./$(TARGET_MODULE).ko
