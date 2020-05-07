if [ $# -ne 2 ]
then 
	echo "please enter <out.csv> <print(bool)>"
	exit
fi

echo "name, height, width, mode, time, numRuns, p" > $1

array=(1 2 4 8)
modes=(0 1 2 3 4)

FILES=$(ls images/*0.bmp)
for f in $FILES
do 
	echo $f
	for mode in "${modes[@]}"
	do
		echo mode: $mode
		./run.sh $f out.bmp $mode $2 $1
		for p in "${array[@]}"
		do
			echo omp_procs: $p
			./omp.sh $f out.bmp $mode $p $2 $1
		done
	done
done
