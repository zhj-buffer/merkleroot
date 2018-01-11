#! /usr/bin/python
# -*- coding: utf8 -*-


"""
Replace green background
"""
import tensorlayer as tl
import numpy as np
from skimage import color
## https://mathematica.stackexchange.com/questions/132844/nontrivial-background-removal
import numpy as np
import cv2
import skimage
from skimage import color
from joblib import Parallel, delayed

def replace_bg(idx, path_fg, path_fg_imgs, path_bg, path_bg_imgs, w, h, save_dir,):

    path_fg_img = path_fg_imgs[idx]
    im = tl.vis.read_image(path_fg_img, path=path_fg)
    # print(im.shape)
    ## resize foreground image
    im = tl.prepro.imresize(im, [h, w], interp='bicubic')

    print("%d: %s min:%f  max:%f" % (idx, path_fg_img, im.min(), im.max()))

    ## get foreground HSV
    hsv = cv2.cvtColor(im, cv2.COLOR_BGR2HSV)

    ## define range of green color in HSV
    #  Green H = 60
    # lower_green = np.array([50,100,50])   # <-- baseline
    # lower_green = np.array([35,100,50])
    # lower_green = np.array([40,100,40])   # <-- remove more
    # lower_green = np.array([35,100,35])   # <-- remove more
    lower_green = np.array([35,90,45])      # <-- remove more

    # upper_green = np.array([70,255,255])  # <-- baseline
    upper_green = np.array([85,255,255])    # <-- remove more

    ## Threshold the HSV image to get only blue colors
    mask = cv2.inRange(hsv, lower_green, upper_green)
    mask = cv2.bitwise_not(mask)

    ## mask erosion
    mask = tl.prepro.binary_erosion(mask, radius=2) # [1]
    mask.dtype = np.uint8
    mask = mask * 255

    ## remove background on foreground images
    im_ = cv2.bitwise_and(im, im, mask = mask)

    ## random change foreground brightness  # small --> brighter
    gamma = np.random.uniform(.5, 5.0)
    im_ = tl.prepro.brightness(im_, gamma=gamma, gain=1, is_random=False)

    ## get 1 background image
    i = tl.utils.get_random_int(min=0, max=len(path_bg_imgs)-1, number=1, seed=None)[0]
    imbg = tl.vis.read_image(path_bg_imgs[i], path=path_bg)

    ## deal with grey-scale image
    # print(imbg.shape)
    if len(imbg.shape) == 2:    # [h, w] --> [h, w, 3]
        imbg = imbg[:, :, np.newaxis]
        imbg = np.tile(imbg, (1, 1, 3))
    elif imbg.shape[-1] == 4:   # RGBA --> RGB
        imbg = imbg[:,:,0:3]
    # print(imbg.shape)

    ## resize background image
    imbg = tl.prepro.imresize(imbg, [h, w], interp='bicubic')
    print("   %s min:%f  max:%f" % (path_bg_imgs[i], imbg.min(), imbg.max()))

    ## random change background brightness
    gamma = np.random.uniform(0.5, 2.0)
    imbg = tl.prepro.brightness(imbg, gamma=gamma, gain=1, is_random=False)

    ## replace background
    mask_fg = cv2.bitwise_not(mask)
    imbg = cv2.bitwise_and(imbg, imbg, mask = mask_fg)
    im_ = im_ + imbg

    ## whole image - mean/averaged filter
    kernel = np.ones((3,3),np.float32)/9
    im_ = cv2.filter2D(im_, -1, kernel)

    ## whole image - dark
    scale = np.random.uniform(1.0, 1.5)
    im_ = im_ / scale

    ## save results
    # tl.vis.save_image(im, '_%d_imo.png' % idx)
    tl.vis.save_image(im_, save_dir + '/' +path_fg_img)




def main():
    h, w = 2976, 2976
    save_dir = "results2"
    tl.files.exists_or_mkdir(save_dir)
    # h, w = 800, 1200
    # h, w = 1200, 1200
    # fg_images = load_image_from_folder("data/foreground")
    path_fg = "data/fg20170915"
    path_fg_imgs = tl.files.load_file_list(path=path_fg, regx='\\.jpg', printable=True) + \
        tl.files.load_file_list(path=path_fg, regx='\\.JPG', printable=True) + \
        tl.files.load_file_list(path=path_fg, regx='\\.jpeg', printable=True) + \
        tl.files.load_file_list(path=path_fg, regx='\\.png', printable=True)

    path_bg = "data/background2"
    path_bg_imgs = tl.files.load_file_list(path=path_bg, regx='\\.jpg', printable=True) + \
        tl.files.load_file_list(path=path_bg, regx='\\.JPG', printable=True) + \
        tl.files.load_file_list(path=path_bg, regx='\\.jpeg', printable=True) + \
        tl.files.load_file_list(path=path_bg, regx='\\.png', printable=True)


    Parallel(n_jobs=72)(delayed(replace_bg)(
            i, path_fg, path_fg_imgs, path_bg, path_bg_imgs, w, h, save_dir
            ) for i in range(len(path_fg_imgs)))


#    for idx, path_fg_img in enumerate(path_fg_imgs):
        ## read 1 foreground image

if __name__ == "__main__":
    main()
