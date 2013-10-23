for i in {1..300}
do
tesseract eng.salt.exp$i.tif eng.salt.exp$i box.train.stderr
done
