/** @file keyboard.h
 *
 *  @brief Declares the keyboard driver API.
 *
 *  @author Enrique Naudon (esn)
 *
 *  @bug No known bugs.
 */

#ifndef __KEYBOARD_H__
#define __KEYBOARD_H__

int kbd_getchar(void);
int kbd_getline(int size, char *buf);
void kbd_putchar(char scancode);

#endif /* __KEYBOARD_H__ */
