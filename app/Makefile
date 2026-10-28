CC_ARM ?= arm-linux-gnueabihf-gcc
CC_HOST ?= gcc
CFLAGS = -Wall -Wextra -pthread
LDFLAGS = -lrt -lm

SRCS = sms_app.c circular_buffer.c sensor_thread.c processor_thread.c alert_thread.c logger_thread.c

all: arm host

arm: $(SRCS)
	$(CC_ARM) -static $(CFLAGS) $(SRCS) -o sms_app $(LDFLAGS)

host: $(SRCS)
	$(CC_HOST) $(CFLAGS) $(SRCS) -o sms_app_x86 $(LDFLAGS)

clean:
	rm -f sms_app sms_app_x86
