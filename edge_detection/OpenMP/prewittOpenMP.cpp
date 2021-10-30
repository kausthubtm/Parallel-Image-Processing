#include "../../lodepng.h"
#include <omp.h>
#include <bits/stdc++.h>

using namespace std;

unsigned char conv(int y, int x, int color, unsigned char rgb_image[821][1026][3], double kernel[3][3]) {

    double ans=0;

    int YlowerLimit = y-1;
    int YupperLimit = y+1;
    int XlowerLimit = x-1;
    int XupperLimit = x+1;

    for(int i=YlowerLimit; i<=YupperLimit; i++){
        for(int j=XlowerLimit; j<=XupperLimit; j++){
            ans = ans + ((int)rgb_image[i][j][color])*kernel[i-YlowerLimit][j-XlowerLimit];
        }
    }

    return ans;

}


int main(int argc, char *argv[]) {

  const char* filename = argc > 1 ? argv[1] : "../../lena.png";
  double t1, t2;

  std::vector<unsigned char> in_image; 
  unsigned width, height;
  unsigned error = lodepng::decode(in_image, width, height, filename);
  if(error) std::cout << "decoder error " << error << ": " << lodepng_error_text(error) << std::endl;

  unsigned char* input_image = (unsigned char*) malloc((int)in_image.size());
  unsigned char* bw_image = (unsigned char*) malloc((int)in_image.size());
  unsigned char* output_image = (unsigned char*) malloc((int)in_image.size());

  unsigned char padded_image[821][1026][3];

  // rgb image
  t1 = omp_get_wtime();
  int i;

  #pragma omp parallel for collapse(2) firstprivate(in_image) private(i)
  for(int k=0; k<10000; k++){
    for(i=0; i< (int) in_image.size(); i=i+4){
      bw_image[i] = in_image[i]*0.299 + in_image[i+1]*0.587 + in_image[i+2]*0.114; 
      bw_image[i+1] = in_image[i]*0.299 + in_image[i+1]*0.587 + in_image[i+2]*0.114;
      bw_image[i+2] = in_image[i]*0.299 + in_image[i+1]*0.587 + in_image[i+2]*0.114;
      bw_image[i+3] = in_image[i]; 
    }
  }

  // forming the padded image
  for(int i=0; i< (int) height; i++) {
      for(int j=0; j< (int) width; j++){
          if(i==0 || i==(int)height+1 || j==2 || j==(int)width+1){
            padded_image[i][j][0] = 0;
            padded_image[i][j][1] = 0;
            padded_image[i][j][2] = 0;
          }
          else {
            padded_image[i][j][0] = bw_image[i*width*4 + j*4 + 0];
            padded_image[i][j][1] = bw_image[i*width*4 + j*4 + 1];
            padded_image[i][j][2] = bw_image[i*width*4 + j*4 + 2];
          }
          
      }
  }

  unsigned int temp, threshold= 170;

  //kernels
  double kernel1[3][3] = {{-1,0,1},{-1,0,1},{-1,0,1}};
  double kernel2[3][3] = {{-1,-1,-1},{0,0,0},{1,1,1}};

  // applying kernels
  #pragma omp parallel for collapse(3) private(temp)
  for(int k=0; k<10000; k++){
      for(int i=1; i<=(int)height; i++){
        for(int j=1; j<=(int)width; j++) {
          temp = sqrt( pow(conv(i,j,0,padded_image,kernel1),2) + pow(conv(i,j,0,padded_image,kernel2),2) );
          if(temp > threshold){
            output_image[(i-1)*width*4 + (j-1)*4 + 0] = 0;
            output_image[(i-1)*width*4 + (j-1)*4 + 1] = 0;
            output_image[(i-1)*width*4 + (j-1)*4 + 2] = 0;
            output_image[(i-1)*width*4 + (j-1)*4 + 3] = 255;
          }
          else {
            output_image[(i-1)*width*4 + (j-1)*4 + 0] = min(2.0*temp, 255.0);
            output_image[(i-1)*width*4 + (j-1)*4 + 1] = min(2.0*temp, 255.0);
            output_image[(i-1)*width*4 + (j-1)*4 + 2] = min(2.0*temp, 255.0);
            output_image[(i-1)*width*4 + (j-1)*4 + 3] = 255;
          }
      }
    }
  }
  t2 = omp_get_wtime();
  printf("%f\n",t2-t1);

  error = lodepng_encode32_file("../../output.png", output_image, width, height);
  if(error) printf("error %u: %s\n", error, lodepng_error_text(error));

  free(input_image);
  free(output_image);
  return 0;

}

