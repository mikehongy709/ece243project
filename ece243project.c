#include "stdio.h"
#include "stdlib.h"
#include <stdbool.h>
#define SCREEN_HEIGHT 240
#define SCREEN_WIDTH 320
#define KEY_BASE 0xFF200050

struct platform
{
  int height;
  int width;
  int pos;
  short int color;
};

struct chess
{
  int x;
  int y;
  int size;
  short int color;
};

volatile int pixel_buffer_start; // global variable
short int Buffer1[240][512];     // 240 rows, 512 (320 + padding) columns
short int Buffer2[240][512];
volatile int *pixel_crtl_ptr = (int *)0xFF203020;
volatile int *Switches = (int *)0xff200040;
volatile int *key_ptr = (int *)KEY_BASE;

void plot_pixel(int x, int y, short int color);
void clear_screen();
void wait_sync();
void draw_line(int x0, int y0, int x1, int y1, short int color);
void swap(int *a, int *b);
void draw_rect(int x0, int y0, int height, int width, short int color);
void draw_platform(struct platform *p);
int get_time_ms();
int get_jump_height();
void draw_circle(int x, int y, int radius, short int color);
void draw_triangle(int x0, int y0, int x1, int y1, int x2, int y2,
                   short int color);
void draw_chess_piece(struct chess *c);
void process_player(struct chess *c);
void process_platform(struct platform *p);
bool is_on_platform(int x, struct platform p);
void reset_game();
void show_game_over();
void draw_image(const unsigned short int pixels[240][320], int h, int w);

int difficulty;
int moving_speed;
int pos1;
int pos2;
int pos3;
int width1;
int width2;
int width3;
struct platform p1 = {100, 0, 0, 0xffff};
struct platform p2 = {100, 0, 0, 0xffff};
struct platform p3 = {100, 0, 0, 0xffff};
struct chess player = {10, 131, 16, 0xff00};
int press_start;
int press_duration;
int jump_distance;
int initial_distance;
bool is_pressed;
bool landed = true;
bool game_over = false;

int main(void)
{
  /* set front pixel buffer to Buffer 1 */
  *(pixel_crtl_ptr + 1) = (int)&Buffer1; // first store the address in the  back buffer
  /* now, swap the front/back buffers, to set the front buffer location */
  wait_sync();
  /* initialize a pointer to the pixel buffer, used by drawing functions */
  pixel_buffer_start = *pixel_crtl_ptr;
  clear_screen(); // pixel_buffer_start points to the pixel buffer

  /* set back pixel buffer to Buffer 2 */
  *(pixel_crtl_ptr + 1) = (int)&Buffer2;
  pixel_buffer_start = *(pixel_crtl_ptr + 1); // we draw on the back buffer
  clear_screen();                             // pixel_buffer_start points to the pixel buffer
  difficulty = 50;
  moving_speed = 5;
  pos1 = 250;
  pos2 = 150;
  pos3 = 10;
  width1 = rand() % difficulty + 10;
  width2 = rand() % difficulty + 10;
  width3 = rand() % difficulty + 10;
  p1.width = width1;
  p2.width = width2;
  p3.width = width3;
  p1.pos = pos1;
  p2.pos = pos2;
  p3.pos = pos3;

  press_start = 0;
  press_duration = 0;
  jump_distance = 0;
  initial_distance = 0;
  is_pressed = false;
  *(key_ptr + 3) = 15;
  while (1)
  {
    while (1)
    {
      clear_screen();
      draw_rect(0, 0, 200, 300, 0xff00);
      int start = *(key_ptr + 3);
      if ((start & 8) != 0)
      {
        reset_game();
        *(key_ptr + 3) = start;
        *(key_ptr) = 0;
        break;
      }
      wait_sync();
      pixel_buffer_start = *(pixel_crtl_ptr + 1);
    }

    while (1)
    {

      clear_screen();

      int key_status = *key_ptr & 0x01;
      if (key_status != 0)
      {
        if (!is_pressed)
        {
          press_start = get_time_ms();
          is_pressed = true;
        }

        int current_time = get_time_ms();
        int current_duration = current_time - press_start;
        int current_distance = current_duration / 10;
        if (current_distance > 310)
        {
          current_distance = 310;
        }

        draw_line(10, 140, 10 + current_distance, 140, 0xFB00);
      }
      else
      {
        if (is_pressed)
        {
          int press_end = get_time_ms();
          press_duration = press_end - press_start;

          jump_distance = press_duration / 10;
          initial_distance = jump_distance;

          is_pressed = false;
          landed = false;
        }
      }

      draw_platform(&p1);
      draw_platform(&p2);
      draw_platform(&p3);
      process_platform(&p1);
      process_platform(&p2);
      process_platform(&p3);

      process_player(&player);
      jump_distance -= moving_speed;
      if (jump_distance <= 0)
      {
        jump_distance = 0;
        player.y = 132;
      }
      if (jump_distance <= 0 && landed == false)
      {
        int piece_x = 10 + initial_distance;
        if (!(is_on_platform(piece_x, p1) || is_on_platform(piece_x, p2) || is_on_platform(piece_x, p3)))
        {
          game_over = true;
        }
        else
        {
          landed = true;
        }
      }

      draw_chess_piece(&player);
      if (game_over)
      {
        break;
      }
      wait_sync();
      pixel_buffer_start = *(pixel_crtl_ptr + 1);
    }

    while (1)
    {
      clear_screen();
      show_game_over();
      int restart = *(key_ptr + 3);

      if ((restart & 2) != 0)
      {
        *(key_ptr + 3) = restart;
        break;
        // reset_game();
        // while (*key_ptr & 0x02) {}
      }

      wait_sync();
      pixel_buffer_start = *(pixel_crtl_ptr + 1);
    }
  }
}

void draw_image(const unsigned short int pixels[240][320], int h, int w)
{
  for (int i = 0; i < h; i++)
  {
    for (int j = 0; j < w; j++)
    {
      int x = j;
      int y = i;
      plot_pixel(x, y, pixels[i][j]);
    }
  }
}

void plot_pixel(int x, int y, short int line_color)
{
  if (x < 0 || x >= SCREEN_WIDTH || y < 0 || y >= SCREEN_HEIGHT)
  {
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

void wait_sync()
{
  int status;

  *pixel_crtl_ptr = 1;
  status = *(pixel_crtl_ptr + 3);
  while ((status & 0x01) != 0)
  {
    status = *(pixel_crtl_ptr + 3);
  }
}

void draw_line(int x0, int y0, int x1, int y1, short int color)
{
  int is_steep = abs(y1 - y0) > abs(x1 - x0);
  if (is_steep)
  {
    swap(&x0, &y0);
    swap(&x1, &y1);
  }
  if (x0 > x1)
  {
    swap(&x0, &x1);
    swap(&y0, &y1);
  }
  int dx = x1 - x0;
  int dy = abs(y1 - y0);
  int err = -(dx / 2);
  int y = y0;
  int y_step = 0;
  if (y0 < y1)
  {
    y_step = 1;
  }
  else
  {
    y_step = -1;
  }

  for (int x = x0; x <= x1; x++)
  {
    if (is_steep)
    {
      plot_pixel(y, x, color);
    }
    else
    {
      plot_pixel(x, y, color);
    }
    err += dy;
    if (err > 0)
    {
      y += y_step;
      err -= dx;
    }
  }
}

void swap(int *a, int *b)
{
  int temp = *a;
  *a = *b;
  *b = temp;
}

void draw_rect(int x0, int y0, int height, int width, short int color)
{
  for (int i = 0; i < height; i++)
  {
    for (int j = 0; j < width; j++)
    {
      plot_pixel(x0 + i, y0 + j, color);
    }
  }
}

void draw_platform(struct platform *p)
{
  for (int i = 0; i < p->height; i++)
  {
    for (int j = 0; j < p->width; j++)
    {
      plot_pixel(p->pos + (j - p->width / 2), SCREEN_HEIGHT - 1 - i, p->color);
    }
  }
}

int get_time_ms()
{
  static int count = 0;
  count += 30;
  return count;
}

int get_jump_height(int jump, int initial)
{
  int t0 = initial / moving_speed;
  int v = 0.3 * t0;
  int t = (initial - jump) * t0 / initial;
  return v * t - 0.3 * t * t;
  // return 0;
}

void draw_triangle(int x0, int y0, int x1, int y1, int x2, int y2,
                   short int color)
{
  if (y0 > y1)
  {
    swap(&x0, &x1);
    swap(&y0, &y1);
  }
  if (y1 > y2)
  {
    swap(&x1, &x2);
    swap(&y1, &y2);
  }
  if (y0 > y1)
  {
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

  if (y1 > y0)
  {
    for (int y = y0; y < y1; y++)
    {
      for (int x = (int)sx; x <= (int)ex; x++)
      {
        plot_pixel(x, y, color);
      }
      sx += dx1;
      ex += dx2;
    }
  }

  sx = x1;
  if (y2 > y1)
  {
    for (int y = y1; y <= y2; y++)
    {
      for (int x = (int)sx; x <= (int)ex; x++)
      {
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

void draw_circle(int x, int y, int radius, short int color)
{
  for (int i = -radius; i <= radius; i++)
  {
    for (int j = -radius; j <= radius; j++)
    {
      if (i * i + j * j <= radius * radius)
      {
        plot_pixel(x + i, y + j, color);
      }
    }
  }
}

void draw_chess_piece(struct chess *p)
{
  int head_radius = p->size / 4;
  int body_width = p->size / 2;
  int body_height = p->size / 3;
  int base_width = p->size * 3 / 4;
  int base_height = p->size / 3;

  int total_height = head_radius * 2 + body_height + base_height;
  int head_y = p->y - total_height / 2 + head_radius;

  int body_y = head_y + head_radius;
  int base_y = body_y + body_height;

  draw_circle(p->x, head_y, head_radius, p->color);

  int body_x = p->x - body_width / 2;
  draw_rect(body_x, body_y, body_width, body_height, p->color);

  int base_x1 = p->x - base_width / 2;
  int base_x2 = p->x + base_width / 2;
  int base_y2 = base_y + base_height;
  draw_triangle(base_x1, base_y2, base_x2, base_y2, p->x, base_y, p->color);
}

void process_platform(struct platform *p)
{
  if (jump_distance > 0)
  {
    p->pos -= moving_speed;
  }
  if (p->pos + p->width / 2 <= 0)
  {
    p->width = rand() % difficulty + 10;
    p->pos = SCREEN_WIDTH + p->width / 2;
  }
}

void process_player(struct chess *p)
{
  if (jump_distance > 0)
  {
    p->y = 131 - get_jump_height(jump_distance, initial_distance);
  }
}

bool is_on_platform(int x, struct platform p)
{
  int platform_left = p.pos - p.width / 2;
  int platform_right = p.pos + p.width / 2;

  if (x >= platform_left && x <= platform_right)
  {
    return true;
  }
  return false;
}

void reset_game()
{
  game_over = false;
  landed = true;
  jump_distance = 0;
  initial_distance = 0;
  is_pressed = false;
  player.y = 131;

  pos1 = 250;
  pos2 = 150;
  pos3 = 10;
  width1 = rand() % difficulty + 10;
  width2 = rand() % difficulty + 10;
  width3 = rand() % difficulty + 10;

  p1.pos = pos1;
  p2.pos = pos2;
  p3.pos = pos3;
  p1.width = width1;
  p2.width = width2;
  p3.width = width3;
}

void show_game_over()
{
  clear_screen();

  draw_rect(60, 60, 150, 150, 0xF800);

  // G
  draw_rect(80, 100, 5, 20, 0xFFFF); // left
  draw_rect(80, 100, 20, 5, 0xFFFF); // top
  draw_rect(80, 120, 20, 5, 0xFFFF); // bottom
  draw_rect(95, 110, 5, 15, 0xFFFF); // right
  draw_rect(90, 110, 10, 5, 0xFFFF); // middle

  // A
  draw_rect(110, 100, 20, 5, 0xFFFF); // top
  draw_rect(110, 100, 5, 25, 0xFFFF); // left
  draw_rect(125, 100, 5, 25, 0xFFFF); // right
  draw_rect(110, 110, 20, 5, 0xFFFF); // middle

  // M
  draw_rect(140, 100, 5, 25, 0xFFFF); // left
  draw_rect(160, 100, 5, 25, 0xFFFF); // right
  // left thick line
  draw_line(145, 100, 153, 110, 0xFFFF);
  draw_line(144, 100, 152, 110, 0xFFFF);
  draw_line(146, 100, 154, 110, 0xFFFF);
  draw_line(143, 100, 151, 110, 0xFFFF);
  draw_line(147, 100, 155, 110, 0xFFFF);
  // right thick line
  draw_line(150, 110, 160, 100, 0xFFFF);
  draw_line(149, 110, 159, 100, 0xFFFF);
  draw_line(151, 110, 161, 100, 0xFFFF);
  draw_line(148, 110, 158, 100, 0xFFFF);
  draw_line(152, 110, 162, 100, 0xFFFF);

  // E
  draw_rect(175, 100, 5, 25, 0xFFFF); // left
  draw_rect(175, 100, 20, 5, 0xFFFF); // top
  draw_rect(175, 110, 15, 5, 0xFFFF); // middle
  draw_rect(175, 120, 20, 5, 0xFFFF); // bottom

  // O
  draw_rect(80, 140, 5, 25, 0xFFFF);  // left
  draw_rect(100, 140, 5, 25, 0xFFFF); // right
  draw_rect(80, 140, 25, 5, 0xFFFF);  // top
  draw_rect(80, 160, 25, 5, 0xFFFF);  // bottom

  // V
  // left line thick
  draw_line(110, 140, 120, 160, 0xFFFF);
  draw_line(109, 140, 119, 160, 0xFFFF);
  draw_line(111, 140, 121, 160, 0xFFFF);
  draw_line(108, 140, 118, 160, 0xFFFF);
  draw_line(112, 140, 122, 160, 0xFFFF);
  // right line thick
  draw_line(120, 160, 130, 140, 0xFFFF);
  draw_line(119, 160, 129, 140, 0xFFFF);
  draw_line(121, 160, 131, 140, 0xFFFF);
  draw_line(118, 160, 128, 140, 0xFFFF);
  draw_line(122, 160, 132, 140, 0xFFFF);

  // E
  draw_rect(140, 140, 5, 25, 0xFFFF); // left
  draw_rect(140, 140, 20, 5, 0xFFFF); // top
  draw_rect(140, 150, 15, 5, 0xFFFF); // middle
  draw_rect(140, 160, 20, 5, 0xFFFF); // bottom

  // R
  draw_rect(170, 140, 5, 25, 0xFFFF); // left
  draw_rect(170, 140, 20, 5, 0xFFFF); // top
  draw_rect(170, 150, 20, 5, 0xFFFF); // middle
  draw_rect(185, 140, 5, 15, 0xFFFF); // right
  // right thick line
  draw_line(175, 150, 190, 165, 0xFFFF);
  draw_line(174, 150, 189, 165, 0xFFFF);
  draw_line(176, 150, 191, 165, 0xFFFF);
  draw_line(177, 150, 192, 165, 0xFFFF);
  draw_line(173, 150, 188, 165, 0xFFFF);
}