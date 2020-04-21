# base source: https://www.geeksforgeeks.org/python-peak-signal-to-noise-ratio-psnr/

from math import log10, sqrt 
import numpy as np 
import matplotlib.pyplot as plt
import sys
import subprocess
import cv2 
  
def PSNR(original, compressed): 
    mse = np.mean((original - compressed) ** 2) 
    if(mse == 0):   
        return 100
    max_pixel = 255.0
    psnr = 20 * log10(max_pixel / sqrt(mse)) 
    return psnr 
  
def main(): 
    original_path = sys.argv[1]
    colored_path = sys.argv[2]
    gray_path = sys.argv[3]
    original = cv2.imread(sys.argv[1]) 
    ext = sys.argv[4]
    psnrs = []
    for i in range(20):
        subprocess.run(("./../lib/build/WelshColorisation", "-c", colored_path, "-g", gray_path, "-d", "{}.{}".format(i, ext)))
        compressed = cv2.imread("{}.{}".format(i, ext), 1)
        psnr_res = PSNR(original, compressed)
        print(psnr_res)
        psnrs.append(psnr_res)
    plt.plot(range(20), psnrs)
    plt.xticks(range(20))
    plt.ylabel('dB')
    plt.xlabel('#run')
    plt.show()

if __name__ == "__main__": 
    main() 
