#version 410 core

#define BLUR_SIZE 15
#define width 800
#define heigth 600

out vec4 FragColor;
in vec2 texCoords;


//uniform int width;
//uniform float heigth;

// texture with the original rendering
uniform sampler2D screenTexture;


vec2 getTexelUnit(){
  vec2 pippo= vec2(1./width,1./heigth);
  return pippo;
}

float gaussianCoeff[31]={0.000012413329783081219,0.000040221571373342736,0.00012017860724662446,0.00033112430318893985,0.0008412952716973684,0.001971045418358334,0.004258265431659228,0.00848309004949083,0.015583274226409627,0.026396471051462745,0.04123028152882454,0.05938436230593951,0.07887067758612198,0.09659333160297948,0.10908490810935616,0.11359811921221667,0.10908490810935616,0.09659333160297948,0.07887067758612198,0.05938436230593951,0.04123028152882454,0.026396471051462745,0.015583274226409627,0.00848309004949083,0.004258265431659228,0.001971045418358334,0.0008412952716973684,0.00033112430318893985,0.00012017860724662446,0.000040221571373342736,0.000012413329783081219};
const vec4 Kernel0BracketsRealXY_ImZW = vec4(-0.000776,0.680418,0.000000,0.302524);
const vec2 Kernel0Weights_RealX_ImY = vec2(0.767583,1.862321);
const vec4 Kernel0_RealX_ImY_RealZ_ImW[] = vec4[](
        vec4(/*XY: Non Bracketed*/-0.000776,0.014351,/*Bracketed WZ:*/0.000000,0.047438),
        vec4(/*XY: Non Bracketed*/0.002486,0.015868,/*Bracketed WZ:*/0.004794,0.052452),
        vec4(/*XY: Non Bracketed*/0.006114,0.016730,/*Bracketed WZ:*/0.010127,0.055303),
        vec4(/*XY: Non Bracketed*/0.009926,0.016905,/*Bracketed WZ:*/0.015728,0.055881),
        vec4(/*XY: Non Bracketed*/0.013744,0.016417,/*Bracketed WZ:*/0.021340,0.054266),
        vec4(/*XY: Non Bracketed*/0.017413,0.015338,/*Bracketed WZ:*/0.026732,0.050701),
        vec4(/*XY: Non Bracketed*/0.020808,0.013780,/*Bracketed WZ:*/0.031722,0.045551),
        vec4(/*XY: Non Bracketed*/0.023843,0.011878,/*Bracketed WZ:*/0.036183,0.039262),
        vec4(/*XY: Non Bracketed*/0.026466,0.009777,/*Bracketed WZ:*/0.040037,0.032317),
        vec4(/*XY: Non Bracketed*/0.028659,0.007623,/*Bracketed WZ:*/0.043260,0.025198),
        vec4(/*XY: Non Bracketed*/0.030429,0.005554,/*Bracketed WZ:*/0.045863,0.018359),
        vec4(/*XY: Non Bracketed*/0.031804,0.003691,/*Bracketed WZ:*/0.047883,0.012201),
        vec4(/*XY: Non Bracketed*/0.032819,0.002136,/*Bracketed WZ:*/0.049374,0.007061),
        vec4(/*XY: Non Bracketed*/0.033511,0.000968,/*Bracketed WZ:*/0.050391,0.003201),
        vec4(/*XY: Non Bracketed*/0.033911,0.000245,/*Bracketed WZ:*/0.050980,0.000810),
        vec4(/*XY: Non Bracketed*/0.034043,0.000000,/*Bracketed WZ:*/0.051173,0.000000),
        vec4(/*XY: Non Bracketed*/0.033911,0.000245,/*Bracketed WZ:*/0.050980,0.000810),
        vec4(/*XY: Non Bracketed*/0.033511,0.000968,/*Bracketed WZ:*/0.050391,0.003201),
        vec4(/*XY: Non Bracketed*/0.032819,0.002136,/*Bracketed WZ:*/0.049374,0.007061),
        vec4(/*XY: Non Bracketed*/0.031804,0.003691,/*Bracketed WZ:*/0.047883,0.012201),
        vec4(/*XY: Non Bracketed*/0.030429,0.005554,/*Bracketed WZ:*/0.045863,0.018359),
        vec4(/*XY: Non Bracketed*/0.028659,0.007623,/*Bracketed WZ:*/0.043260,0.025198),
        vec4(/*XY: Non Bracketed*/0.026466,0.009777,/*Bracketed WZ:*/0.040037,0.032317),
        vec4(/*XY: Non Bracketed*/0.023843,0.011878,/*Bracketed WZ:*/0.036183,0.039262),
        vec4(/*XY: Non Bracketed*/0.020808,0.013780,/*Bracketed WZ:*/0.031722,0.045551),
        vec4(/*XY: Non Bracketed*/0.017413,0.015338,/*Bracketed WZ:*/0.026732,0.050701),
        vec4(/*XY: Non Bracketed*/0.013744,0.016417,/*Bracketed WZ:*/0.021340,0.054266),
        vec4(/*XY: Non Bracketed*/0.009926,0.016905,/*Bracketed WZ:*/0.015728,0.055881),
        vec4(/*XY: Non Bracketed*/0.006114,0.016730,/*Bracketed WZ:*/0.010127,0.055303),
        vec4(/*XY: Non Bracketed*/0.002486,0.015868,/*Bracketed WZ:*/0.004794,0.052452),
        vec4(/*XY: Non Bracketed*/-0.000776,0.014351,/*Bracketed WZ:*/0.000000,0.047438)
);
//float gaussianCoeff[61]={0.00047108753187090845,0.0006299954542634891,0.0008342476118513276,0.0010938911084409344,0.0014202820421456586,0.0018259812420950796,0.0023245510069993323,0.002930237054811048,0.00365752422443188,0.004520561042608063,0.005532457051297556,0.0067044675016952465,0.008045092096580709,0.009559127051781698,0.011246721745323141,0.013102501343509586,0.015114823683117182,0.017265241071896104,0.019528234511337204,0.021871278517810892,0.02425527914005976,0.02663540650524366,0.02896231750779124,0.031183735942891172,0.03324632882505008,0.03509779144363722,0.03668903252214707,0.037976337009618694,0.03892337931271162,0.03950296513759527,0.03969824751877432,0.03950296513759527,0.03892337931271162,0.037976337009618694,0.03668903252214707,0.03509779144363722,0.03324632882505008,0.031183735942891172,0.02896231750779124,0.02663540650524366,0.02425527914005976,0.021871278517810892,0.019528234511337204,0.017265241071896104,0.015114823683117182,0.013102501343509586,0.011246721745323141,0.009559127051781698,0.008045092096580709,0.0067044675016952465,0.005532457051297556,0.004520561042608063,0.00365752422443188,0.002930237054811048,0.0023245510069993323,0.0018259812420950796,0.0014202820421456586,0.0010938911084409344,0.0008342476118513276,0.0006299954542634891,0.00047108753187090845};
/////////////////////////////////////////////////////////////////////
// we must copy and paste the code inside our shaders
// it is not possible to include or to link an external file
////////////////////////////////////////////////////////////////////
// Description : Array and textureless GLSL 2D/3D/4D simplex
//               noise functions.
//      Author : Ian McEwan, Ashima Arts.
//  Maintainer : ijm
//     Lastmod : 20110822 (ijm)
//     License : Copyright (C) 2011 Ashima Arts. All rights reserved.
//               Distributed under the MIT License. See LICENSE file.
//               https://github.com/stegu/webgl-noise/
//

////////////////////////////////////////////////////////////////////

// the "type" of the Subroutine
subroutine vec3 blur_model();

// Subroutine Uniform (it is conceptually similar to a C pointer function)
subroutine uniform blur_model Blur_Model;


vec3 lumaCorrection(vec3 pixel){
  float lum = dot(pixel.rgb,vec3(0.2126,0.7152,0.0722))*1.8;
  vec3 colorImg = pixel *(1.0 + 0.2*lum*lum*lum);
  return colorImg*colorImg;
}


subroutine(blur_model)
vec3 GaussianBlur(){
  vec2 unit=getTexelUnit();
  vec4 color=vec4(0);
  
  for(int i=0;i<=2*BLUR_SIZE;i++ ){
    float x_offset=((i-BLUR_SIZE)*unit.x);
    //float x_offset=texCoords.x+((float)(i-BLUR_SIZE)*unit.x);
    vec2 offset= vec2(x_offset,0.0);
    color+=texture(screenTexture, texCoords.st+offset)*gaussianCoeff[i];
  }
  return vec3(color);
}

subroutine(blur_model)
vec3 NonGaussianBlur(){
  vec2 unit=getTexelUnit();
  vec4 color=vec4(0);
  int lineSize=(2*BLUR_SIZE)+1;
  for(int i=0;i<=2*BLUR_SIZE;i++ ){
    float x_offset=((i-BLUR_SIZE)*unit.x);
    //float x_offset=texCoords.x+((float)(i-BLUR_SIZE)*unit.x);
    vec2 offset= vec2(x_offset,0.0);
    color+=texture(screenTexture, texCoords.st+offset);
  }
  return vec3(color)*(1.0f/float(lineSize));
}

subroutine(blur_model)
vec3 DOFCircular(){
    //real part only
    vec2 unit = getTexelUnit();
    vec3 val = vec3(0.,0.,0.);
    float filterRadius = BLUR_SIZE;
    for (int i=-BLUR_SIZE; i <=BLUR_SIZE; ++i)
    {
        vec2 coords = texCoords + unit*vec2(float(i),0.0);
        vec3 imageTexel = vec3(texture(screenTexture, coords));
        imageTexel=lumaCorrection(imageTexel);
        float c0r = Kernel0_RealX_ImY_RealZ_ImW[i+BLUR_SIZE].x;
        val += imageTexel * c0r;
    }
    return val;
}

subroutine(blur_model)
vec3 DOFSquare(){
    //real part only
    vec2 unit = getTexelUnit();
    vec3 val = vec3(0.,0.,0.);
    float filterRadius = BLUR_SIZE;
    for (int i=-BLUR_SIZE; i <=BLUR_SIZE; ++i)
    {
        vec2 coords = texCoords + unit*vec2(float(i),0.0);
        vec3 imageTexel = vec3(texture(screenTexture, coords));
        imageTexel=lumaCorrection(imageTexel);
        float c0r = Kernel0_RealX_ImY_RealZ_ImW[i+BLUR_SIZE].z;
        val += imageTexel * c0r;
    }
    return val;
}

void main()
{
  vec3 blur=Blur_Model();
  FragColor = vec4(blur,1);
}