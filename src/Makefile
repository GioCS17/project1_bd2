OBJS	= main.o
SOURCE	= main.cpp
HEADER	= disk_manager.h b_plus_tree.h b_plus_tree_node.h b_plus_tree_iterator.h data_base.h
OUT	= main
CC	 = g++
FLAGS	 = -g -c -Wall
LFLAGS	 = 

all: $(OBJS)
	$(CC) -g $(OBJS) -o $(OUT) $(LFLAGS)

main.o: main.cpp
	$(CC) $(FLAGS) main.cpp 


clean:
	rm -f $(OBJS) $(OUT) data.bin data.index
run: $(OUT)
	./$(OUT)