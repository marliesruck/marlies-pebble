#include <syscall.h>
/* @Bug Not sure what constitutes a valid color code
 *        -- colors are in syscall.h
 */
int main(){
	return set_term_color(BGND_LGRAY|FGND_BLACK);
}
