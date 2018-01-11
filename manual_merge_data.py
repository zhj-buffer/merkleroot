import glob
import os
from PIL import Image
import shutil

from joblib import Parallel, delayed
import argparse

ALLOW_FILE_EXTENSIONS = ["jpg", "jpeg", "png"]
SAVE_EXTENSION = "jpg"

def copy_image_label(img_label_detail, save_img_dir, save_label_dir):
    img_file, label_file, save_fname = img_label_detail

    print("Copying filename: {}".format(save_fname))

    im = Image.open(img_file)
    im.save(os.path.join(
        save_img_dir,
        ".".join([save_fname, SAVE_EXTENSION])
    ))
    shutil.copyfile(
        label_file,
        os.path.join(
            save_label_dir, 
            save_fname + ".xml"
        )
    )


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--data_dir", type=str, default="data/manual", help="Directory of original data.")
    parser.add_argument("--output_dir", type=str, default="outputs/manual", help="Directory of output data.")
    args = parser.parse_args()

    save_image_dir = os.path.join(args.output_dir, "images")
    save_label_dir = os.path.join(args.output_dir, "tmp_labels")

    if os.path.exists(save_image_dir):
        shutil.rmtree(save_image_dir)
    os.makedirs(save_image_dir)

    if os.path.exists(save_label_dir):
        shutil.rmtree(save_label_dir)
    os.makedirs(save_label_dir)

    all_img_label_details = []
    for path, subdirs, files in os.walk(args.data_dir):
        for name in files:
            if ".xml" in name.lower():
                fname, fext = name.split(".")
                relative_path = path.replace(args.data_dir, "")
                save_fname = "_".join(relative_path.split("/")[1:]) + "_" + fname
                for allow_ext in ALLOW_FILE_EXTENSIONS:
                    img_file = os.path.join(
                        path,
                        ".".join([fname, allow_ext])
                    )
                    label_file = os.path.join(path, name)
                    if os.path.isfile(img_file):
                        all_img_label_details.append((img_file, label_file, save_fname))
                        break

    Parallel(n_jobs=12)(delayed(copy_image_label)(
        d, 
        save_image_dir, 
        save_label_dir
    ) for d in all_img_label_details)


if __name__ == '__main__':
    main()
