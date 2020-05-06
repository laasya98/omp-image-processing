if [ $# -ne 1 ]
then 
	echo "please enter <out.csv>"
	exit
fi

echo "name, height, width, time, numRuns, p" > $1

array=(1 2 4 8)

FILES=$(ls images/*0.bmp)
for f in $FILES
do 
	echo $f
	./run.sh $f out.bmp 0 0 $1
	for n in "${array[@]}"
	do
		echo $n
		./omp.sh $f out.bmp 0 $n 0 $1
	done
done
