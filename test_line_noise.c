#include <allegro5/allegro5.h>
#include <allegro5/allegro_primitives.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "noise1234.h"
#include "procgenlib.h"

#define WIN_WIDTH_PX (800)
#define WIN_HEIGHT_PX (800)
#define CENTER_X_PX (WIN_HEIGHT_PX / 2)
#define CENTER_Y_PX (WIN_WIDTH_PX / 2)

#define FPS (60.0f)

static void Initialize();
static void Terminate();
static void Draw();
static void Update(double dt, ALLEGRO_EVENT *ev);
static bool IsRunning();

static ALLEGRO_DISPLAY *display = NULL;
static ALLEGRO_EVENT_QUEUE *eq = NULL;
static ALLEGRO_TIMER *timer = NULL;
static bool redraw = true;
static bool doexit = false;

void UpdateCustom(double dt);
void DrawCustom(void);

int main() {
  double timeNow = 0.0, prevTime = 0.0, dt = 0.0;
  ALLEGRO_EVENT ev;

  Initialize();

  while (IsRunning()) {
    al_wait_for_event(eq, &ev);
    timeNow = al_get_time();
    dt = timeNow - prevTime;

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

  al_set_window_title(display, "Test");
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
    DrawCustom();
    al_flip_display();
  }
}

static void Update(double dt, ALLEGRO_EVENT *ev) {
  static double updateTimer = 0.0;
  if (ev->type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
    doexit = true;
    return;
  } else if (ev->type == ALLEGRO_EVENT_TIMER) {
    redraw = true;
  }
  UpdateCustom(dt);
}

static bool IsRunning() { return !doexit; }

void UpdateCustom(double dt) {}
void DrawCustom(void) {}
