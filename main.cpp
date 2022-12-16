#include "lib/serialib.h"
#include <unistd.h>
#include <stdio.h>
#include <getopt.h>
#include <errno.h>
#include <semaphore.h>
#include <pthread.h>

#define CMD_LENGTH          15
#define SERIAL_PORT_DEFAULT (char*)"/dev/ttyUSB0"
#define BAUDRATE_DEFAULT    ((unsigned int)9600)

uint64_t getopt_integer(char *optarg);
static void usage(const char *name);
void *serial_write(void *args);
void *serial_read(void *args);
bool check_echo(uint8_t *buffer);
serialib serial;
sem_t ok_to_send;

const uint8_t cmd[CMD_LENGTH] = {
    0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF,
    0x64, 0x05, 0x90, 0x00,
    0x64, 0x50, 0xC8
};

uint64_t getopt_integer(char *optarg) {
	int rc;
	uint64_t value;

	rc = sscanf(optarg, "0x%lx", &value);
	if (rc <= 0)
		rc = sscanf(optarg, "%lu", &value);

	return value;
}

static struct option const long_opts[] = {
	{"port", required_argument, NULL, 'p'},
	{"badurate", required_argument, NULL, 'b'},
	{"help", no_argument, NULL, 'h'},
	{0, 0, 0, 0}
};

static void usage(const char *name)
{
	int i = 0;

	fprintf(stdout, "%s\n\n", name);
	fprintf(stdout, "usage: %s [OPTIONS]\n\n", name);
	fprintf(stdout, "  -%c (--%s) serial port (defaults to %s)\n",
		long_opts[i].val, long_opts[i].name, SERIAL_PORT_DEFAULT);
	i++;
	fprintf(stdout, "  -%c (--%s) baudrate of serial port (defaults to %d)\n",
		long_opts[i].val, long_opts[i].name, BAUDRATE_DEFAULT);
    i++;
    fprintf(stdout, "  -%c (--%s) print usage help and exit\n",
		long_opts[i].val, long_opts[i].name);
}

/*!
 * \brief main  Simple example that send ASCII characters to the serial device
 * \return      0 : success
 *              1 : an error occured
 */
int main(int argc, char *argv[]) {
    int cmd_opt;
    int rc;
    int *serial_r_return;

    pthread_t serial_r, serial_w;
    char *serial_port = SERIAL_PORT_DEFAULT;
    unsigned int baudrate = BAUDRATE_DEFAULT;
    char errorOpening;

    while ((cmd_opt = getopt_long(argc, argv, "hp:b:", long_opts, NULL)) != -1) {
		switch (cmd_opt) {
            case 'p':
                serial_port = strdup(optarg);
			    break;
            
            case 'b':
                baudrate = getopt_integer(optarg);
                break;
            
            case 'h':
            default:
                usage(argv[0]);
                exit(0);
                break;
        }
    }

    // Connection to serial port
    errorOpening = serial.openDevice(serial_port, baudrate);

    // If connection fails, return the error code otherwise, display a success message
    if (errorOpening != 1) {
        fprintf(stderr, "unable to open serial port %s, %d.\n", serial_port, errorOpening);
		perror("open serial port");
		return -EINVAL;
    }

    printf ("Successful connection to %s, baudrate: %d\n", serial_port, baudrate);

    rc = pthread_create(&serial_r, NULL, serial_read, NULL);
    if (rc != 0) {
        fprintf(stderr, "unable to create serial reading thread, %d.\n", rc);
		perror("create thread");
		return -EINVAL;
    }

    rc = pthread_create(&serial_w, NULL, serial_write, NULL);
    if (rc != 0) {
        fprintf(stderr, "unable to create serial writing thread, %d.\n", rc);
		perror("create thread");
		return -EINVAL;
    }
    sem_init(&ok_to_send, 0, 1);    // Set the initial value for semaphore

    pthread_join(serial_w, NULL);
    pthread_join(serial_r, (void**)&serial_r_return);

    if (*serial_r_return == 0) {
        printf("Config OK.\n");
    }
    else {
        printf("Config error.\n");
    }
    
    return *serial_r_return;
}

void *serial_write(void *args) {
    int count;
    int rc;

    for (count = 0; count < REPEAT_COUNT; count++)
    {
        sem_wait(&ok_to_send);
        printf("Start sending #%d...\n", count+1);
        
        rc = serial.writeBytes(cmd, CMD_LENGTH);
        if (rc != 1) {
            fprintf(stderr, "unable to writing via serial, %d.\n", rc);
            perror("serial writing");
            goto close;
        }

        printf("Sending done #%d.\n", count+1);
        sleep(1);
    }
    
    printf("All sending done.\n");

close:
    // Close the serial device
    serial.closeDevice();

    printf("Serial port close.\n");
}

void *serial_read(void *args) {
    int count, i;
    static int ret = 0;
    uint8_t rc[CMD_LENGTH];

    for (count = 0; count < REPEAT_COUNT; count++) {
        for (i = 0; i < CMD_LENGTH; i++) {
            rc[i] = 0;
        }
        sem_post(&ok_to_send);
        printf("Start receiving #%d...\n", count+1);

        serial.readString((char*)rc, cmd[CMD_LENGTH], CMD_LENGTH);
        
        for (i = 0; i < CMD_LENGTH; i++) {
            printf("0x%02X ", rc[i]);
        }

        if (!check_echo(rc)) {
            ret = 1;
            printf("\nReceived error!");
        }

        printf("\nEnd receiving #%d.\n", count+1);
    }

    pthread_exit((void*)(&ret));
}

bool check_echo(uint8_t *buffer) {
    int i;

    for (i = 0; i < CMD_LENGTH; i++) {
        if (*(buffer+i) != cmd[i]) {
            return false;
        }
    }

    return true;
}