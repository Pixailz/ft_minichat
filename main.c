#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

typedef unsigned long t_size;

int		server_fd = -1;
fd_set	fds;

int		client_id = -1;
int		clients[0x400] = {-1};

char	msg[0x2000];

t_size	ft_strlen(const char *str)
{
	char *ptr;

	ptr = (char *)str;
	if (!ptr)
		return (0);
	while (*ptr)
		ptr++;
	return (ptr - str);
}

void	ft_putstr(int fd, char *str)
{ write(fd, str, ft_strlen(str)); }

int	fatal()
{ ft_putstr(2, "Fatal error\n"); return (1); }

void	sendto_all(int connfd)
{
	int	counter = 0;
	int	msg_len = ft_strlen(msg);

	while (counter <= client_id)
	{
		if (clients[counter] != connfd && clients[counter] > 0)
			send(clients[counter], msg, msg_len, 0);
		counter++;
	}
	ft_putstr(1, msg);
}

int	get_max_fd(void)
{
	int	max = server_fd;
	int	counter = 0;

	while (counter <= client_id)
	{
		if (clients[counter] > max)
			max = clients[counter];
		counter++;
	}
	return (max);
}

int	get_client_id(int fd)
{
	int	counter = 0;
	while (counter <= client_id)
	{
		if (clients[counter] == fd)
			return (counter);
		counter++;
	}
	return (-1);
}

void	reset_fdset(void)
{
	int counter = 0;

	FD_ZERO(&fds);
	while (counter <= client_id)
	{
		FD_SET(clients[counter], &fds);
		counter++;
	}
	FD_SET(server_fd, &fds);
}

int	accept_client(void)
{
	int					connfd;
	struct	sockaddr_in	client_addr;
	int					len = sizeof(client_addr);

	connfd = accept(server_fd, (struct sockaddr *)&client_addr, (socklen_t *)&len);
	client_id++;
	clients[client_id] = connfd;
	sprintf(msg, "server: client %d just arrived\n", client_id);
	sendto_all(connfd);
	FD_SET(connfd, &fds);
	return (connfd);
}

int	manage_client(int fd)
{
	char	buff[0x1000];
	int		received_len;
	int		id;

	id = get_client_id(fd);
	bzero(buff, 0x1000);
	received_len = recv(fd, buff, 0x1000, 0);
	if (received_len <= 0)
	{
		FD_CLR(fd, &fds);
		sprintf(msg, "server: client %d just left\n", id);
		sendto_all(fd);
		close(clients[id]);
		clients[id] = -1;
		return (1);
	}
	sprintf(msg, "client %d: %s", id, buff);
	sendto_all(fd);
	return (0);
}

void	main_listen(void)
{
	while (0x42)
	{
		bzero(&msg, sizeof(msg));
		reset_fdset();
		if (select(get_max_fd() + 1, &fds, NULL, NULL, NULL) < 0)
			continue;
		for (int fd = 0; fd <= get_max_fd(); fd++)
		{
			if (FD_ISSET(fd, &fds))
			{
				if (fd == server_fd)
				{
					accept_client();
					break;
				}
				if (manage_client(fd))
					break;
			}
		}
	}
}

int	init_serv(int port)
{
	// 1. Create socket
	server_fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
	if (server_fd < 0)
		return (1);

	// optional set addr reuse
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (int[1]){1}, sizeof(int)))
		return (1);

	// 2. Specify port
	struct sockaddr_in addr;

	bzero(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = 0x0100007f;
	addr.sin_port = htons(port);

	// 3. Bind
	if (bind(server_fd, (const struct sockaddr *)&addr, sizeof(addr)) != 0)
		return (1);

	// 4. Listen
	if (listen(server_fd, 1))
		return (1);
	return (0);
}

int	main(int ac, char **av)
{
	if (ac != 2)
	{
		ft_putstr(2, "Wrong number of arguments\n");
		return (1);
	}
	if (init_serv(atoi(av[1])))
	{
		fatal();
		return (1);
	}
	main_listen();
	close(server_fd);
	return (0);
}
