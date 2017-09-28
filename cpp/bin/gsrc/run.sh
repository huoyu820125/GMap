if [ $# != 1 ]; then
        echo commond:run.sh [binfile];
        exit 1;
fi
exe=$1

pid=$(pidof $exe)
if [ -n "$pid"  ]; then
        echo "Error:The $exe program aleary running $pid."
        exit 1;
fi


if [ $(echo $0 | grep '^/') ]; then
        exePath=$(dirname $0)
else
        exePath=$(pwd)/$(dirname $0)
fi
echo $exePath

nohup $exePath/$exe >$exePath/null 2>&1 &
pid=$(pidof $exe)
if [ -n "$pid"  ]; then
	echo "$exe($pid) is start"
	echo "log is in $exePath/log/$exe"
fi

