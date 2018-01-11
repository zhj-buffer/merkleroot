import xml.etree.ElementTree as ET
import pickle
import os
from os import listdir
from os.path import join
import shutil
import argparse

classes = ["head", "bicycle", "bird", "hand", "bottle", "bus", "car", "cat", "chair", "cow", "diningtable", "dog", "horse", "motorbike", "person", "pottedplant", "sheep", "sofa", "train", "tvmonitor"]
classes_lv2 = ["head", "hand"]

ALLOW_EXTENSIONS = ["png", "jpg", "bmp", "jpeg"]

def convert(size, box):
    dw = 1./size[0]
    dh = 1./size[1]
    x = (box[0] + box[1])/2.0
    y = (box[2] + box[3])/2.0
    w = box[1] - box[0]
    h = box[3] - box[2]
    x = x*dw
    w = w*dw
    y = y*dh
    h = h*dh
    return (x,y,w,h)

def convert_annotation(label_dir, output_dir, img_fname):
    in_file = open(os.path.join(label_dir, '{}.xml'.format(img_fname)))
    out_file = open(os.path.join(output_dir, '{}.txt'.format(img_fname)), 'w')
    tree = ET.parse(in_file)
    root = tree.getroot()
    size = root.find('size')
    w = int(size.find('width').text)
    h = int(size.find('height').text)

    for obj in root.iter('object'):
        difficult = obj.find('difficult').text
        cls = obj.find('name').text
        if cls not in classes or int(difficult) == 1:
            continue
        cls_id = classes.index(cls)
        xmlbox = obj.find('bndbox')
        b = (float(xmlbox.find('xmin').text), float(xmlbox.find('xmax').text), float(xmlbox.find('ymin').text), float(xmlbox.find('ymax').text))
        bb = convert((w,h), b)
        out_file.write(str(cls_id) + " " + " ".join([str(a) for a in bb]) + '\n')

        if cls in "person":
            for part in obj.iter('part'):
                cls = part.find('name').text
                if cls not in classes_lv2:
                    continue
                cls_id = classes.index(cls)
                xmlbox = part.find('bndbox')
                b = (float(xmlbox.find('xmin').text), float(xmlbox.find('xmax').text), float(xmlbox.find('ymin').text), float(xmlbox.find('ymax').text))
                bb = convert((w,h), b)
                out_file.write(str(cls_id) + " " + " ".join([str(a) for a in bb]) + '\n')

    in_file.close()
    out_file.close()


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--data_dir", type=str, default="outputs/manual", help="Directory of original data.")
    args = parser.parse_args()

    img_dir = os.path.join(args.data_dir, "images")
    label_dir = os.path.join(args.data_dir, "tmp_labels")
    output_dir = os.path.join(args.data_dir, "labels")

    if os.path.exists(output_dir):
        shutil.rmtree(output_dir)
    os.makedirs(output_dir)

    for f in os.listdir(img_dir):
        fname, fext = os.path.basename(f).split(".")
        if fext.lower() in ALLOW_EXTENSIONS:
            convert_annotation(
                label_dir=label_dir, 
                output_dir=output_dir, 
                img_fname=fname
            )


if __name__ == "__main__":
    main()
