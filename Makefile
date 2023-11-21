T		:= prog

S		:= main.c

O		:= $(S:%.c=%.o)

$(T): $(O)
	gcc -Wall -Wextra -ggdb -o $(T) $(O)

%.o: %.c
	gcc -Wall -Wextra -ggdb -o $@ -c $<

clean:
	rm -rf *.o

fclean: clean
	rm -f $(T)

re: fclean $(T)

run: re
	./$(T) 4444
