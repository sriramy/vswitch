#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_CMD_LEN (256)

int main(int argc, char *argv[])
{
	FILE *fp = popen("telnet localhost:8086", "w");
	char buf[MAX_CMD_LEN];
	int i = 0;

	memset(buf, 0, sizeof(buf));
	while (++i < argc) {
		strcat(buf, " ");
		strcat(buf, argv[i]);
	}
	fprintf(fp, "%s\r\n", buf);

	pclose(fp);
	return 0;
}
