# camhd_motion_tracking

Requires OpenCV, TCLAP, boost

    sudo apt-get install libtclap-dev libopencv-dev, libboost-filesystem-dev
    mkdir build
    cd build
    cmake ..
    make

Sample

    motrack --display --skip 5000 --scale 0.25 --output-scale 0.5 --video-out CAMHDA301-20160517T000000Z.mkv --csv-out CAMHDA301-20160517T000000Z.csv ../data/CAMHDA301-20160517T000000Z.mp4
