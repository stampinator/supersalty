STR=''

for i in {0..300}
do
        STR="$STR eng.salt.exp$i.tif"
done

convert $STR multi.tf
