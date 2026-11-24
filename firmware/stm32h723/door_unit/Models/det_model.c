/* Automatically generated source file */
#include <float.h>
#include <tinyengine_function.h>
#include <tinyengine_function_fp.h>

#include "genNN.h"
#include "det_model.h"
#include "genInclude.h"

/* Variables used by all ops */
static ADD_params add_params;
static int i;
static int8_t *int8ptr,*int8ptr2;
static int32_t *int32ptr;
static float *fptr,*fptr2,*fptr3;

signed char* det_getInput() {
    return &buffer0[81920];
}
signed char* det_getOutput() {
    return NNoutput;
}
void det_end2endinference(q7_t* img){
    det_invoke(NULL);
}
void det_invoke(float* labels){
/* layer 0:CONV_2D */
convolve_s8_kernel3_inputch3_stride2_pad1_fpreq(&buffer0[81920],160,128,3,(const q7_t*) weight0,bias0,scales0,-128,1,-128,127,&buffer0[0],80,64,16,sbuf,kbuf,-1);
/* layer 1:DEPTHWISE_CONV_2D */
depthwise_kernel3x3_stride1_inplace_CHW_fpreq(&buffer0[0],80,64,16,(const q7_t*) CHWweight1,offsetBias1,offsetRBias1,scales1,-128,128,-128,127,&buffer0[0],80,64,16,sbuf,-128);
/* layer 2:CONV_2D */
convolve_1x1_s8_ch16_fpreq(&buffer0[0],80,64,16,(const q7_t*) weight2,bias2,scales2,-128,128,-128,127,&buffer0[81920],80,64,16,sbuf);
/* layer 3:DEPTHWISE_CONV_2D */
depthwise_kernel3x3_stride2_inplace_CHW_fpreq(&buffer0[81920],80,64,16,(const q7_t*) CHWweight3,offsetBias3,offsetRBias3,scales3,-128,128,-128,127,&buffer0[81920],40,32,16,sbuf,-128);
/* layer 4:CONV_2D */
convolve_1x1_s8_ch16_fpreq(&buffer0[81920],40,32,16,(const q7_t*) weight4,bias4,scales4,-128,128,-128,127,&buffer0[0],40,32,40,sbuf);
/* layer 5:DEPTHWISE_CONV_2D */
depthwise_kernel3x3_stride1_inplace_CHW_fpreq(&buffer0[0],40,32,40,(const q7_t*) CHWweight5,offsetBias5,offsetRBias5,scales5,-128,128,-128,127,&buffer0[0],40,32,40,sbuf,-128);
/* layer 6:CONV_2D */
convolve_1x1_s8_fpreq(&buffer0[0],40,32,40,(const q7_t*) weight6,bias6,scales6,-128,128,-128,127,&buffer0[51200],40,32,40,sbuf);
/* layer 7:DEPTHWISE_CONV_2D */
depthwise_kernel3x3_stride2_inplace_CHW_fpreq(&buffer0[51200],40,32,40,(const q7_t*) CHWweight7,offsetBias7,offsetRBias7,scales7,-128,128,-128,127,&buffer0[51200],20,16,40,sbuf,-128);
/* layer 8:CONV_2D */
convolve_1x1_s8_fpreq(&buffer0[51200],20,16,40,(const q7_t*) weight8,bias8,scales8,-128,128,-128,127,&buffer0[0],20,16,72,sbuf);
/* layer 9:DEPTHWISE_CONV_2D */
depthwise_kernel3x3_stride1_inplace_CHW_fpreq(&buffer0[0],20,16,72,(const q7_t*) CHWweight9,offsetBias9,offsetRBias9,scales9,-128,128,-128,127,&buffer0[0],20,16,72,sbuf,-128);
/* layer 10:CONV_2D */
convolve_1x1_s8_fpreq(&buffer0[0],20,16,72,(const q7_t*) weight10,bias10,scales10,-128,128,-128,127,&buffer0[23040],20,16,72,sbuf);
/* layer 11:DEPTHWISE_CONV_2D */
depthwise_kernel3x3_stride1_inplace_CHW_fpreq(&buffer0[23040],20,16,72,(const q7_t*) CHWweight11,offsetBias11,offsetRBias11,scales11,-128,128,-128,127,&buffer0[23040],20,16,72,sbuf,-128);
/* layer 12:CONV_2D */
convolve_1x1_s8_fpreq(&buffer0[23040],20,16,72,(const q7_t*) weight12,bias12,scales12,-128,128,-128,127,&buffer0[0],20,16,72,sbuf);
/* layer 13:UPSAMPLE */
upsample_byte(&buffer0[0], 20, 16, 72, &buffer0[23040], 1);
/* layer 14:DEPTHWISE_CONV_2D */
depthwise_kernel3x3_stride2_inplace_CHW_fpreq(&buffer0[0],20,16,72,(const q7_t*) CHWweight13,offsetBias13,offsetRBias13,scales13,-128,128,-128,127,&buffer0[0],10,8,72,sbuf,-128);
/* layer 15:CONV_2D */
convolve_1x1_s8_fpreq(&buffer0[0],10,8,72,(const q7_t*) weight14,bias14,scales14,-128,128,-128,127,&buffer0[5760],10,8,152,sbuf);
/* layer 16:DEPTHWISE_CONV_2D */
depthwise_kernel3x3_stride1_inplace_CHW_fpreq(&buffer0[5760],10,8,152,(const q7_t*) CHWweight15,offsetBias15,offsetRBias15,scales15,-128,128,-128,127,&buffer0[5760],10,8,152,sbuf,-128);
/* layer 17:CONV_2D */
convolve_1x1_s8_fpreq(&buffer0[5760],10,8,152,(const q7_t*) weight16,bias16,scales16,-128,128,-128,127,&buffer0[46080],10,8,152,sbuf);
/* layer 18:UPSAMPLE */
upsample_byte(&buffer0[46080], 10, 8, 152, &buffer0[0], 1);
/* layer 19:DEPTHWISE_CONV_2D */
depthwise_kernel3x3_stride2_inplace_CHW_fpreq(&buffer0[46080],10,8,152,(const q7_t*) CHWweight17,offsetBias17,offsetRBias17,scales17,-128,128,-128,127,&buffer0[46080],5,4,152,sbuf,-128);
/* layer 20:CONV_2D */
convolve_1x1_s8_fpreq(&buffer0[46080],5,4,152,(const q7_t*) weight18,bias18,scales18,-128,128,-128,127,&buffer0[12160],5,4,288,sbuf);
/* layer 21:DEPTHWISE_CONV_2D */
depthwise_kernel3x3_stride1_inplace_CHW_fpreq(&buffer0[12160],5,4,288,(const q7_t*) CHWweight19,offsetBias19,offsetRBias19,scales19,-128,128,-128,127,&buffer0[12160],5,4,288,sbuf,-128);
/* layer 22:CONV_2D */
convolve_1x1_s8_fpreq(&buffer0[12160],5,4,288,(const q7_t*) weight20,bias20,scales20,-128,128,-128,127,&buffer0[46080],5,4,288,sbuf);
/* layer 23:DEPTHWISE_CONV_2D */
depthwise_kernel3x3_stride1_inplace_CHW_fpreq(&buffer0[46080],5,4,288,(const q7_t*) CHWweight21,offsetBias21,offsetRBias21,scales21,-128,128,-128,127,&buffer0[46080],5,4,288,sbuf,-128);
/* layer 24:CONV_2D */
convolve_1x1_s8_fpreq(&buffer0[46080],5,4,288,(const q7_t*) weight22,bias22,scales22,-128,128,-128,127,&buffer0[12160],5,4,288,sbuf);
/* layer 25:DEPTHWISE_CONV_2D */
depthwise_kernel3x3_stride1_inplace_CHW_fpreq(&buffer0[12160],5,4,288,(const q7_t*) CHWweight23,offsetBias23,offsetRBias23,scales23,-128,128,-128,127,&buffer0[12160],5,4,288,sbuf,-128);
/* layer 26:CONV_2D */
convolve_1x1_s8_fpreq(&buffer0[12160],5,4,288,(const q7_t*) weight24,bias24,scales24,-128,128,-128,127,&buffer0[46080],5,4,288,sbuf);
/* layer 27:DEPTHWISE_CONV_2D */
depthwise_kernel3x3_stride1_inplace_CHW_fpreq(&buffer0[46080],5,4,288,(const q7_t*) CHWweight25,offsetBias25,offsetRBias25,scales25,-128,128,-128,127,&buffer0[46080],5,4,288,sbuf,-128);
/* layer 28:CONV_2D */
convolve_1x1_s8_fpreq(&buffer0[46080],5,4,288,(const q7_t*) weight26,bias26,scales26,-128,128,-128,127,&buffer0[12160],5,4,288,sbuf);
/* layer 29:DEPTHWISE_CONV_2D */
depthwise_kernel3x3_stride1_inplace_CHW_fpreq(&buffer0[12160],5,4,288,(const q7_t*) CHWweight27,offsetBias27,offsetRBias27,scales27,-128,128,-128,127,&buffer0[12160],5,4,288,sbuf,-128);
/* layer 30:CONV_2D */
convolve_1x1_s8_fpreq(&buffer0[12160],5,4,288,(const q7_t*) weight28,bias28,scales28,-128,128,-128,127,&buffer0[46080],5,4,288,sbuf);
/* layer 31:CONV_2D */
convolve_1x1_s8_fpreq(&buffer0[46080],5,4,288,(const q7_t*) weight29,bias29,scales29,21,128,-128,127,&buffer0[13440],5,4,16,sbuf);
/* layer 32:UPSAMPLE */
upsample_byte(&buffer0[13440], 5, 4, 16, &buffer0[12160], 2);
/* layer 33:CONV_2D */
convolve_s8_kernel3_stride1_pad1_fpreq(&buffer0[13440],5,4,16,(const q7_t*) weight30,bias30,scales30,10,-21,-128,127,&buffer0[48000],5,4,16,sbuf,21);
/* layer 34:CONV_2D */
convolve_1x1_s8_fpreq(&buffer0[0],10,8,152,(const q7_t*) weight31,bias31,scales31,4,128,-128,127,&buffer0[13440],10,8,16,sbuf);
/* layer 35:ADD */
add_fpreq(1280, &buffer0[13440],0.001496237819083035,4,&buffer0[12160],0.0008813298773020506,21,0.001496237819083035,4,&buffer0[5120]);
/* layer 36:UPSAMPLE */
upsample_byte(&buffer0[5120], 10, 8, 16, &buffer0[0], 2);
/* layer 37:CONV_2D */
convolve_s8_kernel3_stride1_pad1_fpreq(&buffer0[5120],10,8,16,(const q7_t*) weight32,bias32,scales32,-5,-4,-128,127,&buffer0[46080],10,8,16,sbuf,4);
/* layer 38:CONV_2D */
convolve_1x1_s8_fpreq(&buffer0[23040],20,16,72,(const q7_t*) weight33,bias33,scales33,-13,128,-128,127,&buffer0[5120],20,16,16,sbuf);
/* layer 39:ADD */
add_fpreq(5120, &buffer0[5120],0.0028943049255758524,-13,&buffer0[0],0.001496237819083035,4,0.0028188396245241165,-17,&buffer0[10240]);
/* layer 40:CONV_2D */
convolve_s8_kernel3_stride1_pad1_fpreq(&buffer0[10240],20,16,16,(const q7_t*) weight34,bias34,scales34,5,17,-128,127,&buffer0[20480],20,16,16,sbuf,-17);
/* layer 41:UPSAMPLE */
upsample_byte(&buffer0[20480], 20, 16, 16, &buffer0[40960], 1);
/* layer 42:DEPTHWISE_CONV_2D */
depthwise_kernel3x3_stride1_inplace_CHW_fpreq(&buffer0[20480],20,16,16,(const q7_t*) CHWweight35,offsetBias35,offsetRBias35,scales35,-128,-5,-128,127,&buffer0[20480],20,16,16,sbuf,5);
/* layer 43:CONV_2D */
convolve_1x1_s8_ch16_fpreq(&buffer0[20480],20,16,16,(const q7_t*) weight36,bias36,scales36,-128,128,-128,127,&buffer0[0],20,16,64,sbuf);
/* layer 44:DEPTHWISE_CONV_2D */
depthwise_kernel3x3_stride1_inplace_CHW_fpreq(&buffer0[0],20,16,64,(const q7_t*) CHWweight37,offsetBias37,offsetRBias37,scales37,-128,128,-128,127,&buffer0[0],20,16,64,sbuf,-128);
/* layer 45:CONV_2D */
convolve_1x1_s8_fpreq(&buffer0[0],20,16,64,(const q7_t*) weight38,bias38,scales38,-128,128,-128,127,&buffer0[20480],20,16,64,sbuf);
/* layer 46:CONV_2D */
convolve_s8_kernel3_stride1_pad1_fpreq(&buffer0[20480],20,16,64,(const q7_t*) weight39,bias39,scales39,78,128,-128,127,&buffer0[47360],20,16,2,sbuf,-128);
/* layer 47:CONV_2D */
convolve_s8_kernel3_stride1_pad1_fpreq(&buffer0[20480],20,16,64,(const q7_t*) weight40,bias40,scales40,-128,128,-128,127,&buffer0[16640],20,16,8,sbuf,-128);
/* layer 48:CONV_2D */
convolve_s8_kernel3_stride1_pad1_fpreq(&buffer0[20480],20,16,64,(const q7_t*) weight41,bias41,scales41,1,128,-128,127,&buffer0[0],20,16,20,sbuf,-128);
/* layer 49:CONV_2D */
convolve_s8_kernel3_stride2_pad1_fpreq_any(&buffer0[40960],20,16,16,(const q7_t*) weight42,bias42,scales42,-20,-5,-128,127,&buffer0[6400],10,8,16,sbuf,kbuf,5);
/* layer 50:ADD */
add_fpreq(1280, &buffer0[46080],0.0008746108505874872,-5,&buffer0[6400],0.0007943395175971091,-20,0.0008746108505874872,-5,&buffer0[19200]);
/* layer 51:CONV_2D */
convolve_s8_kernel3_stride1_pad1_fpreq(&buffer0[19200],10,8,16,(const q7_t*) weight43,bias43,scales43,16,5,-128,127,&buffer0[11520],10,8,16,sbuf,-5);
/* layer 52:DEPTHWISE_CONV_2D */
depthwise_kernel3x3_stride1_inplace_CHW_fpreq(&buffer0[11520],10,8,16,(const q7_t*) CHWweight44,offsetBias44,offsetRBias44,scales44,-128,-16,-128,127,&buffer0[11520],10,8,16,sbuf,16);
/* layer 53:CONV_2D */
convolve_1x1_s8_ch16_fpreq(&buffer0[11520],10,8,16,(const q7_t*) weight45,bias45,scales45,-128,128,-128,127,&buffer0[6400],10,8,64,sbuf);
/* layer 54:DEPTHWISE_CONV_2D */
depthwise_kernel3x3_stride1_inplace_CHW_fpreq(&buffer0[6400],10,8,64,(const q7_t*) CHWweight46,offsetBias46,offsetRBias46,scales46,-128,128,-128,127,&buffer0[6400],10,8,64,sbuf,-128);
/* layer 55:CONV_2D */
convolve_1x1_s8_fpreq(&buffer0[6400],10,8,64,(const q7_t*) weight47,bias47,scales47,-128,128,-128,127,&buffer0[11520],10,8,64,sbuf);
/* layer 56:CONV_2D */
convolve_s8_kernel3_stride1_pad1_fpreq(&buffer0[11520],10,8,64,(const q7_t*) weight48,bias48,scales48,67,128,-128,127,&buffer0[11200],10,8,2,sbuf,-128);
/* layer 57:CONV_2D */
convolve_s8_kernel3_stride1_pad1_fpreq(&buffer0[11520],10,8,64,(const q7_t*) weight49,bias49,scales49,-128,128,-128,127,&buffer0[10560],10,8,8,sbuf,-128);
/* layer 58:CONV_2D */
convolve_s8_kernel3_stride1_pad1_fpreq(&buffer0[11520],10,8,64,(const q7_t*) weight50,bias50,scales50,-1,128,-128,127,&buffer0[6400],10,8,20,sbuf,-128);
/* layer 59:CONV_2D */
convolve_s8_kernel3_stride2_pad1_fpreq_any(&buffer0[19200],10,8,16,(const q7_t*) weight51,bias51,scales51,1,5,-128,127,&buffer0[8000],5,4,16,sbuf,kbuf,-5);
/* layer 60:ADD */
add_fpreq(320, &buffer0[48000],0.00046758673852309585,10,&buffer0[8000],0.0003003638703376055,1,0.00046758673852309585,10,&buffer0[8320]);
/* layer 61:CONV_2D */
convolve_s8_kernel3_stride1_pad1_fpreq(&buffer0[8320],5,4,16,(const q7_t*) weight52,bias52,scales52,-2,-10,-128,127,&buffer0[9280],5,4,16,sbuf,10);
/* layer 62:DEPTHWISE_CONV_2D */
depthwise_kernel3x3_stride1_inplace_CHW_fpreq(&buffer0[9280],5,4,16,(const q7_t*) CHWweight53,offsetBias53,offsetRBias53,scales53,-128,2,-128,127,&buffer0[9280],5,4,16,sbuf,-2);
/* layer 63:CONV_2D */
convolve_1x1_s8_ch16_fpreq(&buffer0[9280],5,4,16,(const q7_t*) weight54,bias54,scales54,-128,128,-128,127,&buffer0[8000],5,4,64,sbuf);
/* layer 64:DEPTHWISE_CONV_2D */
depthwise_kernel3x3_stride1_inplace_CHW_fpreq(&buffer0[8000],5,4,64,(const q7_t*) CHWweight55,offsetBias55,offsetRBias55,scales55,-128,128,-128,127,&buffer0[8000],5,4,64,sbuf,-128);
/* layer 65:CONV_2D */
convolve_1x1_s8_fpreq(&buffer0[8000],5,4,64,(const q7_t*) weight56,bias56,scales56,-128,128,-128,127,&buffer0[9280],5,4,64,sbuf);
/* layer 66:CONV_2D */
convolve_s8_kernel3_stride1_pad1_fpreq(&buffer0[9280],5,4,64,(const q7_t*) weight57,bias57,scales57,127,128,-128,127,&buffer0[8560],5,4,2,sbuf,-128);
/* layer 67:CONV_2D */
convolve_s8_kernel3_stride1_pad1_fpreq(&buffer0[9280],5,4,64,(const q7_t*) weight58,bias58,scales58,-128,128,-128,127,&buffer0[8400],5,4,8,sbuf,-128);
/* layer 68:CONV_2D */
convolve_s8_kernel3_stride1_pad1_fpreq(&buffer0[9280],5,4,64,(const q7_t*) weight59,bias59,scales59,-21,128,-128,127,&buffer0[8000],5,4,20,sbuf,-128);
}
void det_invoke_inf(){
/* layer 0:CONV_2D */
convolve_s8_kernel3_inputch3_stride2_pad1_fpreq(&buffer0[81920],160,128,3,(const q7_t*) weight0,bias0,scales0,-128,1,-128,127,&buffer0[0],80,64,16,sbuf,kbuf,-1);
/* layer 1:DEPTHWISE_CONV_2D */
depthwise_kernel3x3_stride1_inplace_CHW_fpreq(&buffer0[0],80,64,16,(const q7_t*) CHWweight1,offsetBias1,offsetRBias1,scales1,-128,128,-128,127,&buffer0[0],80,64,16,sbuf,-128);
/* layer 2:CONV_2D */
convolve_1x1_s8_ch16_fpreq(&buffer0[0],80,64,16,(const q7_t*) weight2,bias2,scales2,-128,128,-128,127,&buffer0[81920],80,64,16,sbuf);
/* layer 3:DEPTHWISE_CONV_2D */
depthwise_kernel3x3_stride2_inplace_CHW_fpreq(&buffer0[81920],80,64,16,(const q7_t*) CHWweight3,offsetBias3,offsetRBias3,scales3,-128,128,-128,127,&buffer0[81920],40,32,16,sbuf,-128);
/* layer 4:CONV_2D */
convolve_1x1_s8_ch16_fpreq(&buffer0[81920],40,32,16,(const q7_t*) weight4,bias4,scales4,-128,128,-128,127,&buffer0[0],40,32,40,sbuf);
/* layer 5:DEPTHWISE_CONV_2D */
depthwise_kernel3x3_stride1_inplace_CHW_fpreq(&buffer0[0],40,32,40,(const q7_t*) CHWweight5,offsetBias5,offsetRBias5,scales5,-128,128,-128,127,&buffer0[0],40,32,40,sbuf,-128);
/* layer 6:CONV_2D */
convolve_1x1_s8_fpreq(&buffer0[0],40,32,40,(const q7_t*) weight6,bias6,scales6,-128,128,-128,127,&buffer0[51200],40,32,40,sbuf);
/* layer 7:DEPTHWISE_CONV_2D */
depthwise_kernel3x3_stride2_inplace_CHW_fpreq(&buffer0[51200],40,32,40,(const q7_t*) CHWweight7,offsetBias7,offsetRBias7,scales7,-128,128,-128,127,&buffer0[51200],20,16,40,sbuf,-128);
/* layer 8:CONV_2D */
convolve_1x1_s8_fpreq(&buffer0[51200],20,16,40,(const q7_t*) weight8,bias8,scales8,-128,128,-128,127,&buffer0[0],20,16,72,sbuf);
/* layer 9:DEPTHWISE_CONV_2D */
depthwise_kernel3x3_stride1_inplace_CHW_fpreq(&buffer0[0],20,16,72,(const q7_t*) CHWweight9,offsetBias9,offsetRBias9,scales9,-128,128,-128,127,&buffer0[0],20,16,72,sbuf,-128);
/* layer 10:CONV_2D */
convolve_1x1_s8_fpreq(&buffer0[0],20,16,72,(const q7_t*) weight10,bias10,scales10,-128,128,-128,127,&buffer0[23040],20,16,72,sbuf);
/* layer 11:DEPTHWISE_CONV_2D */
depthwise_kernel3x3_stride1_inplace_CHW_fpreq(&buffer0[23040],20,16,72,(const q7_t*) CHWweight11,offsetBias11,offsetRBias11,scales11,-128,128,-128,127,&buffer0[23040],20,16,72,sbuf,-128);
/* layer 12:CONV_2D */
convolve_1x1_s8_fpreq(&buffer0[23040],20,16,72,(const q7_t*) weight12,bias12,scales12,-128,128,-128,127,&buffer0[0],20,16,72,sbuf);
/* layer 13:UPSAMPLE */
upsample_byte(&buffer0[0], 20, 16, 72, &buffer0[23040], 1);
/* layer 14:DEPTHWISE_CONV_2D */
depthwise_kernel3x3_stride2_inplace_CHW_fpreq(&buffer0[0],20,16,72,(const q7_t*) CHWweight13,offsetBias13,offsetRBias13,scales13,-128,128,-128,127,&buffer0[0],10,8,72,sbuf,-128);
/* layer 15:CONV_2D */
convolve_1x1_s8_fpreq(&buffer0[0],10,8,72,(const q7_t*) weight14,bias14,scales14,-128,128,-128,127,&buffer0[5760],10,8,152,sbuf);
/* layer 16:DEPTHWISE_CONV_2D */
depthwise_kernel3x3_stride1_inplace_CHW_fpreq(&buffer0[5760],10,8,152,(const q7_t*) CHWweight15,offsetBias15,offsetRBias15,scales15,-128,128,-128,127,&buffer0[5760],10,8,152,sbuf,-128);
/* layer 17:CONV_2D */
convolve_1x1_s8_fpreq(&buffer0[5760],10,8,152,(const q7_t*) weight16,bias16,scales16,-128,128,-128,127,&buffer0[46080],10,8,152,sbuf);
/* layer 18:UPSAMPLE */
upsample_byte(&buffer0[46080], 10, 8, 152, &buffer0[0], 1);
/* layer 19:DEPTHWISE_CONV_2D */
depthwise_kernel3x3_stride2_inplace_CHW_fpreq(&buffer0[46080],10,8,152,(const q7_t*) CHWweight17,offsetBias17,offsetRBias17,scales17,-128,128,-128,127,&buffer0[46080],5,4,152,sbuf,-128);
/* layer 20:CONV_2D */
convolve_1x1_s8_fpreq(&buffer0[46080],5,4,152,(const q7_t*) weight18,bias18,scales18,-128,128,-128,127,&buffer0[12160],5,4,288,sbuf);
/* layer 21:DEPTHWISE_CONV_2D */
depthwise_kernel3x3_stride1_inplace_CHW_fpreq(&buffer0[12160],5,4,288,(const q7_t*) CHWweight19,offsetBias19,offsetRBias19,scales19,-128,128,-128,127,&buffer0[12160],5,4,288,sbuf,-128);
/* layer 22:CONV_2D */
convolve_1x1_s8_fpreq(&buffer0[12160],5,4,288,(const q7_t*) weight20,bias20,scales20,-128,128,-128,127,&buffer0[46080],5,4,288,sbuf);
/* layer 23:DEPTHWISE_CONV_2D */
depthwise_kernel3x3_stride1_inplace_CHW_fpreq(&buffer0[46080],5,4,288,(const q7_t*) CHWweight21,offsetBias21,offsetRBias21,scales21,-128,128,-128,127,&buffer0[46080],5,4,288,sbuf,-128);
/* layer 24:CONV_2D */
convolve_1x1_s8_fpreq(&buffer0[46080],5,4,288,(const q7_t*) weight22,bias22,scales22,-128,128,-128,127,&buffer0[12160],5,4,288,sbuf);
/* layer 25:DEPTHWISE_CONV_2D */
depthwise_kernel3x3_stride1_inplace_CHW_fpreq(&buffer0[12160],5,4,288,(const q7_t*) CHWweight23,offsetBias23,offsetRBias23,scales23,-128,128,-128,127,&buffer0[12160],5,4,288,sbuf,-128);
/* layer 26:CONV_2D */
convolve_1x1_s8_fpreq(&buffer0[12160],5,4,288,(const q7_t*) weight24,bias24,scales24,-128,128,-128,127,&buffer0[46080],5,4,288,sbuf);
/* layer 27:DEPTHWISE_CONV_2D */
depthwise_kernel3x3_stride1_inplace_CHW_fpreq(&buffer0[46080],5,4,288,(const q7_t*) CHWweight25,offsetBias25,offsetRBias25,scales25,-128,128,-128,127,&buffer0[46080],5,4,288,sbuf,-128);
/* layer 28:CONV_2D */
convolve_1x1_s8_fpreq(&buffer0[46080],5,4,288,(const q7_t*) weight26,bias26,scales26,-128,128,-128,127,&buffer0[12160],5,4,288,sbuf);
/* layer 29:DEPTHWISE_CONV_2D */
depthwise_kernel3x3_stride1_inplace_CHW_fpreq(&buffer0[12160],5,4,288,(const q7_t*) CHWweight27,offsetBias27,offsetRBias27,scales27,-128,128,-128,127,&buffer0[12160],5,4,288,sbuf,-128);
/* layer 30:CONV_2D */
convolve_1x1_s8_fpreq(&buffer0[12160],5,4,288,(const q7_t*) weight28,bias28,scales28,-128,128,-128,127,&buffer0[46080],5,4,288,sbuf);
/* layer 31:CONV_2D */
convolve_1x1_s8_fpreq(&buffer0[46080],5,4,288,(const q7_t*) weight29,bias29,scales29,21,128,-128,127,&buffer0[13440],5,4,16,sbuf);
/* layer 32:UPSAMPLE */
upsample_byte(&buffer0[13440], 5, 4, 16, &buffer0[12160], 2);
/* layer 33:CONV_2D */
convolve_s8_kernel3_stride1_pad1_fpreq(&buffer0[13440],5,4,16,(const q7_t*) weight30,bias30,scales30,10,-21,-128,127,&buffer0[48000],5,4,16,sbuf,21);
/* layer 34:CONV_2D */
convolve_1x1_s8_fpreq(&buffer0[0],10,8,152,(const q7_t*) weight31,bias31,scales31,4,128,-128,127,&buffer0[13440],10,8,16,sbuf);
/* layer 35:ADD */
add_fpreq(1280, &buffer0[13440],0.001496237819083035,4,&buffer0[12160],0.0008813298773020506,21,0.001496237819083035,4,&buffer0[5120]);
/* layer 36:UPSAMPLE */
upsample_byte(&buffer0[5120], 10, 8, 16, &buffer0[0], 2);
/* layer 37:CONV_2D */
convolve_s8_kernel3_stride1_pad1_fpreq(&buffer0[5120],10,8,16,(const q7_t*) weight32,bias32,scales32,-5,-4,-128,127,&buffer0[46080],10,8,16,sbuf,4);
/* layer 38:CONV_2D */
convolve_1x1_s8_fpreq(&buffer0[23040],20,16,72,(const q7_t*) weight33,bias33,scales33,-13,128,-128,127,&buffer0[5120],20,16,16,sbuf);
/* layer 39:ADD */
add_fpreq(5120, &buffer0[5120],0.0028943049255758524,-13,&buffer0[0],0.001496237819083035,4,0.0028188396245241165,-17,&buffer0[10240]);
/* layer 40:CONV_2D */
convolve_s8_kernel3_stride1_pad1_fpreq(&buffer0[10240],20,16,16,(const q7_t*) weight34,bias34,scales34,5,17,-128,127,&buffer0[20480],20,16,16,sbuf,-17);
/* layer 41:UPSAMPLE */
upsample_byte(&buffer0[20480], 20, 16, 16, &buffer0[40960], 1);
/* layer 42:DEPTHWISE_CONV_2D */
depthwise_kernel3x3_stride1_inplace_CHW_fpreq(&buffer0[20480],20,16,16,(const q7_t*) CHWweight35,offsetBias35,offsetRBias35,scales35,-128,-5,-128,127,&buffer0[20480],20,16,16,sbuf,5);
/* layer 43:CONV_2D */
convolve_1x1_s8_ch16_fpreq(&buffer0[20480],20,16,16,(const q7_t*) weight36,bias36,scales36,-128,128,-128,127,&buffer0[0],20,16,64,sbuf);
/* layer 44:DEPTHWISE_CONV_2D */
depthwise_kernel3x3_stride1_inplace_CHW_fpreq(&buffer0[0],20,16,64,(const q7_t*) CHWweight37,offsetBias37,offsetRBias37,scales37,-128,128,-128,127,&buffer0[0],20,16,64,sbuf,-128);
/* layer 45:CONV_2D */
convolve_1x1_s8_fpreq(&buffer0[0],20,16,64,(const q7_t*) weight38,bias38,scales38,-128,128,-128,127,&buffer0[20480],20,16,64,sbuf);
/* layer 46:CONV_2D */
convolve_s8_kernel3_stride1_pad1_fpreq(&buffer0[20480],20,16,64,(const q7_t*) weight39,bias39,scales39,78,128,-128,127,&buffer0[47360],20,16,2,sbuf,-128);
/* layer 47:CONV_2D */
convolve_s8_kernel3_stride1_pad1_fpreq(&buffer0[20480],20,16,64,(const q7_t*) weight40,bias40,scales40,-128,128,-128,127,&buffer0[16640],20,16,8,sbuf,-128);
/* layer 48:CONV_2D */
convolve_s8_kernel3_stride1_pad1_fpreq(&buffer0[20480],20,16,64,(const q7_t*) weight41,bias41,scales41,1,128,-128,127,&buffer0[0],20,16,20,sbuf,-128);
/* layer 49:CONV_2D */
convolve_s8_kernel3_stride2_pad1_fpreq_any(&buffer0[40960],20,16,16,(const q7_t*) weight42,bias42,scales42,-20,-5,-128,127,&buffer0[6400],10,8,16,sbuf,kbuf,5);
/* layer 50:ADD */
add_fpreq(1280, &buffer0[46080],0.0008746108505874872,-5,&buffer0[6400],0.0007943395175971091,-20,0.0008746108505874872,-5,&buffer0[19200]);
/* layer 51:CONV_2D */
convolve_s8_kernel3_stride1_pad1_fpreq(&buffer0[19200],10,8,16,(const q7_t*) weight43,bias43,scales43,16,5,-128,127,&buffer0[11520],10,8,16,sbuf,-5);
/* layer 52:DEPTHWISE_CONV_2D */
depthwise_kernel3x3_stride1_inplace_CHW_fpreq(&buffer0[11520],10,8,16,(const q7_t*) CHWweight44,offsetBias44,offsetRBias44,scales44,-128,-16,-128,127,&buffer0[11520],10,8,16,sbuf,16);
/* layer 53:CONV_2D */
convolve_1x1_s8_ch16_fpreq(&buffer0[11520],10,8,16,(const q7_t*) weight45,bias45,scales45,-128,128,-128,127,&buffer0[6400],10,8,64,sbuf);
/* layer 54:DEPTHWISE_CONV_2D */
depthwise_kernel3x3_stride1_inplace_CHW_fpreq(&buffer0[6400],10,8,64,(const q7_t*) CHWweight46,offsetBias46,offsetRBias46,scales46,-128,128,-128,127,&buffer0[6400],10,8,64,sbuf,-128);
/* layer 55:CONV_2D */
convolve_1x1_s8_fpreq(&buffer0[6400],10,8,64,(const q7_t*) weight47,bias47,scales47,-128,128,-128,127,&buffer0[11520],10,8,64,sbuf);
/* layer 56:CONV_2D */
convolve_s8_kernel3_stride1_pad1_fpreq(&buffer0[11520],10,8,64,(const q7_t*) weight48,bias48,scales48,67,128,-128,127,&buffer0[11200],10,8,2,sbuf,-128);
/* layer 57:CONV_2D */
convolve_s8_kernel3_stride1_pad1_fpreq(&buffer0[11520],10,8,64,(const q7_t*) weight49,bias49,scales49,-128,128,-128,127,&buffer0[10560],10,8,8,sbuf,-128);
/* layer 58:CONV_2D */
convolve_s8_kernel3_stride1_pad1_fpreq(&buffer0[11520],10,8,64,(const q7_t*) weight50,bias50,scales50,-1,128,-128,127,&buffer0[6400],10,8,20,sbuf,-128);
/* layer 59:CONV_2D */
convolve_s8_kernel3_stride2_pad1_fpreq_any(&buffer0[19200],10,8,16,(const q7_t*) weight51,bias51,scales51,1,5,-128,127,&buffer0[8000],5,4,16,sbuf,kbuf,-5);
/* layer 60:ADD */
add_fpreq(320, &buffer0[48000],0.00046758673852309585,10,&buffer0[8000],0.0003003638703376055,1,0.00046758673852309585,10,&buffer0[8320]);
/* layer 61:CONV_2D */
convolve_s8_kernel3_stride1_pad1_fpreq(&buffer0[8320],5,4,16,(const q7_t*) weight52,bias52,scales52,-2,-10,-128,127,&buffer0[9280],5,4,16,sbuf,10);
/* layer 62:DEPTHWISE_CONV_2D */
depthwise_kernel3x3_stride1_inplace_CHW_fpreq(&buffer0[9280],5,4,16,(const q7_t*) CHWweight53,offsetBias53,offsetRBias53,scales53,-128,2,-128,127,&buffer0[9280],5,4,16,sbuf,-2);
/* layer 63:CONV_2D */
convolve_1x1_s8_ch16_fpreq(&buffer0[9280],5,4,16,(const q7_t*) weight54,bias54,scales54,-128,128,-128,127,&buffer0[8000],5,4,64,sbuf);
/* layer 64:DEPTHWISE_CONV_2D */
depthwise_kernel3x3_stride1_inplace_CHW_fpreq(&buffer0[8000],5,4,64,(const q7_t*) CHWweight55,offsetBias55,offsetRBias55,scales55,-128,128,-128,127,&buffer0[8000],5,4,64,sbuf,-128);
/* layer 65:CONV_2D */
convolve_1x1_s8_fpreq(&buffer0[8000],5,4,64,(const q7_t*) weight56,bias56,scales56,-128,128,-128,127,&buffer0[9280],5,4,64,sbuf);
/* layer 66:CONV_2D */
convolve_s8_kernel3_stride1_pad1_fpreq(&buffer0[9280],5,4,64,(const q7_t*) weight57,bias57,scales57,127,128,-128,127,&buffer0[8560],5,4,2,sbuf,-128);
/* layer 67:CONV_2D */
convolve_s8_kernel3_stride1_pad1_fpreq(&buffer0[9280],5,4,64,(const q7_t*) weight58,bias58,scales58,-128,128,-128,127,&buffer0[8400],5,4,8,sbuf,-128);
/* layer 68:CONV_2D */
convolve_s8_kernel3_stride1_pad1_fpreq(&buffer0[9280],5,4,64,(const q7_t*) weight59,bias59,scales59,-21,128,-128,127,&buffer0[8000],5,4,20,sbuf,-128);
}

signed char *det_te_model_arena(void) { return buffer0; }
