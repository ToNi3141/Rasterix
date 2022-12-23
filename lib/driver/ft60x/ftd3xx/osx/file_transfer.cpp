#include <fstream>
#include <thread>
#include <chrono>
#include <random>
#include "common.hpp"

static bool loop_mode;
static bool is_ft600_mode;
static const uint32_t WR_CTRL_INTERVAL = 1000; /* 1 second */
static const uint32_t RD_CTRL_INTERVAL = 1000; /* 1 second */
static const int BUFFER_LEN = 128*1024;
static random_device rd;
static mt19937 rng(rd());
static uniform_int_distribution<size_t> random_len(1, BUFFER_LEN / 4);
static size_t file_length;
static bool transfer_failed;

static void stream_out(FT_HANDLE handle, uint8_t channel,
		string from)
{
	unique_ptr<uint8_t[]> buf(new uint8_t[BUFFER_LEN]);
	ifstream src;
	try {
		src.open(from, ios::binary);
	} catch (istream::failure e) {
		cout << "Failed to open file " << e.what() << endl;
		return;
	}
	size_t total = 0;

	while (!do_exit && total < file_length) {
		size_t len = random_len(rng) * 4;
		ULONG count = 0;

		src.read((char*)buf.get(), len);
		if (!src)
			len = (int)src.gcount();
		if (len) {
_retry:
			FT_STATUS status = FT_WritePipeEx(handle, channel, buf.get(),
						len, &count, WR_CTRL_INTERVAL + 100);
			if (FT_OK != status) {
				if (do_exit)
					break;
				printf("Channel %d failed to write %zu, ret %d\r\n",
						channel, total, status);
				if (FT_TIMEOUT == status)
					goto _retry;
				do_exit = true;
			}
		} else
			count = 0;
		tx_count += count;
		total += count;
	}
	src.close();
	printf("Channel %d write stopped, %zu\r\n", channel, total);
}

static void stream_in(FT_HANDLE handle, uint8_t channel,
		string to)
{
	unique_ptr<uint8_t[]> buf(new uint8_t[BUFFER_LEN]);
	ofstream dest;
	size_t total = 0;

	try {
		dest.open(to, ofstream::binary | ofstream::in | ofstream::out |
				ofstream::trunc);
	} catch (istream::failure e) {
		cout << "Failed to open file " << e.what() << endl;
		return;
	}

	while (!do_exit && total < file_length) {
		ULONG count = 0;
		size_t len = random_len(rng) * 4;
		size_t left = file_length - total;

		if (len > left)
			len = left;

		FT_STATUS status = FT_ReadPipeEx(handle, channel, buf.get(), len,
				&count, RD_CTRL_INTERVAL + 100);
		if (!count) {
			printf("Failed to read from channel %d, status:%d\r\n",
					channel, status);
			continue;
		}
		dest.write((const char *)buf.get(), count);
		rx_count += count;
		total += count;
	}
	dest.close();
	printf("Channel %d read stopped, %zu\r\n", channel, total);
}

static void show_help(const char *bin)
{
	printf("File transfer through FT245 loopback FPGA\r\n");
	printf("Usage: %s <src> <dest> <mode> [loop]\r\n", bin);
	printf("  src: source file name to read\r\n");
	printf("  dest: target file name to write\r\n");
	printf("  mode: 0 = FT245 mode(default), 1-4 FT600 channel count\r\n");
	printf("  loop: 0 = oneshot(default), 1 =  loop forever\r\n");
}

static bool validate_arguments(int argc, char *argv[])
{
	if (argc != 4 && argc != 5)
		return false;

	if (argc == 5) {
		int val = atoi(argv[4]);
		if (val != 0 && val != 1)
			return false;
		loop_mode = (bool)val;
	}

	int ch_cnt = atoi(argv[3]);
	if (ch_cnt > 4)
		return false;
	is_ft600_mode = ch_cnt != 0;
	in_ch_cnt = out_ch_cnt = ch_cnt;
	return true;
}

static inline ifstream::pos_type get_file_length(ifstream &stream)
{
	return stream.seekg(0, ifstream::end).tellg();
}

static inline ifstream::pos_type get_file_length(const string &name)
{
	ifstream file(name, std::ifstream::binary);

	return get_file_length(file);
}

static bool compare_content(const string &from, const string &to)
{
	ifstream in1(from);
	ifstream in2(to);
	ifstream::pos_type size1, size2;

	size1 = get_file_length(in1);
	in1.seekg(0, ifstream::beg);

	size2 = get_file_length(in2);
	in2.seekg(0, ifstream::beg);

	if(size1 != size2) {
		cout << to << " size not same: " << size1 << " " << size2 << endl;
		in1.close();
		in2.close();
		return false;
	}

	static const size_t BLOCKSIZE = 4096;
	size_t remaining = size1;

	while(remaining) {
		char buffer1[BLOCKSIZE], buffer2[BLOCKSIZE];
		size_t size = std::min(BLOCKSIZE, remaining);

		in1.read(buffer1, size);
		in2.read(buffer2, size);

		if(0 != memcmp(buffer1, buffer2, size)) {
			for (size_t i = 0; i < size; i++) {
				if (buffer1[i] != buffer2[i]) {
					size_t offset = (int)size1 - (int)remaining + i;
					cout << to << " content not same at " << offset << endl;
					break;
				}
			}
			in1.close();
			in2.close();
			return false;
		}

		remaining -= size;
	}
	in1.close();
	in2.close();
	cout << to << " binary same" << endl;
	return true;
}

void file_transfer(FT_HANDLE handle, uint8_t channel, string from, string to)
{
	do {
		thread write_thread = thread(stream_out, handle, channel, from);
		thread read_thread = thread(stream_in, handle, channel, to);

		if (write_thread.joinable())
			write_thread.join();
		if (read_thread.joinable())
			read_thread.join();

		if (!compare_content(from, to))
			transfer_failed = true;
	} while (loop_mode && !do_exit);
}

int main(int argc, char *argv[])
{

	get_version();

	if (!validate_arguments(argc, argv)) {
		show_help(argv[0]);
		return 1;
	}

	if (!get_device_lists(500))
		return 1;

	if (!set_channel_config(is_ft600_mode, CONFIGURATION_FIFO_CLK_100))
		return 1;

	/* Must be called before FT_Create is called */
	turn_off_thread_safe();

	FT_HANDLE handle = NULL;

	FT_Create(0, FT_OPEN_BY_INDEX, &handle);

	if (!handle) {
		printf("Failed to create device\r\n");
		return -1;
	}
	register_signals();

	for (int i = 0; i < in_ch_cnt; i++) {
		FT_SetPipeTimeout(handle, 2 + i, WR_CTRL_INTERVAL + 100);
		FT_SetPipeTimeout(handle, 0x82 + i, RD_CTRL_INTERVAL + 100);
	}

	thread transfer_thread[4];
	thread measure_thread = thread(show_throughput, handle);

	string from(argv[1]);
	string to(argv[2]);

	file_length = get_file_length(from);

	if (file_length == 0) {
		cout << "Input file not correct" << endl;
		return -1;
	}

	for (int i = 0; i < in_ch_cnt; i++) {
		string target = to;
		if (in_ch_cnt > 1)
			target += to_string(i);
		transfer_thread[i] = thread(file_transfer, handle, i, from, target);
	}

	for (int i = 0; i < in_ch_cnt; i++)
		transfer_thread[i].join();

	do_exit = true;
	if (measure_thread.joinable())
		measure_thread.join();
	return transfer_failed;
}
