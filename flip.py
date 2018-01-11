import os
import skimage.draw as skdraw
from matplotlib import pyplot as plt
import numpy as np
import glob
import argparse
import shutil
import cv2 as cv
from joblib import Parallel, delayed


def convert_back(size, bbox):
    w, h = size
    obj_x = int(bbox[0]*w)
    obj_y = int(bbox[1]*h)
    obj_w = int(bbox[2]*w)
    obj_h = int(bbox[3]*h)
    return (
        obj_x - (obj_w // 2),
        obj_x + (obj_w // 2),
        obj_y - (obj_h // 2),
        obj_y + (obj_h // 2),
    )


def drawBB2(img, x, y, w, h):
    img_h, img_w, img_ch = img.shape
    x_min, x_max, y_min, y_max = convert_back((img_w, img_h), (x, y, w, h))
    print(img_w, img_h, x_min, x_max, y_min, y_max)
    # Vertical left
    img[y_min:y_max+1, x_min-1:x_min+1, 0] = 255
    img[y_min:y_max+1, x_min-1:x_min+1, 1:] = 0
    # Vertical left
    img[y_min:y_max+1, x_min-1:x_min+1, 0] = 255
    img[y_min:y_max+1, x_min-1:x_min+1, 1:] = 0
    # Vertical right
    img[y_min:y_max+1, x_max-1:x_max+1, 0] = 255
    img[y_min:y_max+1, x_max-1:x_max+1, 1:] = 0
    # Horizontal top
    img[y_min-1:y_min+1, x_min:x_max+1, 0] = 255
    img[y_min-1:y_min+1, x_min:x_max+1, 1:] = 0
    # Horizontal bottom
    img[y_max-1:y_max+1, x_min:x_max+1, 0] = 255
    img[y_max-1:y_max+1, x_min:x_max+1, 1:] = 0

    return img


def drawBB(img,x,y,w,h):
    rows = [y-int(h/2.),y+int(h/2.)]
    columns = [x-int(h/2.),x+int(w/2)]
    img[rows[0]-2:rows[0]+2,columns[0]:columns[1]+1,0] = 255
    img[rows[0]-2:rows[0]+2,columns[0]:columns[1]+1,1:] = 0
    img[rows[1]-2:rows[1]+2,columns[0]:columns[1]+1,0] = 255
    img[rows[1]-2:rows[1]+2,columns[0]:columns[1]+1,1:] = 0
    img[rows[0]:rows[1]+1,columns[0]-2:columns[0]+2,0] = 255
    img[rows[0]:rows[1]+1,columns[0]-2:columns[0]+2,1:] = 0
    img[rows[0]:rows[1]+1,columns[1]-2:columns[1]+2,0] = 255
    img[rows[0]:rows[1]+1,columns[1]-2:columns[1]+2,1:] = 0

    return img


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--data_dir", type=str, required=True, help="Directory of images.")
    parser.add_argument("--label_dir", type=str, required=True, help="Directory of labels.")
    parser.add_argument("--output_dir", type=str, default="outputs/flip", help="Directory of labels.")
    args = parser.parse_args()

    label_files = glob.glob("{}/*.txt".format(args.label_dir))

    flipdir = args.output_dir
    print flipdir

    flip_image_dir = os.path.join(flipdir, "images")
    flip_label_dir = os.path.join(flipdir, "labels")

    print flip_image_dir, flip_label_dir

    if os.path.exists(args.output_dir):
        shutil.rmtree(args.output_dir)
    os.makedirs(args.output_dir)

    if os.path.exists(flip_image_dir):
        print filp_image_dir,"exist"
    else:
        os.makedirs(flip_image_dir)
    if os.path.exists(flip_label_dir):
        print filp_label_dir,"exist"
    else:
        os.makedirs(flip_label_dir)


    #flipdir = os.path.join(args.output_dir, "flip")
    #if os.path.exists(flipdir):
        #os.makedirs(flipdir)
    #    print "flip dir exists"
    #else:
    #    os.makedirs(flipdir)


    draw_imgs = len(label_files)
    for i in range(draw_imgs):
        #select_idx = np.random.randint(len(label_files))
        select_idx = i
        print i, "of", len(label_files)

        # Label file
        label_file = label_files[select_idx]

        # Get filename
        filename = os.path.basename(label_file).split(".")[0]

        # Image file
        image_file = os.path.join(args.data_dir, "{}.jpg".format(filename))

        img = plt.imread(image_file)
        src_image_file = os.path.join(flip_image_dir, "{}.jpg".format(filename)) 

        flip_image_file = os.path.join(flip_image_dir, "{}_flip.jpg".format(filename)) 
        flip_label_file = os.path.join(flip_label_dir, "{}_flip.txt".format(filename))  
        print flip_image_file

        src = cv.imread(image_file);
        #dst = src
        #dst = cv.cv.CreateImage(cv.GetSize(src), src.depth, 3)
        dst = cv.flip(src, 1);
        cv.imwrite(flip_image_file, dst)
        cv.imwrite(src_image_file, src)
        R,C,Ch = img.shape

        print R, C, Ch
        print label_file
        print flip_label_file
        with open(label_file) as txt:
            label_file = file(flip_label_file, "a+")
            for line in txt:
                lst = line.split()
                index = float(lst[0])
                w = float(lst[3])
                h = float(lst[4])
                x = 1. - float(lst[1])
                y = float(lst[2])
                label_file.write("{} ".format(index))
                label_file.write("{} ".format(x))
                label_file.write("{} ".format(y))
                label_file.write("{} ".format(w))
                label_file.write("{}\n".format(h))
                # img = drawBB(img,int(float(lst[1])*C),int(float(lst[2])*R),int(float(lst[3])*C),int(float(lst[4])*R))
                #img = drawBB2(img,float(lst[1]),float(lst[2]),float(lst[3]),float(lst[4]))
            label_file.close()

#        plt.imsave(
#            os.path.join(args.output_dir, 'drawed_{}_{}.jpg'.format(i, filename)), 
#            img
#        )
        # plt.imshow(img)
        # plt.show()


if __name__ == "__main__":
    main()
