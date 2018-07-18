
from camhd_motion_analysis import frame_stats
from check_json_file import check_json

mov_path = 'RS03ASHS/PN03B/06-CAMHDA301/2018/07/01/CAMHDA301-20180701T001500.mov'

def test_one_frame():
    result = frame_stats( mov_path, 1000 )

    assert( 'frameStats' in result )
    assert( 'movie' in result )
    assert( 'contents' in result )
