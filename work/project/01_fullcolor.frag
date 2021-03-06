/*

01_fullcolor.frag : basic Fragment shader, it applies an uniform color to all the fragments. Colo is passed as uniform from the main application

N.B.)  "00_basic.vert" must be used as vertex shader

author: Davide Gadia

Real-Time Graphics Programming - a.a. 2020/2021
Master degree in Computer Science
Universita' degli Studi di Milano

*/

#version 410 core

// Outputs colors in RGBA
out vec4 FragColor;

// color to assign to the fragments: it is passed from the application
uniform vec3 colorIn;

void main()
{
    FragColor = vec4(colorIn,1);
}
