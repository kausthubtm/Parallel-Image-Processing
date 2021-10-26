#include "../../lodepng.h"
#include <bits/stdc++.h>
#include <omp.h>
using namespace std;

unsigned char rgb_image[2048][2048][3];

unsigned char find_avg(int y, int x, int color) {

    unsigned int ans=0;
    int YlowerLimit = y-3;
    int YupperLimit = y+3;
    int XlowerLimit = x-3;
    int XupperLimit = x+3;

    for(int i=YlowerLimit; i<=YupperLimit; i++){
        for(int j=XlowerLimit; j<=XupperLimit; j++){
            ans += rgb_image[i][j][color];
        }
    }

    return ans/49;

}


int main(int argc, char *argv[]) {

  double t1,t2;

  const char* filename = argc > 1 ? argv[1] : "../../lena.png";

  std::vector<unsigned char> in_image; 
  unsigned width, height;
  unsigned error = lodepng::decode(in_image, width, height, filename);
  if(error) std::cout << "decoder error " << error << ": " << lodepng_error_text(error) << std::endl;

  unsigned char* input_image = (unsigned char*) malloc((int)in_image.size());
  unsigned char* output_image = (unsigned char*) malloc((int)in_image.size());

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
  #pragma omp parallel for collapse(2)
  for(int k=0; k<1000;k++){
      for(int i=3; i<=(int)height+2; i++){
        for(int j=3; j<=(int)width+2; j++) {
          output_image[(i-3)*width*4 + (j-3)*4 + 0] = find_avg(i,j,0);
          output_image[(i-3)*width*4 + (j-3)*4 + 1] = find_avg(i,j,1);
          output_image[(i-3)*width*4 + (j-3)*4 + 2] = find_avg(i,j,2);
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