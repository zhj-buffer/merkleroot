
echo "Prepare Manual Data"
python manual_merge_data.py
python manual_convert_labels.py

echo "Start Training"
cd white-robot-vision2
./eye_c detector train cfg/three.data cfg/tiny-yolo-three.cfg -gpus 0,1
