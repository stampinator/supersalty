j=0
for i in {0..300}
do

FILE=eng.salt.exp$i.box
if [ -f $FILE ];
then
    cat $FILE | sed "s/0$/$j/" >> foo.txt
    ((j++))
else
    echo "File $FILE does not exist"
fi
done

