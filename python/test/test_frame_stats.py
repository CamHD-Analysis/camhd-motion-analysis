
import camhd_motion_analysis as ma


mov_path = '/RS03ASHS/PN03B/06-CAMHDA301/2016/01/01/CAMHDA301-20160101T000000Z.mov'


def test_one_frame():
    j = ma.frame_stats( mov_path, 1000, 1000, frame_stats_path="../build-Debug/bin/frame_stats" )
