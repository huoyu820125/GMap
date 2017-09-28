listType="run"
if [ $# = 1 ]; then
        listType=$1
fi

pid=$(pidof gsrc)
if [ -n "$pid"  ]; then
        if [ "run" = $listType ]; then
                echo "gsrc($pid) is running"
        fi
else
        if [ "stop" = $listType ]; then
                echo "gsrc is not run"
        fi
fi
