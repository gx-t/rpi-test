#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

static void set_conn_timeout() {
	struct timeval tt = {
		.tv_sec = 1,
		.tv_usec = 0
	};      
	setsockopt(STDOUT_FILENO, SOL_SOCKET, SO_SNDTIMEO, (char*)&tt, sizeof(tt));
}

static const char* curr_time() {
	time_t tt = time(0);
	return asctime(localtime(&tt));
}

int main(int argc, char* argv[]) {
	if(argc != 3) {
		return 1;
	}
	int ss = socket(AF_INET, SOCK_STREAM, 0);
	if(ss < 0)
	{
		perror("socket");
		return 2;
	}

	int on = 1;
	setsockopt(ss, SOL_SOCKET, SO_REUSEADDR, (char*)&on, sizeof(on));
	
	struct sockaddr_in sin = {
		.sin_family = AF_INET,
		.sin_addr.s_addr = INADDR_ANY,
		.sin_port = htons(8888)
	};
	if(bind(ss, (struct sockaddr *) &sin, sizeof(sin)) < 0 || listen(ss, 1) < 0)
	{
		close(ss);
		perror("bind/listen");
		return 3;
	}

	memset(&sin, 0, sizeof(sin));
	
	socklen_t addr_len = sizeof(sin);
	int cc = accept(ss, (struct sockaddr*)&sin, &addr_len);
	close(ss);

	dup2(cc, STDOUT_FILENO);
	close(cc);

	fprintf(stderr, "%s\tConnected: %s:%d\n", curr_time(), inet_ntoa(sin.sin_addr), ntohs(sin.sin_port));
	set_conn_timeout();

	argv ++;
	execv(*argv, argv);
	perror("execv");

	return 5;
}

