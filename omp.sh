i=0
MAX=10
time=0
size=0

if [ $# -ne 6 ]
then 
	echo "please enter <in.bmp> <out.bmp> <mode> <p> <print(bool)> <out.csv>"
	exit
fi

# needed for definition of time.h
gcc load_bmp_omp.c -fopenmp -lm -std=c99 -D_POSIX_C_SOURCE=199309L -o load_omp
while [ $i -lt $MAX ]
do 
	./load_omp $1 $2 $3 $4 > out
	t=$(tail -n 1 out | grep -o -E '[0-9].+')
	
	#t=$(./load_omp $1 $2 $3 $4 | grep -o -E '[0-9].+' | tail -n 1)

	time=$(awk "BEGIN {print (${time}+${t})}")
	#time=$(bc -l <<<"${time}+${t}")
	#time=$(awk '{print $1+$2}' <<<"${time} ${t}")
	i=$(( i + 1 ))
done
size=$(head -n 1 out | grep -o -E '[0-9]+')
stringarray=($size)
height=${stringarray[0]}
width=${stringarray[1]}
time=$(awk "BEGIN {print (${time}/${MAX})}")
#time=$(awk '{print $1/$2}' <<<"${time} ${MAX}")
echo "Average time: ${time} over ${MAX} runs"
echo "$1, ${height}, ${width}, ${time}, ${MAX}, $4" >> $6
if [ "$5" -eq "1" ]
then
	gpicview $2 &
fi
