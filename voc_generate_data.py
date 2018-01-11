import xml.etree.ElementTree as ET
import pickle
import os
from os import listdir, getcwd
from os.path import join
import argparse
import shutil

sets=[('2012', 'train'), ('2012', 'val'), ('2007', 'train'), ('2007', 'val'), ('2007', 'test')]

# classes = ["aeroplane", "bicycle", "bird", "boat", "bottle", "bus", "car", "cat", "chair", "cow", "diningtable", "dog", "horse", "motorbike", "person", "pottedplant", "sheep", "sofa", "train", "tvmonitor"]
classes = ["head", "bicycle", "bird", "hand", "bottle", "bus", "car", "cat", "chair", "cow", "diningtable", "dog", "horse", "motorbike", "person", "pottedplant", "sheep", "sofa", "train", "tvmonitor"]
classes_lv2 = ["head", "hand"]

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

def convert_annotation(data_dir, output_dir, year, image_id):
    input_fpath = os.path.join(data_dir, 'VOC{}/Annotations/{}.xml'.format(year, image_id))
    output_fpath = os.path.join(output_dir, 'labels_voc{}/{}.txt'.format(year, image_id))
    in_file = open(input_fpath)
    out_file = open(output_fpath, 'w')
    tree = ET.parse(in_file)
    root = tree.getroot()
    size = root.find('size')
    w = int(size.find('width').text)
    h = int(size.find('height').text)
    n_objs = 0

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
        n_objs += 1

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
                n_objs += 1

    in_file.close()
    out_file.close()

    if n_objs == 0:
        try:
            os.remove(output_fpath)
        except OSError:
            pass

    return n_objs


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--data_dir", type=str, default="data/voc", help="Directory of original data.")
    parser.add_argument("--output_dir", type=str, default="outputs/voc", help="Directory of output data.")
    args = parser.parse_args()

    voc_dir = os.path.join(args.data_dir, "VOCdevkit")

    for year, image_set in sets:
        label_dir = os.path.join(args.output_dir, 'labels_voc{}'.format(year))
        image_dir = os.path.join(args.output_dir, 'images_voc{}'.format(year))
        if not os.path.exists(label_dir):
            os.makedirs(label_dir)
        if not os.path.exists(image_dir):
            os.makedirs(image_dir)
        image_ids = open(os.path.join(voc_dir, 'VOC{}/ImageSets/Main/{}.txt'.format(year, image_set))).read().strip().split()
        list_file = open(os.path.join(args.output_dir, 'voc{}_{}.txt'.format(year, image_set)), 'w')
        for image_id in image_ids:
            n_objs = convert_annotation(
                voc_dir,
                args.output_dir,
                year,
                image_id
            )
            if n_objs > 0:
                src_image_path = os.path.join(voc_dir, 'VOC{}'.format(year), 'JPEGImages', '{}.jpg'.format(image_id))
                dst_image_path = os.path.join(os.getcwd(), image_dir, '{}.jpg'.format(image_id))
                shutil.copyfile(src_image_path, dst_image_path)
                list_file.write(dst_image_path + "\n")

        list_file.close()


if __name__ == "__main__":
    main()
