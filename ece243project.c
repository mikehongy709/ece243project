#include "stdio.h"
#include "stdlib.h"
#include <stdbool.h>
#define SCREEN_HEIGHT 240
#define SCREEN_WIDTH 320
#define KEY_BASE 0xFF200050

volatile int pixel_buffer_start; // global variable
short int Buffer1[240][512]; // 240 rows, 512 (320 + padding) columns
short int Buffer2[240][512];
volatile int * pixel_crtl_ptr = (int *)0xFF203020;
volatile int* Switches = (int*)0xff200040;
volatile int *key_ptr = (int *)KEY_BASE;

void plot_pixel(int x, int y, short int color);
void clear_screen();
void wait_sync();
void draw_line(int x0, int y0, int x1, int y1, short int color);
void swap(int* a, int *b);
void draw_rect(int x0, int y0, int height, int width, short int color);
void draw_platform(int height, int width, int pos, short int color);
int get_time_ms();
int get_jump_height();
void draw_circle(int x, int y, int radius, short int color);
void draw_triangle(int x0, int y0, int x1, int y1, int x2, int y2,
                   short int color);
void draw_chess_piece(int x, int y, int size, short int color);


struct platform{
	int height;
	int width;
	int pos;
	short int color; 
};

int main(void)
{
	/* set front pixel buffer to Buffer 1 */
    *(pixel_crtl_ptr + 1) = (int) &Buffer1; // first store the address in the  back buffer
    /* now, swap the front/back buffers, to set the front buffer location */
    wait_sync();
    /* initialize a pointer to the pixel buffer, used by drawing functions */
    pixel_buffer_start = *pixel_crtl_ptr;
    clear_screen(); // pixel_buffer_start points to the pixel buffer

    /* set back pixel buffer to Buffer 2 */
    *(pixel_crtl_ptr + 1) = (int) &Buffer2;
    pixel_buffer_start = *(pixel_crtl_ptr + 1); // we draw on the back buffer
    clear_screen(); // pixel_buffer_start points to the pixel buffer
	int difficulty = 50;
	
	int moving_speed = 5;
	int pos1 = 250;
	int pos2 = 150;
	int pos3 = 10;
	int width1 = rand() % difficulty + 10;
	int width2 = rand() % difficulty + 10;
	int width3 = rand() % difficulty + 10;
	struct platform p1 = {100, width1, pos1, 0xffff};
	struct platform p2 = {100, width2, pos2, 0xffff};
	struct platform p3 = {100, width3, pos3, 0xffff};
	
	int press_start = 0;
  	int press_duration = 0;
  	int jump_distance = 0;
	int initial_distance;
  	bool is_pressed = false;
	
	
    while (1) {
		clear_screen();
		
		int key_status = *key_ptr & 0x01;
    if (key_status != 0) {
      if (!is_pressed) {
        press_start = get_time_ms();
        is_pressed = true;
      }

      int current_time = get_time_ms();
      int current_duration = current_time - press_start;
      int current_distance = current_duration / 10;
      if (current_distance > 310) {
        current_distance = 310;
      }

      draw_line(10, 140, 10 + current_distance, 140, 0xFB00);
    } else {
      if (is_pressed) {
        int press_end = get_time_ms();
        press_duration = press_end - press_start;

        jump_distance = press_duration / 10;
		initial_distance = jump_distance;

        is_pressed = false;
      }
    }

		
		int move = *Switches & 1;
		draw_platform(100, width1, pos1, 0xffff);
		draw_platform(100, width2, pos2, 0xffff);
		draw_platform(100, width3, pos3, 0xffff);
		if (move || jump_distance >= 0) {
			pos1 -= moving_speed;
			pos2 -= moving_speed;
			pos3 -= moving_speed;
			jump_distance -= moving_speed;
		}
		if (pos1 + width1 / 2 <= 0) {
			width1 = rand() % difficulty + 10;
			pos1 = SCREEN_WIDTH + width1 / 2;
		}
		if (pos2 + width2 / 2 <= 0) {
			width2 = rand() % difficulty + 10;
			pos2 = SCREEN_WIDTH + width2 / 2;
		}
		if (pos3 + width3 / 2 <= 0) {
			width3 = rand() % difficulty + 10;
			pos3 = SCREEN_WIDTH + width3 / 2;
		}
		draw_chess_piece(10, get_jump_height(jump_distance, initial_distance) - 10, 15, 0xff00);
		wait_sync();
		pixel_buffer_start = *(pixel_crtl_ptr + 1);
	}
}    

void plot_pixel(int x, int y, short int line_color)
{
	if (x < 0 || x >= SCREEN_WIDTH || y < 0 || y >= SCREEN_HEIGHT) {
		return;
	}
    volatile short int *one_pixel_address;
        
        one_pixel_address = pixel_buffer_start + (y << 10) + (x << 1);
        
        *one_pixel_address = line_color;
}

void clear_screen()
{
    for (int y = 0; y < 240; y++)
    {
        for (int x = 0; x < 320; x++)
        {
            plot_pixel(x, y, 0x0000); 
        }
    }
}

void wait_sync() {
	int status;
	
	*pixel_crtl_ptr = 1;
	status = *(pixel_crtl_ptr + 3);
	while((status & 0x01) != 0) {
		status = *(pixel_crtl_ptr + 3);
	}
}

void draw_line(int x0, int y0, int x1, int y1, short int color)
{
    int is_steep = abs(y1 - y0) > abs(x1 - x0);
	if (is_steep) {
		swap(&x0, &y0);
		swap(&x1, &y1);
	} 
	if (x0 > x1) {
		swap(&x0, &x1);
		swap(&y0, &y1);
	}
	int dx = x1 - x0;
	int dy = abs(y1 - y0);
	int err = -(dx / 2);
	int y = y0;
	int y_step = 0;
	if (y0 < y1) {
		y_step = 1;
	} else {
		y_step = -1;
	}
	
	for (int x = x0; x <= x1; x++) {
		if (is_steep) {
			plot_pixel(y, x, color);
		} else {
			plot_pixel(x, y, color);
		}
		err += dy;
		if (err > 0) {
			y += y_step;
			err -= dx;
		}
	}
}



void swap(int* a, int* b) {
	int temp = *a;
	*a = *b;
	*b = temp;
}

void draw_rect(int x0, int y0, int height, int width, short int color) {
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			plot_pixel(x0 + i, y0 + j, color);
		}
	}
}

void draw_platform(int height, int width, int pos, short int color) {
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			plot_pixel(pos + (j - width / 2), SCREEN_HEIGHT - 1 - i, color);
		}
	}
}


int get_time_ms() {
  static int count = 0;
  count += 30;
  return count;
}

int get_jump_height(int jump, int initial) {
	if (jump < initial / 2) {
		return 140 - (jump * jump) / 50;
	} else {
		return 140 - (jump - initial) * (jump - initial) / 50;
	}
	return 0;
}


void draw_triangle(int x0, int y0, int x1, int y1, int x2, int y2,
                   short int color) {
  if (y0 > y1) {
    swap(&x0, &x1);
    swap(&y0, &y1);
  }
  if (y1 > y2) {
    swap(&x1, &x2);
    swap(&y1, &y2);
  }
  if (y0 > y1) {
    swap(&x0, &x1);
    swap(&y0, &y1);
  }

  float dx1, dx2, dx3;
  if (y1 - y0 != 0)
    dx1 = (float)(x1 - x0) / (y1 - y0);
  else
    dx1 = 0;
  if (y2 - y0 != 0)
    dx2 = (float)(x2 - x0) / (y2 - y0);
  else
    dx2 = 0;
  if (y2 - y1 != 0)
    dx3 = (float)(x2 - x1) / (y2 - y1);
  else
    dx3 = 0;

  float sx = x0, ex = x0;

  if (y1 > y0) {
    for (int y = y0; y < y1; y++) {
      for (int x = (int)sx; x <= (int)ex; x++) {
        plot_pixel(x, y, color);
      }
      sx += dx1;
      ex += dx2;
    }
  }

  sx = x1;
  if (y2 > y1) {
    for (int y = y1; y <= y2; y++) {
      for (int x = (int)sx; x <= (int)ex; x++) {
        plot_pixel(x, y, color);
      }
      sx += dx3;
      ex += dx2;
    }
  }

  draw_line(x0, y0, x1, y1, color);
  draw_line(x1, y1, x2, y2, color);
  draw_line(x2, y2, x0, y0, color);
}

void draw_circle(int x, int y, int radius, short int color) {
  for (int i = -radius; i <= radius; i++) {
    for (int j = -radius; j <= radius; j++) {
      if (i * i + j * j <= radius * radius) {
        plot_pixel(x + i, y + j, color);
      }
    }
  }
}

void draw_chess_piece(int x, int y, int size, short int color) {
  int head_radius = size / 4;
  int body_width = size / 2;
  int body_height = size / 3;
  int base_width = size * 3 / 4;
  int base_height = size / 3;

  int total_height = head_radius * 2 + body_height + base_height;
  int head_y = y - total_height / 2 + head_radius;

  int body_y = head_y + head_radius;
  int base_y = body_y + body_height;

  draw_circle(x, head_y, head_radius, color);

  int body_x = x - body_width / 2;
  draw_rect(body_x, body_y, body_width, body_height, color);

  int base_x1 = x - base_width / 2;
  int base_x2 = x + base_width / 2;
  int base_y2 = base_y + base_height;
  draw_triangle(base_x1, base_y2, base_x2, base_y2, x, base_y, color);
}


