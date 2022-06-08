#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

int main () {
	int fd;
	int retval;
  unsigned char buf[4];
	
	if ((fd = open( "/proc/logger/buffer", O_RDONLY)) < 0) {
    return -1;
	}
	while ( ( retval = read ( fd, &buf, 4 ) ) > 0 ) {
		printf ( "Chunk: %s", buf );
		if ( retval == 4 ) {
			sleep ( 2 );
		} else {
			break;
		}
	}
	return 0;
}
