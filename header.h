#include <stdio.h>
#include <stdlib.h>
#include <assert.h>



int Read(int fd, char *buffer, int size){
    ssize_t total_bytes_read = 0;
    while (total_bytes_read != size)
    {
        assert(total_bytes_read < size);
        ssize_t bytes_read = read(fd,
                                    buffer + total_bytes_read,
                                    size - total_bytes_read);
        if (bytes_read <= 0)
        {
            return EXIT_FAILURE;
            //break;
        }
        total_bytes_read += bytes_read;
    }
    return EXIT_SUCCESS;
}

int Write(int fd, char *buffer, int size){
    ssize_t total_bytes_written = 0;
    while (total_bytes_written != size)
    {
        assert(total_bytes_written < size);
        ssize_t bytes_written = write(fd,
                                    buffer + total_bytes_written,
                                    size - total_bytes_written);
        if (bytes_written <= 0)
        {
            return EXIT_FAILURE;
            //break;
        }
        total_bytes_written += bytes_written;
    }
    return EXIT_SUCCESS;
}

