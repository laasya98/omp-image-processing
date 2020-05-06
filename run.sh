i=0
MAX=10
time=0
size=0

if [ $# -ne 5 ]
then 
	echo "please enter <in.bmp> <out.bmp> <mode> <print(bool)> <out.csv>"
	exit
fi

gcc load_bmp.c -lm -o load_bmp
while [ $i -lt $MAX ]
do 
	./load_bmp $1 $2 $3 > out
	t=$(tail -n 1 out | grep -o -E '[0-9].+')
	time=$(awk "BEGIN {print (${time}+${t})}")
	i=$(( i + 1 ))
done

size=$(head -n 1 out | grep -o -E '[0-9]+')
stringarray=($size)
height=${stringarray[0]}
width=${stringarray[1]}
time=$(awk "BEGIN {print (${time}/${MAX})}")

echo "Average time: ${time} over ${MAX} runs"
echo "$1, ${height}, ${width}, ${time}, ${MAX}," >> $5
if [ "$4" -eq "1" ]
then
	gpicview $2 &
fi

