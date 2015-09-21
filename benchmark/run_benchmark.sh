echo -------------------------
let start=$((`date +%s`*1000+`date +%-N`/1000000))
time ./c-ray/seq/c-ray-mt -i ./c-ray/sphfract -o /dev/null -s 1920x1080 -r 2
let end=$((`date +%s`*1000+`date +%-N`/1000000))
let time=($end-$start)
echo time for test $time milli
echo -------------------------
echo

echo -------------------------
let start=$((`date +%s`*1000+`date +%-N`/1000000))
time ./kmeans/seq/kmeans -i ./kmeans/edge  -b -n 2000
let end=$((`date +%s`*1000+`date +%-N`/1000000))
let time=($end-$start)
echo time for test $time milli
echo -------------------------
echo

echo -------------------------
let start=$((`date +%s`*1000+`date +%-N`/1000000))
time ./md5/seq/md5 -i 7 -c 10
let end=$((`date +%s`*1000+`date +%-N`/1000000))
let time=($end-$start)
echo time for test $time milli
echo -------------------------
echo

echo -------------------------
let start=$((`date +%s`*1000+`date +%-N`/1000000))
time ./tinyjpeg/seq/tinyjpeg --benchmark ./tinyjpeg/earth-marker.jpg /dev/null
let end=$((`date +%s`*1000+`date +%-N`/1000000))
let time=($end-$start)
echo time for test $time milli
echo -------------------------
echo


