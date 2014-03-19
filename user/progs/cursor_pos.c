#include <syscall.h>

int main(){
	set_cursor_pos(5,4);
	int r,c;
	int result = get_cursor_pos(&r,&c);
	return result;
}
