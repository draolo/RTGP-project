#version 410 core

#define MAX_OFFSET 0.1

out vec4 FragColor;
in vec2 texCoords;

// force and power of noise
uniform float frequency;

// number of octaves to create and sum
uniform float harmonics;

//
uniform vec2 normalizedContactPoints[16];
uniform float powers[16];

uniform int contactPointNumber;

// texture with the original rendering
uniform sampler2D screenTexture;

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

vec3 mod289(vec3 x) {
  return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec4 mod289(vec4 x) {
  return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec4 permute(vec4 x) {
     return mod289(((x*34.0)+1.0)*x);
}

vec4 taylorInvSqrt(vec4 r)
{
  return 1.79284291400159 - 0.85373472095314 * r;
}

float snoise(vec3 v)
  {
  const vec2  C = vec2(1.0/6.0, 1.0/3.0) ;
  const vec4  D = vec4(0.0, 0.5, 1.0, 2.0);

// First corner
  vec3 i  = floor(v + dot(v, C.yyy) );
  vec3 x0 =   v - i + dot(i, C.xxx) ;

// Other corners
  vec3 g = step(x0.yzx, x0.xyz);
  vec3 l = 1.0 - g;
  vec3 i1 = min( g.xyz, l.zxy );
  vec3 i2 = max( g.xyz, l.zxy );

  //   x0 = x0 - 0.0 + 0.0 * C.xxx;
  //   x1 = x0 - i1  + 1.0 * C.xxx;
  //   x2 = x0 - i2  + 2.0 * C.xxx;
  //   x3 = x0 - 1.0 + 3.0 * C.xxx;
  vec3 x1 = x0 - i1 + C.xxx;
  vec3 x2 = x0 - i2 + C.yyy; // 2.0*C.x = 1/3 = C.y
  vec3 x3 = x0 - D.yyy;      // -1.0+3.0*C.x = -0.5 = -D.y

// Permutations
  i = mod289(i);
  vec4 p = permute( permute( permute(
             i.z + vec4(0.0, i1.z, i2.z, 1.0 ))
           + i.y + vec4(0.0, i1.y, i2.y, 1.0 ))
           + i.x + vec4(0.0, i1.x, i2.x, 1.0 ));

// Gradients: 7x7 points over a square, mapped onto an octahedron.
// The ring size 17*17 = 289 is close to a multiple of 49 (49*6 = 294)
  float n_ = 0.142857142857; // 1.0/7.0
  vec3  ns = n_ * D.wyz - D.xzx;

  vec4 j = p - 49.0 * floor(p * ns.z * ns.z);  //  mod(p,7*7)

  vec4 x_ = floor(j * ns.z);
  vec4 y_ = floor(j - 7.0 * x_ );    // mod(j,N)

  vec4 x = x_ *ns.x + ns.yyyy;
  vec4 y = y_ *ns.x + ns.yyyy;
  vec4 h = 1.0 - abs(x) - abs(y);

  vec4 b0 = vec4( x.xy, y.xy );
  vec4 b1 = vec4( x.zw, y.zw );

  //vec4 s0 = vec4(lessThan(b0,0.0))*2.0 - 1.0;
  //vec4 s1 = vec4(lessThan(b1,0.0))*2.0 - 1.0;
  vec4 s0 = floor(b0)*2.0 + 1.0;
  vec4 s1 = floor(b1)*2.0 + 1.0;
  vec4 sh = -step(h, vec4(0.0));

  vec4 a0 = b0.xzyw + s0.xzyw*sh.xxyy ;
  vec4 a1 = b1.xzyw + s1.xzyw*sh.zzww ;

  vec3 p0 = vec3(a0.xy,h.x);
  vec3 p1 = vec3(a0.zw,h.y);
  vec3 p2 = vec3(a1.xy,h.z);
  vec3 p3 = vec3(a1.zw,h.w);


//Normalise gradients
  vec4 norm = taylorInvSqrt(vec4(dot(p0,p0), dot(p1,p1), dot(p2, p2), dot(p3,p3)));
  p0 *= norm.x;
  p1 *= norm.y;
  p2 *= norm.z;
  p3 *= norm.w;

// Mix final noise value
  vec4 m = max(0.6 - vec4(dot(x0,x0), dot(x1,x1), dot(x2,x2), dot(x3,x3)), 0.0);
  m = m * m;
  return 42.0 * dot( m*m, vec4( dot(p0,x0), dot(p1,x1),
                                dot(p2,x2), dot(p3,x3) ) );
                                
  }

////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////

// aastep function calculates the length of the gradient given from the difference between the current fragment and the neighbours on the right and on the top. 
// We can then apply a smoothstep function using as threshold the value given by the gradient.
float aastep(float threshold, float value) {
  float afwidth = 0.7 * length(vec2(dFdx(value), dFdy(value)));

  return smoothstep(threshold-afwidth, threshold+afwidth, value);
}
////////////////////////////////////////////////////////////////////


int TurbulenceAAstep(float power, vec2 pos,int t)
{

  float p = power;
  float f = frequency*t;
  float value = 0.0;
  float h=harmonics;
  for (int i=0;i<h;i++)
  {
      value += p*snoise(vec3((texCoords+pos)*f, 0));
      p*=0.5;
      f*=2.0;
  }

  // we apply aastep to the turbulence result to obtain a "cow skin" effect
  float keep = aastep(0.05,value);
  if(keep<0.5){
    return 0;
  }
 /* if(keep!=1.){
  return vec4(1.0,0.0,0.0,1.0);
  }*/
  //in this case, we are creating a grayscale image
  return 1;
}

void main()
{
    //for(int i = 0; i < 9; i++)
    //    color += vec3(texture(screenTexture, texCoords.st + offsets[i])) * kernel[i];
   // FragColor = TurbulenceAAstep();//vec4(color, 1.0f);
    int impacted=0;
    int n=1;
    for(int i=0;i<contactPointNumber;i++){
      vec2 connecting=texCoords.st-normalizedContactPoints[i];
      //if no power or far away from the impact point skip
      if((powers[i]==0)||(length(connecting)>MAX_OFFSET+0.15)){
        continue;
      }

      //add perling noise loop th the the border of the impacted area
      float x=dot(normalize(connecting), vec2(1,0));
      float a= acos(x);
      float y=sin(a);
      //coud be any offset, just to have a more randomic shape
      vec2 offset=vec2(x,y)+texCoords.st;
      float noise= snoise(vec3(offset*1.7,0))*MAX_OFFSET;
      
      if((length(connecting)<noise+0.15)){
        impacted+=TurbulenceAAstep(powers[i],normalizedContactPoints[i],n);
        n++;
      }
    }
    if(impacted!=0){
      FragColor = vec4(vec3(0), 1.0);//vec4(color, 1.0f);
    }else{
      //FragColor = TurbulenceAAstep(1);//vec4(color, 1.0f);
      FragColor=texture(screenTexture, texCoords.st);
    }
}