#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

#define THERAML_PATH "/sys/class/thermal/thermal_zone0/temp"

int main(void)
{

int fd = open(THERAML_PATH, O_RDONLY);
if (fd <0) {
    fprintf(stderr, "Failed to open %s: %s\n", THERAML_PATH, strerror(errno));
    return EXIT_FAILURE;
}
char buffer[32];
memset(buffer, 0, sizeof(buffer));
ssize_t bytesRead = read(fd, buffer, sizeof(buffer) - 1);
if (bytesRead < 0) {
    perror("Failed to read from temperature file");
    close(fd);
    return EXIT_FAILURE;
}

long raw_temp = strtol(buffer, NULL, 10);
double temperature = raw_temp / 1000.0;
printf("[System Sensor]Current CPU Temperature: %.2f°C\n", temperature);
close(fd);
return EXIT_SUCCESS;
}