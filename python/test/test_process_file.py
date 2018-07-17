
from camhd_motion_analysis import process_file

mov_path = 'RS03ASHS/PN03B/06-CAMHDA301/2018/07/01/CAMHDA301-20180701T001500.mov'

def test_process_file_with_output():

    result = process_file( mov_path, "test_tmp/test_process_file_with_output.json",
                stop=20 )

    # result = frame_stats( mov_path, 1000 )
    #
    # assert( 'frameStats' in result )
    # assert( 'movie' in result )
    # assert( 'contents' in result )
