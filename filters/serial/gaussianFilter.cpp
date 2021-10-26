#include "../../lodepng.h"
#include <bits/stdc++.h>
#include <omp.h>
using namespace std;

unsigned char rgb_image[2048][2048][3];

unsigned char find_avg(int y, int x, int color, double kernel[7][7]) {

    double ans=0;

    int YlowerLimit = y-3;
    int YupperLimit = y+3;
    int XlowerLimit = x-3;
    int XupperLimit = x+3;

    for(int i=YlowerLimit; i<=YupperLimit; i++){
        for(int j=XlowerLimit; j<=XupperLimit; j++){
            ans = ans + ((int)rgb_image[i][j][color])*kernel[i-YlowerLimit][j-XlowerLimit];
        }
    }

    return ans;

}


int main(int argc, char *argv[]) {

  const char* filename = argc > 1 ? argv[1] : "../../lena.png";

  std::vector<unsigned char> in_image; 
  unsigned width, height;
  unsigned error = lodepng::decode(in_image, width, height, filename);
  if(error) std::cout << "decoder error " << error << ": " << lodepng_error_text(error) << std::endl;

  unsigned char* input_image = (unsigned char*) malloc((int)in_image.size());
  unsigned char* output_image = (unsigned char*) malloc((int)in_image.size());



  // gaussian 7x7 kernel
  double kernel[7][7];
  double stDev = 13.0; 
  double r, s = 2.0 * stDev * stDev;
  double t1, t2;
 
  // sum is for normalization
  double sum = 0.0;
 
  for (int x = -3; x <= 3; x++) {
    for (int y = -3; y <= 3; y++) {
        r = sqrt(x * x + y * y);
        kernel[x + 3][y + 3] = (exp(-(r * r) / s)) / (M_PI * s);
        sum += kernel[x + 3][y + 3];
    }
  }
 
  // normalising the Kernel
  for (int i = 0; i < 7; i++)
    for (int j = 0; j < 7; j++)
        kernel[i][j] /= sum;


  // forming the padded image and rgb channels
  for(int i=0; i< (int) height; i++) {
      for(int j=0; j< (int) width; j++){
          if(i<=2 || i>=(int) height+3 || j<=2 || j>=(int)width+3){
            rgb_image[i][j][0] = 0;
            rgb_image[i][j][1] = 0;
            rgb_image[i][j][2] = 0;
          }
          else {
            rgb_image[i][j][0] = in_image[i*width*4 + j*4 + 0];
            rgb_image[i][j][1] = in_image[i*width*4 + j*4 + 1];
            rgb_image[i][j][2] = in_image[i*width*4 + j*4 + 2];
          }
          
      }
  }

  // applying kernel i.e., avg of surrounding pixels
  t1 = omp_get_wtime();
  for(int k=0; k<1000; k++){
    for(int i=3; i<=(int)height+2; i++){
      for(int j=3; j<=(int)width+2; j++) {
          output_image[(i-3)*width*4 + (j-3)*4 + 0] = find_avg(i,j,0,kernel);
          output_image[(i-3)*width*4 + (j-3)*4 + 1] = find_avg(i,j,1,kernel);
          output_image[(i-3)*width*4 + (j-3)*4 + 2] = find_avg(i,j,2,kernel);
          output_image[(i-3)*width*4 + (j-3)*4 + 3] = 255;
      }
    }
  }
  t2 = omp_get_wtime();
  cout<<t2-t1<<endl;
  
  


  error = lodepng_encode32_file("../../output.png", output_image, width, height);
  if(error) printf("error %u: %s\n", error, lodepng_error_text(error));

  free(input_image);
  free(output_image);
  return 0;

}