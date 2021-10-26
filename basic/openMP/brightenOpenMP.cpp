#include "../../lodepng.h"
#include <bits/stdc++.h>
#include <omp.h>
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
  #pragma omp parallel for num_threads(4)
  for(int k=0; k<1000; k++){
      for(int i=0; i< (int) in_image.size(); i++) {
        output_image[i] = min(1.5*in_image[i], 255.0);
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