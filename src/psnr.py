# base source: https://www.geeksforgeeks.org/python-peak-signal-to-noise-ratio-psnr/

from math import log10, sqrt 
import sys
import cv2 
import numpy as np 
  
def PSNR(original, compressed): 
    mse = np.mean((original - compressed) ** 2) 
    if(mse == 0):   
        return 100
    max_pixel = 255.0
    psnr = 20 * log10(max_pixel / sqrt(mse)) 
    return psnr 
  
def main(): 
     original = cv2.imread(sys.argv[1]) 
     compressed = cv2.imread(sys.argv[2], 1) 
     value = PSNR(original, compressed) 
     print(f"PSNR value is {value} dB") 
       
if __name__ == "__main__": 
    main() 
