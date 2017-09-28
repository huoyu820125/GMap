if [ $# == 0 ]; then
		pid=$(pidof gsrc)
		if [ -n "$pid"  ]; then
			kill $pid
			echo "kill gsrc $pid"
		fi
        exit 1;
fi

if [ $# != 1 ]; then
        echo commond:stop.sh [binfile];
        exit 1;
fi
exe=$1
echo $exe

pid=$(pidof $exe)
if [ -n "$pid"  ]; then
	kill $pid
	echo "kill $exe $pid"
fi

