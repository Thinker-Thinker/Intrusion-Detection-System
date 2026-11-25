/* Windows (lld-link) build only: lld resolves symbols referenced by code that the GNU linker would have
 * dropped as unused. These functions are never called by the door models; they abort if they ever are. */
#include <stdlib.h>
void arm_nn_mat_mult_kernel_s8_s16_4col(void) { abort(); }
void mat_mult_kernel_s8_s16_reordered(void) { abort(); }
void get_kernel_buffer(void) { abort(); }
unsigned __QSUB16(unsigned a, unsigned b) { (void)a; (void)b; abort(); }
unsigned __QSUB8(unsigned a, unsigned b) { (void)a; (void)b; abort(); }
