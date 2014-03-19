/* @Bug problems linking with stdio so getchar() isn't actually reading from 
 * file stream 
 * */
#include <syscall.h>
#include <simics.h>
int main(){
	char ch = getchar();
	char *test = &ch;
	print(1,test);
	return 5;
}
