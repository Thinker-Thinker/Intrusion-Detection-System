/* Door logic: face pipeline per camera frame, decisions, enrolment, commands, events.
 *
 * Per frame: detect -> largest face -> (too small: "Come closer") -> liveness -> recognition,
 * embeddings averaged over 3 frames of the same face -> best match against the enrolled people.
 *   match >= threshold  -> GRANT: servo opens for 5 s, green LED, short beep
 *   below threshold     -> DENY (unknown): red LED, two beeps; 3 unknowns within a minute -> alarm
 *   3 spoof frames in a row (photo/screen) -> SPOOF: red LED + alarm
 * The exit button opens the door from inside.
 *
 * Commands (one per line, from USB or the ESP32 UART; replies go back to the sender):
 *   HELP, INFO, LIST, ENROL <name> [samples], CANCEL, DELETE <name>|ALL, THRESH <0..1>, OPEN,
 *   DEBUG 0|1, REED 0|1, SNAP, STREAM 0|1, AE <+|->, AUTO 0|1
 *   USB only: FLASHREC <len> <crc>, DATA <off> <n> <crc> + n raw bytes, FLASHEND  (tools/door.py)
 * Events (to both): "EVT GRANT <name> <score>", "EVT UNKNOWN <score>", "EVT SPOOF", "EVT ALERT ...",
 *   "EVT EXIT", "EVT MOTION", "EVT FORCED", "EVT ENROLLED <name>". */
#ifndef DOOR_H
#define DOOR_H
#include <stdint.h>

enum { DOOR_SRC_USB = 1, DOOR_SRC_UART = 2 };

void door_init(void);                       /* after xf_init(): loads the enrolled people */
void door_process(uint8_t *frame);          /* one camera frame (destroys it: the models reuse its RAM) */
void door_poll(void);                       /* often: USB commands, door timer, exit button, PIR */
void door_command(char *line, int src);
void door_reply(int src, const char *s);   /* to USB or the ESP32 link */
int  door_busy(void);                       /* 1 while a model upload is running (skip the camera) */
int  door_want_frame(void);                 /* 1 when the PC asked for this frame (SNAP / STREAM) */

/* What the LCD should show: a short message, its colour, and the last face box (frame pixels, w=0: none). */
const char *door_lcd_text(uint16_t *color);
void door_face_box(int *x, int *y, int *w, int *h, uint16_t *color);

/* Camera commands live in main.c (AE target, auto brightness). Return 1 if handled. */
int cam_command(const char *ucmd, char *out, int n);
#endif
