/* Automatically generated source file */
#include <float.h>
#include <tinyengine_function.h>
#include <tinyengine_function_fp.h>

#include "genNN.h"
#include "rec_model.h"
#include "genInclude.h"

/* Variables used by all ops */
static ADD_params add_params;
static int i;
static int8_t *int8ptr,*int8ptr2;
static int32_t *int32ptr;
static float *fptr,*fptr2,*fptr3;

signed char* rec_getInput() {
    return &buffer0[0];
}
signed char* rec_getOutput() {
    return NNoutput;
}
void rec_end2endinference(q7_t* img){
    rec_invoke(NULL);
}
void global_avgpool_s8(const int8_t*,int,int,int,int8_t*,int,float,int,float);
void rec_invoke(float* labels){
/* layer 0:CONV_2D */
convolve_s8_kernel3_stride2_tfsame_fpreq(&buffer0[0],112,112,3,(const q7_t*) weight0,bias0,scales0,-128,1,-128,127,&buffer0[200704],56,56,32,sbuf,kbuf,-1);
/* layer 1:DEPTHWISE_CONV_2D */
depthwise_kernel3x3_stride1_inplace_CHW_fpreq(&buffer0[200704],56,56,32,(const q7_t*) CHWweight1,offsetBias1,offsetRBias1,scales1,-128,128,-128,127,&buffer0[200704],56,56,32,sbuf,-128);
/* layer 2:CONV_2D */
convolve_1x1_s8_fpreq(&buffer0[200704],56,56,32,(const q7_t*) weight2,bias2,scales2,-128,128,-128,127,&buffer0[0],56,56,64,sbuf);
/* layer 3:DEPTHWISE_CONV_2D */
depthwise_kernel3x3_stride2_inplace_CHW_fpreq_tfsame(&buffer0[0],56,56,64,(const q7_t*) CHWweight3,offsetBias3,offsetRBias3,scales3,-128,128,-128,127,&buffer0[0],28,28,64,sbuf,-128);
/* layer 4:CONV_2D */
convolve_1x1_s8_fpreq(&buffer0[0],28,28,64,(const q7_t*) weight4,bias4,scales4,-14,128,-128,127,&buffer0[50176],28,28,32,sbuf);
/* layer 5:CONV_2D */
convolve_1x1_s8_fpreq(&buffer0[50176],28,28,32,(const q7_t*) weight5,bias5,scales5,-128,14,-128,127,&buffer0[0],28,28,64,sbuf);
/* layer 6:DEPTHWISE_CONV_2D */
depthwise_kernel3x3_stride1_inplace_CHW_fpreq(&buffer0[0],28,28,64,(const q7_t*) CHWweight6,offsetBias6,offsetRBias6,scales6,-128,128,-128,127,&buffer0[0],28,28,64,sbuf,-128);
/* layer 7:CONV_2D */
convolve_1x1_s8_fpreq(&buffer0[0],28,28,64,(const q7_t*) weight7,bias7,scales7,-6,128,-128,127,&buffer0[75264],28,28,32,sbuf);
/* layer 8:ADD */
add_fpreq(25088, &buffer0[50176],0.07585080713033676,-14,&buffer0[75264],0.09772136807441711,-6,0.09772136807441711,-6,&buffer0[100352]);
/* layer 9:CONV_2D */
convolve_1x1_s8_fpreq(&buffer0[100352],28,28,32,(const q7_t*) weight8,bias8,scales8,-128,6,-128,127,&buffer0[0],28,28,64,sbuf);
/* layer 10:DEPTHWISE_CONV_2D */
depthwise_kernel3x3_stride1_inplace_CHW_fpreq(&buffer0[0],28,28,64,(const q7_t*) CHWweight9,offsetBias9,offsetRBias9,scales9,-128,128,-128,127,&buffer0[0],28,28,64,sbuf,-128);
/* layer 11:CONV_2D */
convolve_1x1_s8_fpreq(&buffer0[0],28,28,64,(const q7_t*) weight10,bias10,scales10,4,128,-128,127,&buffer0[50176],28,28,32,sbuf);
/* layer 12:ADD */
add_fpreq(25088, &buffer0[100352],0.09772136807441711,-6,&buffer0[50176],0.11594011634588242,4,0.11594011634588242,4,&buffer0[75264]);
/* layer 13:CONV_2D */
convolve_1x1_s8_fpreq(&buffer0[75264],28,28,32,(const q7_t*) weight11,bias11,scales11,-128,-4,-128,127,&buffer0[0],28,28,64,sbuf);
/* layer 14:DEPTHWISE_CONV_2D */
depthwise_kernel3x3_stride1_inplace_CHW_fpreq(&buffer0[0],28,28,64,(const q7_t*) CHWweight12,offsetBias12,offsetRBias12,scales12,-128,128,-128,127,&buffer0[0],28,28,64,sbuf,-128);
/* layer 15:CONV_2D */
convolve_1x1_s8_fpreq(&buffer0[0],28,28,64,(const q7_t*) weight13,bias13,scales13,-7,128,-128,127,&buffer0[50176],28,28,32,sbuf);
/* layer 16:ADD */
add_fpreq(25088, &buffer0[75264],0.11594011634588242,4,&buffer0[50176],0.1379382312297821,-7,0.1379382312297821,-7,&buffer0[100352]);
/* layer 17:CONV_2D */
convolve_1x1_s8_fpreq(&buffer0[100352],28,28,32,(const q7_t*) weight14,bias14,scales14,-128,7,-128,127,&buffer0[0],28,28,64,sbuf);
/* layer 18:DEPTHWISE_CONV_2D */
depthwise_kernel3x3_stride1_inplace_CHW_fpreq(&buffer0[0],28,28,64,(const q7_t*) CHWweight15,offsetBias15,offsetRBias15,scales15,-128,128,-128,127,&buffer0[0],28,28,64,sbuf,-128);
/* layer 19:CONV_2D */
convolve_1x1_s8_fpreq(&buffer0[0],28,28,64,(const q7_t*) weight16,bias16,scales16,-13,128,-128,127,&buffer0[50176],28,28,32,sbuf);
/* layer 20:ADD */
add_fpreq(25088, &buffer0[100352],0.1379382312297821,-7,&buffer0[50176],0.1658189594745636,-13,0.1658189594745636,-13,&buffer0[125440]);
/* layer 21:CONV_2D */
convolve_1x1_s8_fpreq(&buffer0[125440],28,28,32,(const q7_t*) weight17,bias17,scales17,-128,13,-128,127,&buffer0[0],28,28,128,sbuf);
/* layer 22:DEPTHWISE_CONV_2D */
depthwise_kernel3x3_stride2_inplace_CHW_fpreq_tfsame(&buffer0[0],28,28,128,(const q7_t*) CHWweight18,offsetBias18,offsetRBias18,scales18,-128,128,-128,127,&buffer0[0],14,14,128,sbuf,-128);
/* layer 23:CONV_2D */
convolve_1x1_s8_fpreq(&buffer0[0],14,14,128,(const q7_t*) weight19,bias19,scales19,0,128,-128,127,&buffer0[25088],14,14,64,sbuf);
/* layer 24:CONV_2D */
convolve_1x1_s8_fpreq(&buffer0[25088],14,14,64,(const q7_t*) weight20,bias20,scales20,-128,0,-128,127,&buffer0[0],14,14,128,sbuf);
/* layer 25:DEPTHWISE_CONV_2D */
depthwise_kernel3x3_stride1_inplace_CHW_fpreq(&buffer0[0],14,14,128,(const q7_t*) CHWweight21,offsetBias21,offsetRBias21,scales21,-128,128,-128,127,&buffer0[0],14,14,128,sbuf,-128);
/* layer 26:CONV_2D */
convolve_1x1_s8_fpreq(&buffer0[0],14,14,128,(const q7_t*) weight22,bias22,scales22,8,128,-128,127,&buffer0[37632],14,14,64,sbuf);
/* layer 27:ADD */
add_fpreq(12544, &buffer0[25088],0.039060547947883606,0,&buffer0[37632],0.05043719336390495,8,0.05043719336390495,8,&buffer0[50176]);
/* layer 28:CONV_2D */
convolve_1x1_s8_fpreq(&buffer0[50176],14,14,64,(const q7_t*) weight23,bias23,scales23,-128,-8,-128,127,&buffer0[0],14,14,128,sbuf);
/* layer 29:DEPTHWISE_CONV_2D */
depthwise_kernel3x3_stride1_inplace_CHW_fpreq(&buffer0[0],14,14,128,(const q7_t*) CHWweight24,offsetBias24,offsetRBias24,scales24,-128,128,-128,127,&buffer0[0],14,14,128,sbuf,-128);
/* layer 30:CONV_2D */
convolve_1x1_s8_fpreq(&buffer0[0],14,14,128,(const q7_t*) weight25,bias25,scales25,-12,128,-128,127,&buffer0[25088],14,14,64,sbuf);
/* layer 31:ADD */
add_fpreq(12544, &buffer0[50176],0.05043719336390495,8,&buffer0[25088],0.06947130709886551,-12,0.06947130709886551,-12,&buffer0[37632]);
/* layer 32:CONV_2D */
convolve_1x1_s8_fpreq(&buffer0[37632],14,14,64,(const q7_t*) weight26,bias26,scales26,-128,12,-128,127,&buffer0[0],14,14,128,sbuf);
/* layer 33:DEPTHWISE_CONV_2D */
depthwise_kernel3x3_stride1_inplace_CHW_fpreq(&buffer0[0],14,14,128,(const q7_t*) CHWweight27,offsetBias27,offsetRBias27,scales27,-128,128,-128,127,&buffer0[0],14,14,128,sbuf,-128);
/* layer 34:CONV_2D */
convolve_1x1_s8_fpreq(&buffer0[0],14,14,128,(const q7_t*) weight28,bias28,scales28,2,128,-128,127,&buffer0[25088],14,14,64,sbuf);
/* layer 35:ADD */
add_fpreq(12544, &buffer0[37632],0.06947130709886551,-12,&buffer0[25088],0.08522669970989227,2,0.08522669970989227,2,&buffer0[50176]);
/* layer 36:CONV_2D */
convolve_1x1_s8_fpreq(&buffer0[50176],14,14,64,(const q7_t*) weight29,bias29,scales29,-128,-2,-128,127,&buffer0[0],14,14,128,sbuf);
/* layer 37:DEPTHWISE_CONV_2D */
depthwise_kernel3x3_stride1_inplace_CHW_fpreq(&buffer0[0],14,14,128,(const q7_t*) CHWweight30,offsetBias30,offsetRBias30,scales30,-128,128,-128,127,&buffer0[0],14,14,128,sbuf,-128);
/* layer 38:CONV_2D */
convolve_1x1_s8_fpreq(&buffer0[0],14,14,128,(const q7_t*) weight31,bias31,scales31,8,128,-128,127,&buffer0[25088],14,14,64,sbuf);
/* layer 39:ADD */
add_fpreq(12544, &buffer0[50176],0.08522669970989227,2,&buffer0[25088],0.09593308717012405,8,0.09593308717012405,8,&buffer0[37632]);
/* layer 40:CONV_2D */
convolve_1x1_s8_fpreq(&buffer0[37632],14,14,64,(const q7_t*) weight32,bias32,scales32,-128,-8,-128,127,&buffer0[0],14,14,128,sbuf);
/* layer 41:DEPTHWISE_CONV_2D */
depthwise_kernel3x3_stride1_inplace_CHW_fpreq(&buffer0[0],14,14,128,(const q7_t*) CHWweight33,offsetBias33,offsetRBias33,scales33,-128,128,-128,127,&buffer0[0],14,14,128,sbuf,-128);
/* layer 42:CONV_2D */
convolve_1x1_s8_fpreq(&buffer0[0],14,14,128,(const q7_t*) weight34,bias34,scales34,-5,128,-128,127,&buffer0[25088],14,14,64,sbuf);
/* layer 43:ADD */
add_fpreq(12544, &buffer0[37632],0.09593308717012405,8,&buffer0[25088],0.12886345386505127,-5,0.12886345386505127,-5,&buffer0[50176]);
/* layer 44:CONV_2D */
convolve_1x1_s8_fpreq(&buffer0[50176],14,14,64,(const q7_t*) weight35,bias35,scales35,-128,5,-128,127,&buffer0[0],14,14,128,sbuf);
/* layer 45:DEPTHWISE_CONV_2D */
depthwise_kernel3x3_stride1_inplace_CHW_fpreq(&buffer0[0],14,14,128,(const q7_t*) CHWweight36,offsetBias36,offsetRBias36,scales36,-128,128,-128,127,&buffer0[0],14,14,128,sbuf,-128);
/* layer 46:CONV_2D */
convolve_1x1_s8_fpreq(&buffer0[0],14,14,128,(const q7_t*) weight37,bias37,scales37,-3,128,-128,127,&buffer0[25088],14,14,64,sbuf);
/* layer 47:ADD */
add_fpreq(12544, &buffer0[50176],0.12886345386505127,-5,&buffer0[25088],0.17116279900074005,-3,0.17116279900074005,-3,&buffer0[62720]);
/* layer 48:CONV_2D */
convolve_1x1_s8_fpreq(&buffer0[62720],14,14,64,(const q7_t*) weight38,bias38,scales38,-128,3,-128,127,&buffer0[0],14,14,256,sbuf);
/* layer 49:DEPTHWISE_CONV_2D */
depthwise_kernel3x3_stride2_inplace_CHW_fpreq_tfsame(&buffer0[0],14,14,256,(const q7_t*) CHWweight39,offsetBias39,offsetRBias39,scales39,-128,128,-128,127,&buffer0[0],7,7,256,sbuf,-128);
/* layer 50:CONV_2D */
convolve_1x1_s8_fpreq(&buffer0[0],7,7,256,(const q7_t*) weight40,bias40,scales40,-8,128,-128,127,&buffer0[12544],7,7,64,sbuf);
/* layer 51:CONV_2D */
convolve_1x1_s8_fpreq(&buffer0[12544],7,7,64,(const q7_t*) weight41,bias41,scales41,-128,8,-128,127,&buffer0[0],7,7,128,sbuf);
/* layer 52:DEPTHWISE_CONV_2D */
depthwise_kernel3x3_stride1_inplace_CHW_fpreq(&buffer0[0],7,7,128,(const q7_t*) CHWweight42,offsetBias42,offsetRBias42,scales42,-128,128,-128,127,&buffer0[0],7,7,128,sbuf,-128);
/* layer 53:CONV_2D */
convolve_1x1_s8_fpreq(&buffer0[0],7,7,128,(const q7_t*) weight43,bias43,scales43,-6,128,-128,127,&buffer0[6272],7,7,64,sbuf);
/* layer 54:ADD */
add_fpreq(3136, &buffer0[12544],0.035589396953582764,-8,&buffer0[6272],0.06698376685380936,-6,0.06698376685380936,-6,&buffer0[9408]);
/* layer 55:CONV_2D */
convolve_1x1_s8_fpreq(&buffer0[9408],7,7,64,(const q7_t*) weight44,bias44,scales44,-128,6,-128,127,&buffer0[0],7,7,128,sbuf);
/* layer 56:DEPTHWISE_CONV_2D */
depthwise_kernel3x3_stride1_inplace_CHW_fpreq(&buffer0[0],7,7,128,(const q7_t*) CHWweight45,offsetBias45,offsetRBias45,scales45,-128,128,-128,127,&buffer0[0],7,7,128,sbuf,-128);
/* layer 57:CONV_2D */
convolve_1x1_s8_fpreq(&buffer0[0],7,7,128,(const q7_t*) weight46,bias46,scales46,1,128,-128,127,&buffer0[6272],7,7,64,sbuf);
/* layer 58:ADD */
add_fpreq(3136, &buffer0[9408],0.06698376685380936,-6,&buffer0[6272],0.08557287603616714,1,0.08557287603616714,1,&buffer0[12544]);
/* layer 59:CONV_2D */
convolve_1x1_s8_fpreq(&buffer0[12544],7,7,64,(const q7_t*) weight47,bias47,scales47,-128,-1,-128,127,&buffer0[0],7,7,256,sbuf);
/* layer 60:CONV_2D */
global_avgpool_s8((const int8_t *)&buffer0[0],7,7,256,(int8_t *)&buffer0[12544],-128,0.0215111636f,-128,0.00239735702f);
convolve_1x1_s8_fpreq(&buffer0[12544],1,1,256,(const q7_t*) weight48,bias48,scales48,-15,128,-128,127,&buffer0[12544],1,1,512,sbuf);
}
void rec_invoke_inf(){
/* layer 0:CONV_2D */
convolve_s8_kernel3_stride2_tfsame_fpreq(&buffer0[0],112,112,3,(const q7_t*) weight0,bias0,scales0,-128,1,-128,127,&buffer0[200704],56,56,32,sbuf,kbuf,-1);
/* layer 1:DEPTHWISE_CONV_2D */
depthwise_kernel3x3_stride1_inplace_CHW_fpreq(&buffer0[200704],56,56,32,(const q7_t*) CHWweight1,offsetBias1,offsetRBias1,scales1,-128,128,-128,127,&buffer0[200704],56,56,32,sbuf,-128);
/* layer 2:CONV_2D */
convolve_1x1_s8_fpreq(&buffer0[200704],56,56,32,(const q7_t*) weight2,bias2,scales2,-128,128,-128,127,&buffer0[0],56,56,64,sbuf);
/* layer 3:DEPTHWISE_CONV_2D */
depthwise_kernel3x3_stride2_inplace_CHW_fpreq_tfsame(&buffer0[0],56,56,64,(const q7_t*) CHWweight3,offsetBias3,offsetRBias3,scales3,-128,128,-128,127,&buffer0[0],28,28,64,sbuf,-128);
/* layer 4:CONV_2D */
convolve_1x1_s8_fpreq(&buffer0[0],28,28,64,(const q7_t*) weight4,bias4,scales4,-14,128,-128,127,&buffer0[50176],28,28,32,sbuf);
/* layer 5:CONV_2D */
convolve_1x1_s8_fpreq(&buffer0[50176],28,28,32,(const q7_t*) weight5,bias5,scales5,-128,14,-128,127,&buffer0[0],28,28,64,sbuf);
/* layer 6:DEPTHWISE_CONV_2D */
depthwise_kernel3x3_stride1_inplace_CHW_fpreq(&buffer0[0],28,28,64,(const q7_t*) CHWweight6,offsetBias6,offsetRBias6,scales6,-128,128,-128,127,&buffer0[0],28,28,64,sbuf,-128);
/* layer 7:CONV_2D */
convolve_1x1_s8_fpreq(&buffer0[0],28,28,64,(const q7_t*) weight7,bias7,scales7,-6,128,-128,127,&buffer0[75264],28,28,32,sbuf);
/* layer 8:ADD */
add_fpreq(25088, &buffer0[50176],0.07585080713033676,-14,&buffer0[75264],0.09772136807441711,-6,0.09772136807441711,-6,&buffer0[100352]);
/* layer 9:CONV_2D */
convolve_1x1_s8_fpreq(&buffer0[100352],28,28,32,(const q7_t*) weight8,bias8,scales8,-128,6,-128,127,&buffer0[0],28,28,64,sbuf);
/* layer 10:DEPTHWISE_CONV_2D */
depthwise_kernel3x3_stride1_inplace_CHW_fpreq(&buffer0[0],28,28,64,(const q7_t*) CHWweight9,offsetBias9,offsetRBias9,scales9,-128,128,-128,127,&buffer0[0],28,28,64,sbuf,-128);
/* layer 11:CONV_2D */
convolve_1x1_s8_fpreq(&buffer0[0],28,28,64,(const q7_t*) weight10,bias10,scales10,4,128,-128,127,&buffer0[50176],28,28,32,sbuf);
/* layer 12:ADD */
add_fpreq(25088, &buffer0[100352],0.09772136807441711,-6,&buffer0[50176],0.11594011634588242,4,0.11594011634588242,4,&buffer0[75264]);
/* layer 13:CONV_2D */
convolve_1x1_s8_fpreq(&buffer0[75264],28,28,32,(const q7_t*) weight11,bias11,scales11,-128,-4,-128,127,&buffer0[0],28,28,64,sbuf);
/* layer 14:DEPTHWISE_CONV_2D */
depthwise_kernel3x3_stride1_inplace_CHW_fpreq(&buffer0[0],28,28,64,(const q7_t*) CHWweight12,offsetBias12,offsetRBias12,scales12,-128,128,-128,127,&buffer0[0],28,28,64,sbuf,-128);
/* layer 15:CONV_2D */
convolve_1x1_s8_fpreq(&buffer0[0],28,28,64,(const q7_t*) weight13,bias13,scales13,-7,128,-128,127,&buffer0[50176],28,28,32,sbuf);
/* layer 16:ADD */
add_fpreq(25088, &buffer0[75264],0.11594011634588242,4,&buffer0[50176],0.1379382312297821,-7,0.1379382312297821,-7,&buffer0[100352]);
/* layer 17:CONV_2D */
convolve_1x1_s8_fpreq(&buffer0[100352],28,28,32,(const q7_t*) weight14,bias14,scales14,-128,7,-128,127,&buffer0[0],28,28,64,sbuf);
/* layer 18:DEPTHWISE_CONV_2D */
depthwise_kernel3x3_stride1_inplace_CHW_fpreq(&buffer0[0],28,28,64,(const q7_t*) CHWweight15,offsetBias15,offsetRBias15,scales15,-128,128,-128,127,&buffer0[0],28,28,64,sbuf,-128);
/* layer 19:CONV_2D */
convolve_1x1_s8_fpreq(&buffer0[0],28,28,64,(const q7_t*) weight16,bias16,scales16,-13,128,-128,127,&buffer0[50176],28,28,32,sbuf);
/* layer 20:ADD */
add_fpreq(25088, &buffer0[100352],0.1379382312297821,-7,&buffer0[50176],0.1658189594745636,-13,0.1658189594745636,-13,&buffer0[125440]);
/* layer 21:CONV_2D */
convolve_1x1_s8_fpreq(&buffer0[125440],28,28,32,(const q7_t*) weight17,bias17,scales17,-128,13,-128,127,&buffer0[0],28,28,128,sbuf);
/* layer 22:DEPTHWISE_CONV_2D */
depthwise_kernel3x3_stride2_inplace_CHW_fpreq_tfsame(&buffer0[0],28,28,128,(const q7_t*) CHWweight18,offsetBias18,offsetRBias18,scales18,-128,128,-128,127,&buffer0[0],14,14,128,sbuf,-128);
/* layer 23:CONV_2D */
convolve_1x1_s8_fpreq(&buffer0[0],14,14,128,(const q7_t*) weight19,bias19,scales19,0,128,-128,127,&buffer0[25088],14,14,64,sbuf);
/* layer 24:CONV_2D */
convolve_1x1_s8_fpreq(&buffer0[25088],14,14,64,(const q7_t*) weight20,bias20,scales20,-128,0,-128,127,&buffer0[0],14,14,128,sbuf);
/* layer 25:DEPTHWISE_CONV_2D */
depthwise_kernel3x3_stride1_inplace_CHW_fpreq(&buffer0[0],14,14,128,(const q7_t*) CHWweight21,offsetBias21,offsetRBias21,scales21,-128,128,-128,127,&buffer0[0],14,14,128,sbuf,-128);
/* layer 26:CONV_2D */
convolve_1x1_s8_fpreq(&buffer0[0],14,14,128,(const q7_t*) weight22,bias22,scales22,8,128,-128,127,&buffer0[37632],14,14,64,sbuf);
/* layer 27:ADD */
add_fpreq(12544, &buffer0[25088],0.039060547947883606,0,&buffer0[37632],0.05043719336390495,8,0.05043719336390495,8,&buffer0[50176]);
/* layer 28:CONV_2D */
convolve_1x1_s8_fpreq(&buffer0[50176],14,14,64,(const q7_t*) weight23,bias23,scales23,-128,-8,-128,127,&buffer0[0],14,14,128,sbuf);
/* layer 29:DEPTHWISE_CONV_2D */
depthwise_kernel3x3_stride1_inplace_CHW_fpreq(&buffer0[0],14,14,128,(const q7_t*) CHWweight24,offsetBias24,offsetRBias24,scales24,-128,128,-128,127,&buffer0[0],14,14,128,sbuf,-128);
/* layer 30:CONV_2D */
convolve_1x1_s8_fpreq(&buffer0[0],14,14,128,(const q7_t*) weight25,bias25,scales25,-12,128,-128,127,&buffer0[25088],14,14,64,sbuf);
/* layer 31:ADD */
add_fpreq(12544, &buffer0[50176],0.05043719336390495,8,&buffer0[25088],0.06947130709886551,-12,0.06947130709886551,-12,&buffer0[37632]);
/* layer 32:CONV_2D */
convolve_1x1_s8_fpreq(&buffer0[37632],14,14,64,(const q7_t*) weight26,bias26,scales26,-128,12,-128,127,&buffer0[0],14,14,128,sbuf);
/* layer 33:DEPTHWISE_CONV_2D */
depthwise_kernel3x3_stride1_inplace_CHW_fpreq(&buffer0[0],14,14,128,(const q7_t*) CHWweight27,offsetBias27,offsetRBias27,scales27,-128,128,-128,127,&buffer0[0],14,14,128,sbuf,-128);
/* layer 34:CONV_2D */
convolve_1x1_s8_fpreq(&buffer0[0],14,14,128,(const q7_t*) weight28,bias28,scales28,2,128,-128,127,&buffer0[25088],14,14,64,sbuf);
/* layer 35:ADD */
add_fpreq(12544, &buffer0[37632],0.06947130709886551,-12,&buffer0[25088],0.08522669970989227,2,0.08522669970989227,2,&buffer0[50176]);
/* layer 36:CONV_2D */
convolve_1x1_s8_fpreq(&buffer0[50176],14,14,64,(const q7_t*) weight29,bias29,scales29,-128,-2,-128,127,&buffer0[0],14,14,128,sbuf);
/* layer 37:DEPTHWISE_CONV_2D */
depthwise_kernel3x3_stride1_inplace_CHW_fpreq(&buffer0[0],14,14,128,(const q7_t*) CHWweight30,offsetBias30,offsetRBias30,scales30,-128,128,-128,127,&buffer0[0],14,14,128,sbuf,-128);
/* layer 38:CONV_2D */
convolve_1x1_s8_fpreq(&buffer0[0],14,14,128,(const q7_t*) weight31,bias31,scales31,8,128,-128,127,&buffer0[25088],14,14,64,sbuf);
/* layer 39:ADD */
add_fpreq(12544, &buffer0[50176],0.08522669970989227,2,&buffer0[25088],0.09593308717012405,8,0.09593308717012405,8,&buffer0[37632]);
/* layer 40:CONV_2D */
convolve_1x1_s8_fpreq(&buffer0[37632],14,14,64,(const q7_t*) weight32,bias32,scales32,-128,-8,-128,127,&buffer0[0],14,14,128,sbuf);
/* layer 41:DEPTHWISE_CONV_2D */
depthwise_kernel3x3_stride1_inplace_CHW_fpreq(&buffer0[0],14,14,128,(const q7_t*) CHWweight33,offsetBias33,offsetRBias33,scales33,-128,128,-128,127,&buffer0[0],14,14,128,sbuf,-128);
/* layer 42:CONV_2D */
convolve_1x1_s8_fpreq(&buffer0[0],14,14,128,(const q7_t*) weight34,bias34,scales34,-5,128,-128,127,&buffer0[25088],14,14,64,sbuf);
/* layer 43:ADD */
add_fpreq(12544, &buffer0[37632],0.09593308717012405,8,&buffer0[25088],0.12886345386505127,-5,0.12886345386505127,-5,&buffer0[50176]);
/* layer 44:CONV_2D */
convolve_1x1_s8_fpreq(&buffer0[50176],14,14,64,(const q7_t*) weight35,bias35,scales35,-128,5,-128,127,&buffer0[0],14,14,128,sbuf);
/* layer 45:DEPTHWISE_CONV_2D */
depthwise_kernel3x3_stride1_inplace_CHW_fpreq(&buffer0[0],14,14,128,(const q7_t*) CHWweight36,offsetBias36,offsetRBias36,scales36,-128,128,-128,127,&buffer0[0],14,14,128,sbuf,-128);
/* layer 46:CONV_2D */
convolve_1x1_s8_fpreq(&buffer0[0],14,14,128,(const q7_t*) weight37,bias37,scales37,-3,128,-128,127,&buffer0[25088],14,14,64,sbuf);
/* layer 47:ADD */
add_fpreq(12544, &buffer0[50176],0.12886345386505127,-5,&buffer0[25088],0.17116279900074005,-3,0.17116279900074005,-3,&buffer0[62720]);
/* layer 48:CONV_2D */
convolve_1x1_s8_fpreq(&buffer0[62720],14,14,64,(const q7_t*) weight38,bias38,scales38,-128,3,-128,127,&buffer0[0],14,14,256,sbuf);
/* layer 49:DEPTHWISE_CONV_2D */
depthwise_kernel3x3_stride2_inplace_CHW_fpreq_tfsame(&buffer0[0],14,14,256,(const q7_t*) CHWweight39,offsetBias39,offsetRBias39,scales39,-128,128,-128,127,&buffer0[0],7,7,256,sbuf,-128);
/* layer 50:CONV_2D */
convolve_1x1_s8_fpreq(&buffer0[0],7,7,256,(const q7_t*) weight40,bias40,scales40,-8,128,-128,127,&buffer0[12544],7,7,64,sbuf);
/* layer 51:CONV_2D */
convolve_1x1_s8_fpreq(&buffer0[12544],7,7,64,(const q7_t*) weight41,bias41,scales41,-128,8,-128,127,&buffer0[0],7,7,128,sbuf);
/* layer 52:DEPTHWISE_CONV_2D */
depthwise_kernel3x3_stride1_inplace_CHW_fpreq(&buffer0[0],7,7,128,(const q7_t*) CHWweight42,offsetBias42,offsetRBias42,scales42,-128,128,-128,127,&buffer0[0],7,7,128,sbuf,-128);
/* layer 53:CONV_2D */
convolve_1x1_s8_fpreq(&buffer0[0],7,7,128,(const q7_t*) weight43,bias43,scales43,-6,128,-128,127,&buffer0[6272],7,7,64,sbuf);
/* layer 54:ADD */
add_fpreq(3136, &buffer0[12544],0.035589396953582764,-8,&buffer0[6272],0.06698376685380936,-6,0.06698376685380936,-6,&buffer0[9408]);
/* layer 55:CONV_2D */
convolve_1x1_s8_fpreq(&buffer0[9408],7,7,64,(const q7_t*) weight44,bias44,scales44,-128,6,-128,127,&buffer0[0],7,7,128,sbuf);
/* layer 56:DEPTHWISE_CONV_2D */
depthwise_kernel3x3_stride1_inplace_CHW_fpreq(&buffer0[0],7,7,128,(const q7_t*) CHWweight45,offsetBias45,offsetRBias45,scales45,-128,128,-128,127,&buffer0[0],7,7,128,sbuf,-128);
/* layer 57:CONV_2D */
convolve_1x1_s8_fpreq(&buffer0[0],7,7,128,(const q7_t*) weight46,bias46,scales46,1,128,-128,127,&buffer0[6272],7,7,64,sbuf);
/* layer 58:ADD */
add_fpreq(3136, &buffer0[9408],0.06698376685380936,-6,&buffer0[6272],0.08557287603616714,1,0.08557287603616714,1,&buffer0[12544]);
/* layer 59:CONV_2D */
convolve_1x1_s8_fpreq(&buffer0[12544],7,7,64,(const q7_t*) weight47,bias47,scales47,-128,-1,-128,127,&buffer0[0],7,7,256,sbuf);
/* layer 60:CONV_2D */
global_avgpool_s8((const int8_t *)&buffer0[0],7,7,256,(int8_t *)&buffer0[12544],-128,0.0215111636f,-128,0.00239735702f);
convolve_1x1_s8_fpreq(&buffer0[12544],1,1,256,(const q7_t*) weight48,bias48,scales48,-15,128,-128,127,&buffer0[12544],1,1,512,sbuf);
}

signed char *rec_te_model_arena(void) { return buffer0; }
