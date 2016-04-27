#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main() {
	int fd = open("./main.c", O_RDONLY);

	size_t right = 140000000000000000;
	size_t result = 0;
	size_t left = 0;
	while(left != right) {
		size_t middle = (left + right) / 2;
		void* map = mmap(0, middle, PROT_READ, MAP_SHARED, fd, 0);
		if (map == (void*) -1) {
			right = middle;
		}
		else {
			result += middle;
			left = middle;
		}
	}
	
	printf("%zd\n", result);
	return 0;
}
