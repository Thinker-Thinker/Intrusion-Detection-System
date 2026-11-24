/* Automatically generated source file */
#include <float.h>
#include <tinyengine_function.h>
#include <tinyengine_function_fp.h>

#include "genNN.h"
#include "liv_model.h"
#include "genInclude.h"

/* Variables used by all ops */
static ADD_params add_params;
static int i;
static int8_t *int8ptr,*int8ptr2;
static int32_t *int32ptr;
static float *fptr,*fptr2,*fptr3;

signed char* liv_getInput() {
    return &buffer0[0];
}
signed char* liv_getOutput() {
    return NNoutput;
}
void liv_end2endinference(q7_t* img){
    liv_invoke(NULL);
}
void global_avgpool_s8(const int8_t*,int,int,int,int8_t*,int,float,int,float);
void liv_invoke(float* labels){
/* layer 0:CONV_2D */
convolve_s8_kernel3_stride2_tfsame_fpreq(&buffer0[0],64,64,3,(const q7_t*) weight0,bias0,scales0,-128,1,-128,127,&buffer0[32768],32,32,16,sbuf,kbuf,-1);
/* layer 1:DEPTHWISE_CONV_2D */
depthwise_kernel3x3_stride1_inplace_CHW_fpreq(&buffer0[32768],32,32,16,(const q7_t*) CHWweight1,offsetBias1,offsetRBias1,scales1,-128,128,-128,127,&buffer0[32768],32,32,16,sbuf,-128);
/* layer 2:CONV_2D */
convolve_1x1_s8_ch16_fpreq(&buffer0[32768],32,32,16,(const q7_t*) weight2,bias2,scales2,-128,128,-128,127,&buffer0[0],32,32,32,sbuf);
/* layer 3:DEPTHWISE_CONV_2D */
depthwise_kernel3x3_stride2_inplace_CHW_fpreq_tfsame(&buffer0[0],32,32,32,(const q7_t*) CHWweight3,offsetBias3,offsetRBias3,scales3,-128,128,-128,127,&buffer0[0],16,16,32,sbuf,-128);
/* layer 4:CONV_2D */
convolve_1x1_s8_fpreq(&buffer0[0],16,16,32,(const q7_t*) weight4,bias4,scales4,-128,128,-128,127,&buffer0[8192],16,16,48,sbuf);
/* layer 5:DEPTHWISE_CONV_2D */
depthwise_kernel3x3_stride1_inplace_CHW_fpreq(&buffer0[8192],16,16,48,(const q7_t*) CHWweight5,offsetBias5,offsetRBias5,scales5,-128,128,-128,127,&buffer0[8192],16,16,48,sbuf,-128);
/* layer 6:CONV_2D */
convolve_1x1_s8_ch48_fpreq(&buffer0[8192],16,16,48,(const q7_t*) weight6,bias6,scales6,-128,128,-128,127,&buffer0[20480],16,16,48,sbuf);
/* layer 7:DEPTHWISE_CONV_2D */
depthwise_kernel3x3_stride2_inplace_CHW_fpreq_tfsame(&buffer0[20480],16,16,48,(const q7_t*) CHWweight7,offsetBias7,offsetRBias7,scales7,-128,128,-128,127,&buffer0[20480],8,8,48,sbuf,-128);
/* layer 8:CONV_2D */
convolve_1x1_s8_ch48_fpreq(&buffer0[20480],8,8,48,(const q7_t*) weight8,bias8,scales8,-128,128,-128,127,&buffer0[0],8,8,64,sbuf);
/* layer 9:DEPTHWISE_CONV_2D */
depthwise_kernel3x3_stride1_inplace_CHW_fpreq(&buffer0[0],8,8,64,(const q7_t*) CHWweight9,offsetBias9,offsetRBias9,scales9,-128,128,-128,127,&buffer0[0],8,8,64,sbuf,-128);
/* layer 10:CONV_2D */
convolve_1x1_s8_fpreq(&buffer0[0],8,8,64,(const q7_t*) weight10,bias10,scales10,-128,128,-128,127,&buffer0[4096],8,8,64,sbuf);
/* layer 11:DEPTHWISE_CONV_2D */
depthwise_kernel3x3_stride2_inplace_CHW_fpreq_tfsame(&buffer0[4096],8,8,64,(const q7_t*) CHWweight11,offsetBias11,offsetRBias11,scales11,-128,128,-128,127,&buffer0[4096],4,4,64,sbuf,-128);
/* layer 12:CONV_2D */
convolve_1x1_s8_fpreq(&buffer0[4096],4,4,64,(const q7_t*) weight12,bias12,scales12,-128,128,-128,127,&buffer0[0],4,4,96,sbuf);
/* layer 13:DEPTHWISE_CONV_2D */
depthwise_kernel3x3_stride1_inplace_CHW_fpreq(&buffer0[0],4,4,96,(const q7_t*) CHWweight13,offsetBias13,offsetRBias13,scales13,-128,128,-128,127,&buffer0[0],4,4,96,sbuf,-128);
/* layer 14:CONV_2D */
convolve_1x1_s8_fpreq(&buffer0[0],4,4,96,(const q7_t*) weight14,bias14,scales14,-128,128,-128,127,&buffer0[1536],4,4,96,sbuf);
/* layer 15:CONV_2D */
global_avgpool_s8((const int8_t *)&buffer0[1536],4,4,96,(int8_t *)&buffer0[0],-128,0.0345654003f,-128,0.012086384f);
convolve_1x1_s8_fpreq(&buffer0[0],1,1,96,(const q7_t*) weight15,bias15,scales15,15,128,-128,127,&buffer0[0],1,1,2,sbuf);
}
void liv_invoke_inf(){
/* layer 0:CONV_2D */
convolve_s8_kernel3_stride2_tfsame_fpreq(&buffer0[0],64,64,3,(const q7_t*) weight0,bias0,scales0,-128,1,-128,127,&buffer0[32768],32,32,16,sbuf,kbuf,-1);
/* layer 1:DEPTHWISE_CONV_2D */
depthwise_kernel3x3_stride1_inplace_CHW_fpreq(&buffer0[32768],32,32,16,(const q7_t*) CHWweight1,offsetBias1,offsetRBias1,scales1,-128,128,-128,127,&buffer0[32768],32,32,16,sbuf,-128);
/* layer 2:CONV_2D */
convolve_1x1_s8_ch16_fpreq(&buffer0[32768],32,32,16,(const q7_t*) weight2,bias2,scales2,-128,128,-128,127,&buffer0[0],32,32,32,sbuf);
/* layer 3:DEPTHWISE_CONV_2D */
depthwise_kernel3x3_stride2_inplace_CHW_fpreq_tfsame(&buffer0[0],32,32,32,(const q7_t*) CHWweight3,offsetBias3,offsetRBias3,scales3,-128,128,-128,127,&buffer0[0],16,16,32,sbuf,-128);
/* layer 4:CONV_2D */
convolve_1x1_s8_fpreq(&buffer0[0],16,16,32,(const q7_t*) weight4,bias4,scales4,-128,128,-128,127,&buffer0[8192],16,16,48,sbuf);
/* layer 5:DEPTHWISE_CONV_2D */
depthwise_kernel3x3_stride1_inplace_CHW_fpreq(&buffer0[8192],16,16,48,(const q7_t*) CHWweight5,offsetBias5,offsetRBias5,scales5,-128,128,-128,127,&buffer0[8192],16,16,48,sbuf,-128);
/* layer 6:CONV_2D */
convolve_1x1_s8_ch48_fpreq(&buffer0[8192],16,16,48,(const q7_t*) weight6,bias6,scales6,-128,128,-128,127,&buffer0[20480],16,16,48,sbuf);
/* layer 7:DEPTHWISE_CONV_2D */
depthwise_kernel3x3_stride2_inplace_CHW_fpreq_tfsame(&buffer0[20480],16,16,48,(const q7_t*) CHWweight7,offsetBias7,offsetRBias7,scales7,-128,128,-128,127,&buffer0[20480],8,8,48,sbuf,-128);
/* layer 8:CONV_2D */
convolve_1x1_s8_ch48_fpreq(&buffer0[20480],8,8,48,(const q7_t*) weight8,bias8,scales8,-128,128,-128,127,&buffer0[0],8,8,64,sbuf);
/* layer 9:DEPTHWISE_CONV_2D */
depthwise_kernel3x3_stride1_inplace_CHW_fpreq(&buffer0[0],8,8,64,(const q7_t*) CHWweight9,offsetBias9,offsetRBias9,scales9,-128,128,-128,127,&buffer0[0],8,8,64,sbuf,-128);
/* layer 10:CONV_2D */
convolve_1x1_s8_fpreq(&buffer0[0],8,8,64,(const q7_t*) weight10,bias10,scales10,-128,128,-128,127,&buffer0[4096],8,8,64,sbuf);
/* layer 11:DEPTHWISE_CONV_2D */
depthwise_kernel3x3_stride2_inplace_CHW_fpreq_tfsame(&buffer0[4096],8,8,64,(const q7_t*) CHWweight11,offsetBias11,offsetRBias11,scales11,-128,128,-128,127,&buffer0[4096],4,4,64,sbuf,-128);
/* layer 12:CONV_2D */
convolve_1x1_s8_fpreq(&buffer0[4096],4,4,64,(const q7_t*) weight12,bias12,scales12,-128,128,-128,127,&buffer0[0],4,4,96,sbuf);
/* layer 13:DEPTHWISE_CONV_2D */
depthwise_kernel3x3_stride1_inplace_CHW_fpreq(&buffer0[0],4,4,96,(const q7_t*) CHWweight13,offsetBias13,offsetRBias13,scales13,-128,128,-128,127,&buffer0[0],4,4,96,sbuf,-128);
/* layer 14:CONV_2D */
convolve_1x1_s8_fpreq(&buffer0[0],4,4,96,(const q7_t*) weight14,bias14,scales14,-128,128,-128,127,&buffer0[1536],4,4,96,sbuf);
/* layer 15:CONV_2D */
global_avgpool_s8((const int8_t *)&buffer0[1536],4,4,96,(int8_t *)&buffer0[0],-128,0.0345654003f,-128,0.012086384f);
convolve_1x1_s8_fpreq(&buffer0[0],1,1,96,(const q7_t*) weight15,bias15,scales15,15,128,-128,127,&buffer0[0],1,1,2,sbuf);
}

signed char *liv_te_model_arena(void) { return buffer0; }
