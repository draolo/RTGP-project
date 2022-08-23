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

float blurCoeff[31]={0.000012413329783081219,0.000040221571373342736,0.00012017860724662446,0.00033112430318893985,0.0008412952716973684,0.001971045418358334,0.004258265431659228,0.00848309004949083,0.015583274226409627,0.026396471051462745,0.04123028152882454,0.05938436230593951,0.07887067758612198,0.09659333160297948,0.10908490810935616,0.11359811921221667,0.10908490810935616,0.09659333160297948,0.07887067758612198,0.05938436230593951,0.04123028152882454,0.026396471051462745,0.015583274226409627,0.00848309004949083,0.004258265431659228,0.001971045418358334,0.0008412952716973684,0.00033112430318893985,0.00012017860724662446,0.000040221571373342736,0.000012413329783081219};
//float blurCoeff[61]={0.00047108753187090845,0.0006299954542634891,0.0008342476118513276,0.0010938911084409344,0.0014202820421456586,0.0018259812420950796,0.0023245510069993323,0.002930237054811048,0.00365752422443188,0.004520561042608063,0.005532457051297556,0.0067044675016952465,0.008045092096580709,0.009559127051781698,0.011246721745323141,0.013102501343509586,0.015114823683117182,0.017265241071896104,0.019528234511337204,0.021871278517810892,0.02425527914005976,0.02663540650524366,0.02896231750779124,0.031183735942891172,0.03324632882505008,0.03509779144363722,0.03668903252214707,0.037976337009618694,0.03892337931271162,0.03950296513759527,0.03969824751877432,0.03950296513759527,0.03892337931271162,0.037976337009618694,0.03668903252214707,0.03509779144363722,0.03324632882505008,0.031183735942891172,0.02896231750779124,0.02663540650524366,0.02425527914005976,0.021871278517810892,0.019528234511337204,0.017265241071896104,0.015114823683117182,0.013102501343509586,0.011246721745323141,0.009559127051781698,0.008045092096580709,0.0067044675016952465,0.005532457051297556,0.004520561042608063,0.00365752422443188,0.002930237054811048,0.0023245510069993323,0.0018259812420950796,0.0014202820421456586,0.0010938911084409344,0.0008342476118513276,0.0006299954542634891,0.00047108753187090845};
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


subroutine(blur_model)
vec3 GaussianBlur(){
  vec2 unit=getTexelUnit();
  vec4 color=vec4(0);
  
  for(int i=0;i<=2*BLUR_SIZE;i++ ){
    float x_offset=((i-BLUR_SIZE)*unit.x);
    //float x_offset=texCoords.x+((float)(i-BLUR_SIZE)*unit.x);
    vec2 offset= vec2(x_offset,0.0);
    color+=texture(screenTexture, texCoords.st+offset)*blurCoeff[i];
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

void main()
{
  vec3 blur=Blur_Model();
  FragColor = vec4(blur,1);
}