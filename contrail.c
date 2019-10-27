#include <allegro5/allegro5.h>
#include <allegro5/allegro_primitives.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "noise1234.h"

#define WIN_WIDTH_PX (800)
#define WIN_HEIGHT_PX (800)
#define CENTER_X_PX (WIN_HEIGHT_PX / 2)
#define CENTER_Y_PX (WIN_WIDTH_PX / 2)
#define RADIUS_PX (WIN_WIDTH_PX / 2)

#define FPS (60.0f)

static ALLEGRO_DISPLAY *display = NULL;
static ALLEGRO_EVENT_QUEUE *eq = NULL;
static ALLEGRO_TIMER *timer = NULL;
static bool redraw = true;
static bool doexit = false;

static void Initialize();
static void Terminate();
static void Draw();
static void ProcessInput(ALLEGRO_EVENT *ev);
static void Update(double dt, ALLEGRO_EVENT *ev);
static bool IsRunning();

struct Particle {
  float x;
  float y;
  ALLEGRO_COLOR color;
  unsigned int size;
  float src_x;
  float src_y;
  float vel_x;
  float vel_y;
  float vel_mag;    // in units of pixels/sec
  float vel_angle;  // in units of degrees
  float dst_x;
  float dst_y;
  float lerp_duration;  // how many seconds the particle takes to travel from
                        // src to dst. Calculated from velocity
  float lerp_time;      // how many seconds the particle has been traveling from
                        // src.
};

struct Line {
  ALLEGRO_COLOR color;
  float src_x;
  float src_y;
  float dst_x;
  float dst_y;
};

#define NUM_LINE_SEGS (16)
#define NUM_CONTRAILS (2)

struct Contrail {
  struct Line main_line;
  struct Line line_segs[NUM_LINE_SEGS];
};

void Line_BreakIntoSegs(struct Line *sl, struct Line *segs,
                        unsigned int num_segs);
void AddNoiseToLineSegs(struct Line *segs, unsigned int num_segs);
void Init_Contrails(void);
void Contrail_Draw(struct Contrail *c);

struct Contrail contrails[NUM_CONTRAILS];

void Particle_Update(struct Particle *p, double dt);
void Particle_Draw(struct Particle *p);
void Particle_SetRandomVelocity(struct Particle *p);
void Init_Particles(void);
float Lerp(float start, float end, float percent);
float EaseOut(float t);

#define NUM_PARTICLES (50)
struct Particle particles[NUM_PARTICLES];

int main() {
  double timeNow = 0.0, prevTime = 0.0, dt = 0.0;
  ALLEGRO_EVENT ev;

  Initialize();
  Init_Particles();
  Init_Contrails();

  while (IsRunning()) {
    al_wait_for_event(eq, &ev);
    timeNow = al_get_time();
    dt = timeNow - prevTime;

    ProcessInput(&ev);
    Update(dt, &ev);
    Draw();

    prevTime = timeNow;
  }

  Terminate();
  return 0;
}

static void Initialize() {
  time_t t;
  srand(time(&t));

  if (!al_init()) {
    fprintf(stderr, "ERROR: Failed to initialize allegro!\n");
    goto init_fail;
  }

  if (!al_init_primitives_addon()) {
    fprintf(stderr, "ERROR: Failed to load primitives addon!\n");
    goto prim_addon_fail;
  }

  timer = al_create_timer(1.0 / FPS);
  if (!timer) {
    fprintf(stderr, "ERROR: Failed to create timer!\n");
    goto timer_fail;
  }

  display = al_create_display(WIN_WIDTH_PX, WIN_HEIGHT_PX);
  if (!display) {
    fprintf(stderr, "ERROR: Failed to create display!\n");
    goto disp_fail;
  }

  eq = al_create_event_queue();
  if (!eq) {
    fprintf(stderr, "ERROR: Failed to create event queue!\n");
    goto eq_fail;
  }

  al_register_event_source(eq, al_get_display_event_source(display));
  al_register_event_source(eq, al_get_timer_event_source(timer));

  al_set_window_title(display, "Contrails");
  al_start_timer(timer);

  return;

eq_fail:
disp_fail:
  al_destroy_timer(timer);
timer_fail:
prim_addon_fail:
init_fail:
  exit(1);
}

static void Terminate() {
  al_destroy_event_queue(eq);
  al_destroy_display(display);
  al_destroy_timer(timer);
}

static void Draw() {
  if (redraw && al_is_event_queue_empty(eq)) {
    redraw = false;
    al_clear_to_color(al_map_rgb(0x30, 0x34, 0x3f));

#if 0
    for (int i = 0; i < NUM_PARTICLES; i++) {
      Particle_Draw(&particles[i]);
    }
#endif
#if 0
    al_draw_line(200, 400, 600, 400, al_map_rgb(0xff, 0x70, 0x3b),
                 8 * (noise1(al_get_time()) + 1));
#endif
#if 0
    for (int i = 0; i < NUM_CONTRAILS; i++) {
      Contrail_Draw(&contrails[i]);
    }
#endif

    {
      float xs = 400;
      float xy = 400;
      float len = 100;
      float angle = M_PI / 128;
      float base_len = 2 * len * tan(angle);

      al_draw_triangle(xs - base_len / 2, xy, xs + base_len / 2, xy, xs,
                       xy - len, al_map_rgb(0x70, 0x70, 0x70), 1);
    }
    al_flip_display();
  }
}

static void ProcessInput(ALLEGRO_EVENT *ev) {}

static void Update(double dt, ALLEGRO_EVENT *ev) {
  static double updateTimer = 0.0;
  if (ev->type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
    doexit = true;
    return;
  } else if (ev->type == ALLEGRO_EVENT_TIMER) {
    redraw = true;
  }

  for (int i = 0; i < NUM_PARTICLES; i++) {
    Particle_Update(&particles[i], dt);
  }
}

static bool IsRunning() { return !doexit; }

void Particle_Update(struct Particle *p, double dt) {
  p->lerp_time += dt;
  if (p->lerp_time > p->lerp_duration) {
    p->x = p->src_x;
    p->y = p->src_y;
    Particle_SetRandomVelocity(p);
  } else {
    p->x = Lerp(p->src_x, p->dst_x, EaseOut(p->lerp_time / p->lerp_duration));
    p->y = Lerp(p->src_y, p->dst_y, EaseOut(p->lerp_time / p->lerp_duration));
  }
}

void Particle_Draw(struct Particle *p) {
  al_draw_line(p->src_x, p->src_y, p->x, p->y, p->color, 1);

  // Draw the head of the line (the actual particle) centered at (p->x, p->y)
  al_draw_filled_rectangle(p->x - p->size / 2, p->y - p->size / 2,
                           p->x + p->size / 2, p->y + p->size / 2, p->color);
}

void Init_Particles(void) {
  for (int i = 0; i < NUM_PARTICLES; i++) {
    particles[i].size = 5;
    particles[i].color = al_map_rgb(0xff, rand() % 0xff, 0x38);
    particles[i].src_x = CENTER_X_PX;
    particles[i].src_y = CENTER_Y_PX;
    particles[i].x = CENTER_X_PX;
    particles[i].y = CENTER_Y_PX;
    Particle_SetRandomVelocity(&particles[i]);
  }
}

void Particle_SetRandomVelocity(struct Particle *p) {
  p->vel_mag = 50.0 + ((float)rand() / RAND_MAX) * 100.0;
  p->vel_angle = ((float)rand() / RAND_MAX) * 2.0 * M_PI;
  p->vel_x = cos(p->vel_angle) * p->vel_mag;
  p->vel_y = sin(p->vel_angle) * p->vel_mag;
  p->dst_x = p->src_x + cos(p->vel_angle) * RADIUS_PX;
  p->dst_y = p->src_y + sin(p->vel_angle) * RADIUS_PX;
  p->lerp_duration = sqrtf((p->dst_x - p->src_x) * (p->dst_x - p->src_x) +
                           (p->dst_y - p->src_y) * (p->dst_y - p->src_y)) /
                     p->vel_mag;
  p->lerp_time = 0.0;
}

float Lerp(float start, float end, float percent) {
  return (start + (end - start) * percent);
}

float EaseOut(float t) { return (1 - (1 - t) * (1 - t)); }

void Line_BreakIntoSegs(struct Line *sl, struct Line *segs,
                        unsigned int num_segs) {
  float len_x = (sl->dst_x - sl->src_x) / num_segs;
  float len_y = (sl->dst_y - sl->src_y) / num_segs;

  for (int i = 0; i < num_segs; i++) {
    segs[i].src_x = sl->src_x + i * len_x;
    segs[i].src_y = sl->src_y + i * len_y;
    segs[i].dst_x = sl->src_x + (i + 1) * len_x;
    segs[i].dst_y = sl->src_y + (i + 1) * len_y;
  }
}
void AddNoiseToLineSegs(struct Line *segs, unsigned int num_segs) {
  if (num_segs == 0) {
    return;
  }

  // maximum amount of noise is proportional to the length of a line segment
  float seg_len =
      sqrt((segs[0].dst_x - segs[0].src_x) * (segs[0].dst_x - segs[0].src_x) +
           (segs[0].dst_y - segs[0].src_y) * (segs[0].dst_y - segs[0].src_y));
  float max_noise_mult = 1.0;
  float max_noise;
  float dx;
  float dy;
  for (int i = 1; i < num_segs; i++) {
    max_noise =
        seg_len * Lerp(1.0, 0.0, EaseOut(((float)i - 1.0) / (float)num_segs));
    dx = max_noise / 2 - max_noise * (float)rand() / (float)RAND_MAX;
    dy = max_noise / 2 - max_noise * (float)rand() / (float)RAND_MAX;
    segs[i].src_x += dx;
    segs[i].src_y += dy;
    segs[i - 1].dst_x = segs[i].src_x;
    segs[i - 1].dst_y = segs[i].src_y;
  }
}

void Init_Contrails(void) {
  for (int i = 0; i < NUM_CONTRAILS; i++) {
    contrails[i].main_line.color = al_map_rgb(0x80, 0x80, 0x80);
    if (i == 0) {
      contrails[i].main_line.src_x = 200;
      contrails[i].main_line.src_y = 400;
      contrails[i].main_line.dst_x = 600;
      contrails[i].main_line.dst_y = 400;
    } else {
      contrails[i].main_line.src_x = 600;
      contrails[i].main_line.src_y = 500;
      contrails[i].main_line.dst_x = 200;
      contrails[i].main_line.dst_y = 700;
    }
    Line_BreakIntoSegs(&contrails[i].main_line, contrails[i].line_segs,
                       NUM_LINE_SEGS);
    AddNoiseToLineSegs(contrails[i].line_segs, NUM_LINE_SEGS);
  }
}

void Contrail_Draw(struct Contrail *c) {
  al_draw_line(c->main_line.src_x, c->main_line.src_y, c->main_line.dst_x,
               c->main_line.dst_y, c->main_line.color, 1.0);
  for (int i = 0; i < NUM_LINE_SEGS; i++) {
    al_draw_line(c->line_segs[i].src_x, c->line_segs[i].src_y,
                 c->line_segs[i].dst_x, c->line_segs[i].dst_y,
                 al_map_rgb(0xff, 0xff, 0x38), 1.0);
  }
}
