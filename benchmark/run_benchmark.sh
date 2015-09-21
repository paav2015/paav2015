echo -------------------------
let start=$((`date +%s`*1000+`date +%-N`/1000000))
time ../benchmark/c-ray/seq/c-ray-mt -i  ../benchmark/c-ray/sphfract -o /dev/null -s 1920x1080 -r 2
let end=$((`date +%s`*1000+`date +%-N`/1000000))
let time=($end-$start)
echo time for test $time milli
echo -------------------------
echo

echo -------------------------
let start=$((`date +%s`*1000+`date +%-N`/1000000))
time ../benchmark/kmeans/seq/kmeans -i  ../benchmark/kmeans/edge  -b -n 2000
let end=$((`date +%s`*1000+`date +%-N`/1000000))
let time=($end-$start)
echo time for test $time milli
echo -------------------------
echo

echo -------------------------
let start=$((`date +%s`*1000+`date +%-N`/1000000))
time ../benchmark/md5/seq/md5 -i 7 -c 10
let end=$((`date +%s`*1000+`date +%-N`/1000000))
let time=($end-$start)
echo time for test $time milli
echo -------------------------
echo

echo -------------------------
let start=$((`date +%s`*1000+`date +%-N`/1000000))
time ../benchmark/tinyjpeg/seq/tinyjpeg --benchmark  ../benchmark/tinyjpeg/earth-marker.jpg /dev/null
let end=$((`date +%s`*1000+`date +%-N`/1000000))
let time=($end-$start)
echo time for test $time milli
echo -------------------------
echo


