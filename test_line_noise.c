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

Line2D_t horizLine;
Line2D_t vertLine;
Line2D_t diagLine;

PolyLine2D_t *horizPolyLine;
PolyLine2D_t *vertPolyLine;
PolyLine2D_t *diagPolyLine;

/*
 * TODO: Add variability in length
 * TODO: Make number of segments dependent on line length
 * TODO: Make noise magnitude depend on segment length
 *
 * @note caller is responsible for disposing of the polyline structure with
 * PolyLine2D_Destroy().
 */
PolyLine2D_t *GetHandDawnLine(Line2D_t *line) {
  PolyLine2D_t *pl = PolyLine2D_Create(32);
  if (pl == NULL) {
    return NULL;
  }
  const float seg_len_x =
      (line->EndPoint.x - line->StartPoint.x) / (pl->NumPoints - 1);
  const float seg_len_y =
      (line->EndPoint.y - line->StartPoint.y) / (pl->NumPoints - 1);
  const Vector2D_t orthoVector = Vector2D_Orthogonal(Vector2D_FromLine(line));

  pl->Color = line->Color;
  pl->Thickness = line->Thickness;

  // Start and end points of the polyline are the same as the input line
  pl->Points[0] = line->StartPoint;
  pl->Points[pl->NumPoints - 1] = line->EndPoint;

  // Divide the line into line segments
  for (int i = 1; i < pl->NumPoints - 1; i++) {
    pl->Points[i].x = pl->Points[i - 1].x + seg_len_x;
    pl->Points[i].y = pl->Points[i - 1].y + seg_len_y;
  }

  // This is so we don't have to sprinkle constant checks later on. It just
  // protects against a programmer that decides to use less than 3 points for
  // some reason.
  if (pl->NumPoints < 3) {
    return pl;
  }

  // Jitter all line segment points except for the first and the last one.
  // This can be combined with the previousloop, but for readability it is not.
  Line2D_t lineSeg;
  Vector2D_t segVec;         // line segment vector
  Vector2D_t segStartVec;    // vector to the start of the line segment
  float orthoVecNoiseScale;  // Multiplier for the orthogonal vector noise
  Vector2D_t noisyOrthoVec;  // orthogonal vector multiplied by the noise factor
  Vector2D_t
      noisySegVec;  // line segment vector with the noise added at the end

  for (int i = 1; i < pl->NumPoints - 1; i++) {
    lineSeg.StartPoint = pl->Points[i - 1];
    lineSeg.EndPoint = pl->Points[i];
    segVec = Vector2D_FromLine(&lineSeg);
    segStartVec = Vector2D_FromPoint(lineSeg.StartPoint);
    orthoVecNoiseScale = 2.0 - 4.0 * ((float)rand() / (float)RAND_MAX);
    noisyOrthoVec = Vector2D_Scale(orthoVector, orthoVecNoiseScale);
    noisySegVec = Vector2D_Add(noisyOrthoVec, segVec);

    pl->Points[i] = Point2D_FromVector(Vector2D_Add(segStartVec, noisySegVec));
  }
  return pl;
}

void InitCustom(void) {
  horizLine.Color = Color_FromHex(0xffff50, 0);
  horizLine.Thickness = 3.0;
  horizLine.StartPoint = (Point2D_t){.x = 100, .y = 50};
  horizLine.EndPoint = (Point2D_t){.x = WIN_WIDTH_PX - 100, .y = 50};

  vertLine.Color = Color_FromHex(0xffff50, 0);
  vertLine.Thickness = 3.0;
  vertLine.StartPoint = (Point2D_t){.x = 100, .y = 100};
  vertLine.EndPoint =
      (Point2D_t){.x = WIN_WIDTH_PX - 100, .y = WIN_HEIGHT_PX - 100};

  diagLine.Color = Color_FromHex(0xffff50, 0);
  diagLine.Thickness = 3.0;
  diagLine.StartPoint = (Point2D_t){.x = WIN_WIDTH_PX - 50, .y = 50};
  diagLine.EndPoint =
      (Point2D_t){.x = WIN_WIDTH_PX - 50, .y = WIN_HEIGHT_PX - 50};

  horizPolyLine = GetHandDawnLine(&horizLine);
  vertPolyLine = GetHandDawnLine(&vertLine);
  diagPolyLine = GetHandDawnLine(&diagLine);
}

void UpdateCustom(double dt) {}

void DrawCustom(void) {
  Polyline2D_Draw(horizPolyLine);
  Polyline2D_Draw(vertPolyLine);
  Polyline2D_Draw(diagPolyLine);
}

void TerminateCustom(void) {
  PolyLine2D_Destroy(horizPolyLine);
  PolyLine2D_Destroy(vertPolyLine);
  PolyLine2D_Destroy(diagPolyLine);
}

int main() {
  double timeNow = 0.0, prevTime = 0.0, dt = 0.0;
  ALLEGRO_EVENT ev;

  Initialize();
  InitCustom();

  while (IsRunning()) {
    al_wait_for_event(eq, &ev);
    timeNow = al_get_time();
    dt = timeNow - prevTime;

    Update(dt, &ev);
    Draw();

    prevTime = timeNow;
  }

  Terminate();
  TerminateCustom();
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