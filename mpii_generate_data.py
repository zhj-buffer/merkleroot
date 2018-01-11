import argparse
import functools
import json
import math
import os
import shutil

import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
from PIL import Image

from joblib import Parallel, delayed


"""
{
    "is_visible": {
        "11": 1,
        "10": 1,
        "13": 1,
        "12": 1,
        "15": 1,
        "14": 1,
        "1": 0,
        "0": 1,
        "3": 1,
        "2": 0,
        "4": 1,
        "7": 1,
        "6": 0,
        "9": 0,
        "8": 0
    },
    "head_rect": [
        749.0, --> xmin
        130.0, --> ymin
        845.0, --> xmax
        256.0  --> ymax
    ],
    "train": 1,
    "joint_pos": {
        "11": [
            702.0,  --> x
            161.0   --> y
        ],
        "10": [
            710.0,
            44.0
        ],
        "13": [
            874.0,
            285.0
        ],
        "12": [
            735.0,
            259.0
        ],
        "15": [
            880.0,
            24.0
        ],
        "14": [
            894.0,
            173.0
        ],
        "1": [
            800.0,
            590.0
        ],
        "0": [
            901.0,
            566.0
        ],
        "3": [
            823.0,
            523.0
        ],
        "2": [
            747.0,
            510.0
        ],
        "4": [
            673.0,
            606.0
        ],
        "7": [
            805.0,
            272.0
        ],
        "6": [
            785.0,
            517.0
        ],
        "9": [
            790.6163,
            129.9606
        ],
        "8": [
            803.3837,
            256.0394
        ]
    },
    "filename": "005147875.jpg"
}
"""

"""
[Darknet]

= [Directory] =
images/*.JPG
labels/*.txt

= [*.txt] =
label_id xmin xmax ymin ymax

"""

# 7 - thorax, 8 - upper neck, 9 - head top, 10 - r wrist, 11 - r elbow, 12 - r shoulder, 13 - l shoulder, 14 - l elbow, 15 - l wrist
mpii_id = {
    # Hand
    "left_wrist": "15",
    "right_wrist": "10",
    # Elbow
    "left_elbow": "14",
    "right_elbow": "11"
}

# classes = ["head", "bicycle", "bird", "hand", "bottle", "bus", "car", "cat", "chair", "cow", "diningtable", "dog", "horse", "motorbike", "person", "pottedplant", "sheep", "sofa", "foot", "tvmonitor"]
darknet_head_id = 0
darknet_hand_id = 3
darknet_person_id = 14
darknet_foot_id = 18


def convert(size, box):
    """Convert from size = (w, h) and box = (xmin, xmax, ymin, ymax) to x, y, w, h (copied from darknet)"""
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


def contain(point, box):
    "Check whether the box contains the specify point"
    x, y = point
    xmin, xmax, ymin, ymax = box
    if x >= xmin and x <= xmax and y >= ymin and y <= ymax:
        return True
    else:
        return False


def create_label_file(json_str, data_dir, output_dir):
    data = json.loads(json_str)
    fnames = data["filename"].split(".")

    img_file = os.path.join(data_dir, "images", data["filename"])
    img = plt.imread(img_file)
    image_h, image_w, image_c = img.shape

    # # Resize images
    # basewidth = 640
    # img = Image.open(img_file)
    # wpercent = (basewidth/float(img.size[0]))
    # hsize = int((float(img.size[1])*float(wpercent)))
    # img = img.resize((basewidth, hsize), Image.ANTIALIAS)
    # fname = os.path.basename(img_file)
    # name, _ = fname.split(".")
    # save_img_file = os.path.join(output_dir, "images", "{}.jpg".format(name))
    # img.save(save_img_file)

    # Use original images
    im = Image.open(img_file)
    fname = os.path.basename(img_file)
    name, _ = fname.split(".")
    im.save(os.path.join(
        output_dir,
        "images",
        "{}.jpg".format(name)
    ))

    with open(os.path.join(output_dir, "labels", "{}.txt".format(fnames[0])), "a") as out_f:
        ### Head ###
        head_box = (
            float(data["head_rect"][0]), 
            float(data["head_rect"][2]), 
            float(data["head_rect"][1]), 
            float(data["head_rect"][3])
        )
        head_bbox = convert((image_w, image_h), head_box)
        out_f.write(str(darknet_head_id) + " " + " ".join([str(a) for a in head_bbox]) + '\n')

        ### Hand ###
        # Estimate hand size based on head size
        head_w = head_box[1] - head_box[0]
        head_h = head_box[3] - head_box[2]
        hand_w = 1.0 * head_w
        hand_h = 1.0 * head_h

        def add_hand(side):
            if not side in ["left", "right"]:
                raise Exception("Invalid side")
            if data["is_visible"][mpii_id["{}_wrist".format(side)]]:
                hand_x, hand_y = data["joint_pos"][mpii_id["{}_wrist".format(side)]]
                hand_xmin = hand_x - (0.5 * hand_w)
                hand_xmax = hand_x + (0.5 * hand_w)
                hand_ymin = hand_y - (0.5 * hand_h)
                hand_ymax = hand_y + (0.5 * hand_h)
                hand_xmin = hand_xmin if hand_xmin >= 0 else 0
                hand_xmax = hand_xmax if hand_xmax < image_w else image_w-1
                hand_ymin = hand_ymin if hand_ymin >= 0 else 0
                hand_ymax = hand_ymax if hand_ymax < image_h else image_h-1
                adj_hand_w = abs(hand_xmin - hand_xmax)
                adj_hand_h = abs(hand_ymin - hand_ymax)

                # if data["is_visible"][mpii_id["{}_elbow".format(side)]]:
                elbow_x, elbow_y = data["joint_pos"][mpii_id["{}_elbow".format(side)]]
                dir_x = hand_x - elbow_x
                dir_y = hand_y - elbow_y
                dir_length = math.sqrt((dir_x * dir_x) + (dir_y * dir_y))
                unit_dir_x = dir_x / dir_length
                unit_dir_y = dir_y / dir_length
                # if abs(dir_x) > hand_w / 6:
                #     unit_dir_x = -1 if dir_x < 0 else 1
                # else:
                #     unit_dir_x = 0
                # if abs(dir_y) > hand_h / 6:
                #     unit_dir_y = -1 if dir_y < 0 else 1
                # else:
                #     unit_dir_y = 0
                shift_x = 0.2 * adj_hand_w * unit_dir_x
                shift_y = 0.2 * adj_hand_h * unit_dir_y
                hand_xmin += shift_x
                hand_xmax += shift_x
                hand_ymin += shift_y
                hand_ymax += shift_y

                # Include arm in hand label
                hand_xmin = min(hand_xmin, elbow_x)
                hand_xmax = max(hand_xmax, elbow_x)
                hand_ymin = min(hand_ymin, elbow_y)
                hand_ymax = max(hand_ymax, elbow_y)

                # Add hand box
                hand_box = (
                    hand_xmin,
                    hand_xmax,
                    hand_ymin,
                    hand_ymax
                )
                hand_bbox = convert((image_w, image_h), hand_box)
                out_f.write(str(darknet_hand_id) + " " + " ".join([str(a) for a in hand_bbox]) + '\n')

        add_hand(side="left")
        add_hand(side="right")

        # Estimate person box
        person_xmin = image_w + 1
        person_xmax = -1
        person_ymin = image_h + 1
        person_ymax = -1
        for k, v in data["joint_pos"].iteritems():
            joint_x, joint_y = v
            person_xmin = min(person_xmin, joint_x)
            person_xmax = max(person_xmax, joint_x)
            person_ymin = min(person_ymin, joint_y)
            person_ymax = max(person_ymax, joint_y)

        # Include head
        person_xmin = min([person_xmin, head_box[0], head_box[1]])
        person_xmax = max([person_xmax, head_box[0], head_box[1]])
        person_ymin = min([person_ymin, head_box[2], head_box[3]])
        person_ymax = max([person_ymax, head_box[2], head_box[3]])

        # Add person box
        person_box = (
            person_xmin,
            person_xmax,
            person_ymin,
            person_ymax
        )
        person_bbox = convert((image_w, image_h), person_box)
        out_f.write(str(darknet_person_id) + " " + " ".join([str(a) for a in person_bbox]) + '\n')


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--data_dir", type=str, default="data/mpii", help="Directory of original data.")
    parser.add_argument("--output_dir", type=str, default="outputs/mpii", help="Directory of output data.")
    args = parser.parse_args()

    image_dir = os.path.join(args.output_dir, "images")
    if os.path.exists(image_dir):
        shutil.rmtree(image_dir)
    os.makedirs(image_dir)

    label_dir = os.path.join(args.output_dir, "labels")
    if os.path.exists(label_dir):
        shutil.rmtree(label_dir)
    os.makedirs(label_dir)

    with open(os.path.join(args.data_dir, "data.json")) as f:
        lines = f.readlines()
        Parallel(n_jobs=12)(delayed(create_label_file)(
            json_str, 
            args.data_dir, 
            args.output_dir
        ) for json_str in lines)


if __name__ == "__main__":
    main()
