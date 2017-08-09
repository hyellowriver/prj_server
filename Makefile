CUR_DIR :=.

INC_DIR = $(CUR_DIR)

INCLUDE = $(foreach dir, $(INC_DIR), -I$(dir))

CC_OPTS += $(INCLUDE)

CFLAGS = -g -lpthread

CC = g++

OBJ = io_mux.o\
	  paas_pro.o\
	  sig_deal.o\
	  sys_log.o\
	  thread_base.o\
	  sql_ctr.o

server : $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(INCLUDE) -lpthread


clean:
	rm -rf *.o server