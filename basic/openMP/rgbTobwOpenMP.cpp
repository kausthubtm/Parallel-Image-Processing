#include "../../lodepng.h"
#include <omp.h>
#include <bits/stdc++.h>

using namespace std;


int main(int argc, char *argv[]) {

  const char* filename = argc > 1 ? argv[1] : "../../lena.png";
  double t1, t2;

  std::vector<unsigned char> in_image; 
  unsigned width, height;
  unsigned error = lodepng::decode(in_image, width, height, filename);
  if(error) std::cout << "decoder error " << error << ": " << lodepng_error_text(error) << std::endl;


  unsigned char* input_image = (unsigned char*) malloc((int)in_image.size());
  unsigned char* output_image = (unsigned char*) malloc((int)in_image.size());

  t1 = omp_get_wtime();
  #pragma omp parallel for num_threads(8)
  for(int k=0; k<5000; k++){
      for(int i=0; i< (int) in_image.size(); i=i+4){
        output_image[i] = in_image[i]*0.299 + in_image[i+1]*0.587 + in_image[i+2]*0.114; 
        output_image[i+1] = in_image[i]*0.299 + in_image[i+1]*0.587 + in_image[i+2]*0.114;
        output_image[i+2] = in_image[i]*0.299 + in_image[i+1]*0.587 + in_image[i+2]*0.114;
        output_image[i+3] = in_image[i]; 
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
