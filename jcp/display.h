/* display.h -- the display module public interface */
/* ver. 1.03 */
#ifndef DISPLAY_H
#define DISPLAY_H

#define FRAME_ROWS 24	// 24 lines
#define FRAME_COLS 79	// 79 characters in each line

void disp_clear(void);
/* returns: Nothing.
 * 
 * description: Clears the console screen. */

void disp_init_frame(void);
/* returns: Nothing.
 * 
 * description: Initializes the frame buffer, filling in constant
 * information. */


enum {HEX_DSP, DEC_DSP};
/* enum constants for base conversion */

void disp_print(int hex_dec, int last_instr);
/* returns: Nothing.
 * 
 * description: Prints the current state of the jcpu. hex_dec specifies if
 * the ram and the registers should be printed in hex or in decimal. last_instr
 * is the value of the IAR register from the previous cpu step. */

void disp_move_cursor_xy(int row, int col);
/* returns: Nothing.
 * 
 * description: Moves the terminal text cursor to the row-th row and 
 * the col-th column. */
#endif

