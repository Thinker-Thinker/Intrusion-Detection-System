/* Door administration support: time of day, the local access log, and the queue of unknown faces
 * sent to the administrator dashboard through the ESP32.
 *
 * Link with the ESP32 (USART1, 921600 baud, one line per message):
 *   door -> ESP32   LOG <seq> <time> <TYPE> <name|-> <score>      one access-log record
 *                   LOGEND <count>                                 end of a LOGSINCE reply
 *                   FACE <id> <time> <score> <closest|-> <len> <crc32>  then <len> raw bytes:
 *                        112x112x3 int8 aligned face, float scale, 512 int8 embedding
 *                   PERSON <name> <start> <end> <days> ... SCHED <on> <start> <end> <days>  PEOPLEEND
 *   ESP32 -> door   TIME <local unix seconds>   LINK UP | LINK DOWN   FACEACK <id>   LOGSINCE <seq> [max]
 *                   PEOPLE   ADMIT <id>   ADDB <name> <start> <end> <days> <scale> <base64 embedding>
 *                   WIN <name> <start> <end> <days>   DELETE <name>   SCHED <on> <start> <end> <days>   OPEN
 * Times of day are minutes after midnight (0..1439, both ends included); days is a bit mask, bit 0 = Monday. */
#ifndef ADMIN_H
#define ADMIN_H
#include <stdint.h>

enum { L_BOOT, L_GRANT, L_UNKNOWN, L_SPOOF, L_HOURS, L_CLOSED, L_EXIT, L_ADMIT, L_ENROL, L_DELETE, L_OPEN,
       L_MOTION, L_FORCED, L_NTYPES };

void adm_init(void);                       /* after xf_init(): finds the end of the log, checks the face queue */
void adm_poll(void);                       /* often: sends queued faces when the link is up */
int  adm_command(const char *ucmd, const char *arg, int src);   /* TIME LINK FACEACK LOGSINCE; 1 if handled */

uint32_t adm_now(void);                    /* local time, unix seconds; 0 until the ESP32 or the PC has set it */
int  adm_minute(void);                     /* minutes after midnight, -1 if the time is not known */
int  adm_weekday(void);                    /* 0 = Monday, -1 if not known */
int  adm_in_window(int start, int end, int days);   /* 1 inside the window, or when the time is not known */
int  adm_link_up(void);

uint32_t adm_log(int type, const char *name, float score);      /* writes a record, returns its number */
void adm_face(uint32_t id, float score, const char *closest, const int8_t *crop112, float scale, const int8_t *q512);
int  adm_face_recent(uint32_t id, uint32_t max_age_ms);         /* was this unknown face seen recently? */
int  adm_faces_waiting(void);

void adm_base64_decode(const char *in, uint8_t *out, int max, int *n);
#endif
