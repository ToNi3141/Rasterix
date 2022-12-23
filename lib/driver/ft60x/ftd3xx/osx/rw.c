#include <string.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "ftd3xx.h"

#define FIFO_CHANNEL_1	0
#define FIFO_CHANNEL_2	1
#define FIFO_CHANNEL_3	2
#define FIFO_CHANNEL_4	3

static void show_help(const char *bin)
{
	printf("Usage: %s <write length> <read length>\r\n", bin);
	printf("  Only FT245 mode is supported in this demo\r\n");
}

int main(int argc, char *argv[])
{
	FT_HANDLE handle = NULL;

	if (argc != 3) {
		show_help(argv[0]);
		return false;
	}
	uint32_t to_write = atoi(argv[1]);
	uint32_t to_read = atoi(argv[2]);

	FT_Create(0, FT_OPEN_BY_INDEX, &handle);

	if (!handle) {
		printf("Failed to create device\r\n");
		return -1;
	}
	printf("Device created\r\n");

	uint8_t *out_buf = (uint8_t *)malloc(to_write);
	uint8_t *in_buf = (uint8_t *)malloc(to_read);
	DWORD count;

	if (FT_OK != FT_WritePipeEx(handle, FIFO_CHANNEL_1, out_buf, to_write,
				&count, 0xFFFFFFFF)) {
		printf("Failed to write\r\n");
		goto _Exit;
	}
	printf("Wrote %d bytes\r\n", count);

	if (FT_OK != FT_ReadPipeEx(handle, FIFO_CHANNEL_1, in_buf, to_read,
				&count, 0xFFFFFFFF)) {
		printf("Failed to read\r\n");
		goto _Exit;
	}
	printf("Read %d bytes\r\n", count);

_Exit:
	free(in_buf);
	free(out_buf);
	FT_Close(handle);
	return 0;
}
