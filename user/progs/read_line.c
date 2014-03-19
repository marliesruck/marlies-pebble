#include <syscall.h>
#include <simics.h>

#define LEN	64

int main(){
	char buf[LEN];
	readline(LEN,buf);
	print(LEN,buf);

	return 0;
}
