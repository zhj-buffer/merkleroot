# Prepare Human Datasets

## Environment ##
- Ubuntu 16.04
- CUDA 8.0 and cuDNN 5.1
- OpenCV 2.4.13 ([https://gist.github.com/arthurbeggs/06df46af94af7f261513934e56103b30](https://gist.github.com/arthurbeggs/06df46af94af7f261513934e56103b30))
- Python 2.7


## Python package ##
- numpy
- scipy
- pillow
- matplotlib
- scikit-image
- joblib


## 1. Create directories ##

    mkdir data
    mkdir outputs


## 2. MPII dataset ##
Run the following commands to download and generate MPII datasets.

    echo "Download dataset ..."
    mkdir data/mpii
    cd data/mpii
    wget http://datasets.d2.mpi-inf.mpg.de/andriluka14cvpr/mpii_human_pose_v1.tar.gz
    wget http://datasets.d2.mpi-inf.mpg.de/andriluka14cvpr/mpii_human_pose_v1_u12_2.zip
    tar -xzvf mpii_human_pose_v1.tar.gz
    unzip mpii_human_pose_v1_u12_2.zip

    echo "Prepare metadata for training and test sets ..."
    cd ../..
    python mpii_prepare_meta.py

    echo "Generate labelled MPII dataset ..."
    python mpii_generate_data.py


## 3. VOC dataset ##
Run the following commands to download and generate VOC datasets.

    echo "Download dataset ..."
    mkdir data/voc
    cd data/voc
    wget https://pjreddie.com/media/files/VOCtrainval_11-May-2012.tar
    wget https://pjreddie.com/media/files/VOCtrainval_06-Nov-2007.tar
    wget https://pjreddie.com/media/files/VOCtest_06-Nov-2007.tar
    tar xf VOCtrainval_11-May-2012.tar
    tar xf VOCtrainval_06-Nov-2007.tar
    tar xf VOCtest_06-Nov-2007.tar

    echo "Generate labelled VOC dataset ..."
    cd ../..
    python voc_generate_data.py

####
    python generate_list_txt_voc.py --dataset=voc

## 4. Manual dataset ##
Prepare your manually labelled data, and copy all of them into ``data/manual``. Then run the following command to generate manually labelled datasets.

    python manual_merge_data.py
    python manual_convert_labels.py


## 4.1
Flip the images to get more data.

    python parallel_flip.py --data_dir=outputs/manual/images --label_dir=outputs/manual/labels --output_dir=outputs/manual_flip
    python parallel_flip.py --data_dir=outputs/voc/images_voc2007 --label_dir=outputs/voc/labels_voc2007 --output_dir=outputs/voc2007_flip
    python parallel_flip.py --data_dir=outputs/voc/images_voc2012 --label_dir=outputs/voc/labels_voc2012 --output_dir=outputs/voc2012_flip
    python parallel_flip.py --data_dir=outputs/mpii/images --label_dir=outputs/mpii/labels --output_dir=outputs/mpii_flip

## 5. Plot labelled images (optional) ##
Run the following command to verify whether the preprocessed, labelled images are correct.

    python drawbb.py --data_dir=outputs/manual/images --label_dir=outputs/manual/labels

The output images will be saved in ``outputs/drawbb``.


python parallel_roi.py --data_dir=outputs/mpii/images --label_dir=outputs/mpii/labels --output_dir=outputs/mpii_roi
python parallel_roi.py --data_dir=outputs/manual/images --label_dir=outputs/manual/labels --output_dir=outputs/manual_roi

## 6. Generate metadata files for training ##
Generate metadata files for all datasets, which will be used during training.

    cp outputs/voc/voc*.txt outputs/
    python generate_list_txt.py --dataset=mpii
    python generate_list_txt.py --dataset=manual
#rm outputs/voc.txt
    python generate_list_txt_voc.py --dataset=voc

    python generate_list_txt.py --dataset=mpii_flip
    python generate_list_txt.py --dataset=manual_flip
    python generate_list_txt.py --dataset=voc2007_flip
    python generate_list_txt.py --dataset=voc2012_flip
    sh merge_list_files.sh
    cp outputs/voc2007_test.txt test.txt

You will get ``train.txt`` and ``test.txt`` files.


## 7. Training ##
Run the following commands to train the model, which will be stored in ``white-robot-vision2/backup``.

    cp train.txt white-robot-vision2/
    cp test.txt white-robot-vision2/
    cd white-robot-vision2
    make -j4
    chmod u+x train_model.sh
    ./train_model.sh

You will get ``white-robot-vision2/backup/tiny-yolo-all_final.weights`` as the trained model.

## 8. Re-train the model when there are more manually, labelled images ##
Remove the preprocessed image of the manually, labelled dataset

    rm -rf outputs/manual

Then, repeat step 4, 6 and 7 to train a new model.


## 9. convert txt to xml file.
python parallel_txt2xml.py  --data_dir=outputs/manual/images/ --label_dir=outputs/manual/labels/ --output_dir=./tests1/
