#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include <errno.h>

#define MAX_CMD_LEN (4096)

int main(int argc, char *argv[])
{
	char finish[] = "\0\1\0\1\0";
	struct sockaddr_in servaddr;
	char buf[MAX_CMD_LEN];
	int i = 0, again = 1;
	int fd, rc;

	fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd < 0) {
		fprintf(stderr, "%s: socket failed %s", argv[0], strerror(errno));
		return -1;
	}

	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	servaddr.sin_port = htons(8086);

	rc = connect(fd, &servaddr, sizeof(servaddr));
	if (rc < 0) {
		fprintf(stderr, "%s: connect failed %s", argv[0], strerror(errno));
		goto err;
	}

	memset(buf, 0, sizeof(buf));
	while (++i < argc) {
		strcat(buf, " ");
		strcat(buf, argv[i]);
	}
	strcat(buf, "\r\n");

	rc = write(fd, buf, strlen(buf));
	if (rc < 0) {
		fprintf(stderr, "%s: write failed %s", argv[0], strerror(errno));
		goto err;
	}

	while (again) {
		memset(buf, 0, sizeof(buf));
		rc = read(fd, buf, sizeof(buf));
		if (rc == -1) {
			if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
				continue;
			}
			fprintf(stderr, "%s: read failed %s", argv[0], strerror(errno));
		}

		if (rc > 0)
			printf("%s\n", buf);

		again = (rc > 0) && (strncmp(buf, finish, strlen(finish)) != 0);
	};

err:
	close(fd);
	return rc;
}
