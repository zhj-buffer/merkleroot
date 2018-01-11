# cat voc2007_train.txt voc2007_val.txt voc2012_*.txt coco.txt mpii.txt manual.txt > train.txt
#cat outputs/voc2007_train.txt outputs/voc2007_val.txt outputs/voc2012_*.txt outputs/mpii.txt outputs/manual.txt > train.txt
#cat outputs/voc.txt outputs/mpii.txt outputs/manual.txt  > train.txt
cat outputs/*.txt > train.txt
